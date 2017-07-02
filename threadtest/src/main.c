//
// threadtest/src/main.c: Threading test cart (entry point).
//
// n64chain: A (free) open-source N64 development toolchain.
// Copyright 2014-16 Tyler J. Stachecki <stachecki.tyler@gmail.com>
//
// This file is subject to the terms and conditions defined in
// 'LICENSE', which is part of this source code package.
//

#include <fbtext.h>
#include <rcp/vi.h>
#include <stdint.h>
#include <syscall.h>
#include <vr4300/cp0.h>

// These pre-defined values are suitable for NTSC.
// TODO: Add support for PAL and PAL-M televisions.
static vi_state_t vi_state = {
  0x0000324E, // status
  0x00200000, // origin
  0x00000140, // width
  0x00000002, // intr
  0x00000000, // current
  0x03E52239, // burst
  0x0000020D, // v_sync
  0x00000C15, // h_sync
  0x0C150C15, // leap
  0x006C02EC, // h_start
  0x002501FF, // v_start
  0x000E0204, // v_burst
  0x00000200, // x_scale
  0x00000400, // y_scale
};

struct libn64_fbtext_context fbtext;
uint32_t fb_origin;

// Flip to the next page, clear the framebuffer.
void vi_thread(void *opaque) {
  struct vi_state_t *vi_state = (struct vi_state_t *) opaque;

  while (1) {
    unsigned i;

    // Point to VI to the last fb, swap the fbs.
    vi_flush_state(vi_state);
    vi_state->origin ^= 0x100000; // 1MB

    for (i = 0; i < 320 * 240 * 2; i += 16) {
      __asm__ __volatile__(
        ".set gp=64\n\t"
        "cache 0xD, 0x0(%0)\n\t"
        "sd $zero, 0x0(%0)\n\t"
        "sd $zero, 0x8(%0)\n\t"
        "cache 0x19, 0x0(%0)\n\t"
        ".set gp=default\n\t"

        :: "r" (0x80000000 | (vi_state->origin + i))
        : "memory"
      );
    }

    // Block until the next VI interrupt comes in.
    libn64_recv_message();
  }
}

struct box_anim_args {
  struct vi_state_t *vi_state;
  char init_x_dir, init_y_dir;
  unsigned init_x, init_y;
};

void box_anim_thread(void *opaque) {
  struct box_anim_args *args = (struct box_anim_args *) opaque;
  struct vi_state_t *vi_state = args->vi_state;
  unsigned cur_x = args->init_x;
  unsigned cur_y = args->init_y;
  char x_dir = args->init_x_dir;
  char y_dir = args->init_y_dir;

  libn64_recv_message();

  while (1) {
    // Draw the box (8x8).
    unsigned i, j;

    for (j = 0; j < 8; j++) {
      for (i = 0; i < 8 * 2; i += 16) {
        uint32_t offs = (cur_y + j) * 320 * 2 + cur_x * 2 +
                        vi_state->origin;

        __asm__ __volatile__(
          ".set noat\n\t"
          ".set gp=64\n\t"
          "addiu $at, $zero, -0x1\n\t"
          "sh $at, 0x0(%0)\n\t"
          "sh $at, 0x2(%0)\n\t"
          "sh $at, 0x4(%0)\n\t"
          "sh $at, 0x6(%0)\n\t"
          "sh $at, 0x8(%0)\n\t"
          "sh $at, 0xA(%0)\n\t"
          "sh $at, 0xC(%0)\n\t"
          "sh $at, 0xE(%0)\n\t"
          ".set gp=default\n\t"
          ".set at\n\t"

          :: "r" (0x80000000 | offs)
          : "memory"
        );
      }
    }

    // Update the box's coordinates.
    if (x_dir) { cur_x++; } else { cur_x--; }
    if (y_dir) { cur_y++; } else { cur_y--; }

    if (cur_x >= 300)
      x_dir = 0;

    else if (cur_x <= 20)
      x_dir = 1;

    if (cur_y >= 220)
      y_dir = 0;

    else if (cur_y <= 20)
      y_dir = 1;

    // Block until the next VI interrupt comes in.
    libn64_recv_message();
  }
}

// Application entry point.
static struct box_anim_args args;

void main(void *unused __attribute__((unused))) {
  unsigned i;

  // Spawn a very high-priority clear framebuffer thread.
  libn64_thread vi_thr = libn64_thread_create(
    vi_thread, &vi_state, 255);

  libn64_thread_reg_intr(vi_thr, LIBN64_INTERRUPT_VI);

  // Spawn a few medium-priority box animation threads.
  for (i = 1; i < 12; i++) {
    args.vi_state = &vi_state;
    args.init_x_dir = i & 1;
    args.init_y_dir = (i >> 1) & 1;
    args.init_x = i * 25;
    args.init_y = i * 15;

    libn64_thread box_thr = libn64_thread_create(
      box_anim_thread, &args, 2);

    libn64_thread_reg_intr(box_thr, LIBN64_INTERRUPT_VI);
  }

  // Update the 'Running...' animation.
  libn64_thread_reg_intr(libn64_thread_self(), LIBN64_INTERRUPT_VI);

  libn64_fbtext_init(&fbtext, 0x200000, LIBN64_FBTEXT_COLOR_WHITE,
      LIBN64_FBTEXT_COLOR_BLACK, 0x140, LIBN64_FBTEXT_16BPP);

  unsigned cur_frames = 1;

  while (1) {
    fbtext.x = fbtext.y = 1;
    fbtext.fb_origin = 0x80000000 | vi_state.origin;
    libn64_fbtext_puts(&fbtext, " Running");

    for (i = 0; i < cur_frames / 60 + 1; i++)
      libn64_fbtext_puts(&fbtext, ".");

    for (; i < 3; i++)
      libn64_fbtext_puts(&fbtext, " ");

    cur_frames++;
    if (cur_frames >= 240)
      cur_frames = 1;

    libn64_recv_message();
  }
}

