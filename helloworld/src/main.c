//
// helloworld/src/main.c: Threading test cart (entry point).
//
// n64chain: A (free) open-source N64 development toolchain.
// Copyright 2014-16 Tyler J. Stachecki <stachecki.tyler@gmail.com>
//
// This file is subject to the terms and conditions defined in
// 'LICENSE', which is part of this source code package.
//

#include <filesystem.h>
#include <os/fbtext.h>
#include <rcp/vi.h>
#include <stdint.h>
#include <syscall.h>

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

static uint32_t fb_origin;

void main(void *unused __attribute__((unused))) {
  struct libn64_fbtext_context context;

  // Setup the OS's private/hidden text rendering engine.
  // The 0 and ~0 are just fill colors (black and white).
  libn64_fbtext_init(&context, 0x80000000 | vi_state.origin,
      ~0, 0, 320, LIBN64_FBTEXT_16BPP);

  // Register VI interrupts on this thread. When registering a thread w/
  // interrupts, it causes the threads message queue to get populated w/
  // a message each time an interrupt fires.
  libn64_thread_reg_intr(libn64_thread_self(), LIBN64_INTERRUPT_VI);

  // For each frame...
  while (1) {
    unsigned i;

    // Point to VI to the last fb, swap the front and back fbs.
    vi_flush_state(&vi_state);
    vi_state.origin ^= 0x100000; // 1MB

    // Wipe the back-buffer to black.
    for (i = 0; i < 320 * 240 * 2; i += 16) {
      __asm__ __volatile__(
        ".set gp=64\n\t"
        "cache 0xD, 0x0(%0)\n\t"
        "sd $zero, 0x0(%0)\n\t"
        "sd $zero, 0x8(%0)\n\t"
        "cache 0x19, 0x0(%0)\n\t"
        ".set gp=default\n\t"

        :: "r" (0x80000000 | (vi_state.origin + i))
        : "memory"
      );
    }

    // Tell the text engine to render on the back buffer.
    context.fb_origin = 0x80000000 | vi_state.origin;

    // Set the position of the cursor. If your string
    // contains \n, it will automatically advance it.
    // The text also wraps around, but it won't scroll.
    context.x = 13;
    context.y = 6;

    // Finally, render text where the cursor is placed.
    extern char _binary_filesystem_bin_start;
    uint32_t text_offset = CART_OFFS_DATA_TXT - 0x1000;
    char *fs_ptr = &_binary_filesystem_bin_start;
    libn64_fbtext_puts(&context, fs_ptr + text_offset);

    // Block until the next VI interrupt comes in.
    libn64_recvt_message();
  }
}

