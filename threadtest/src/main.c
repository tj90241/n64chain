//
// fputest/src/main.c: FPU test cart (entry point).
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

// Higher priority thread.
unsigned threads_spawned;

void thread_main(void *arg __attribute__((unused))) {
  unsigned my_prio;

  my_prio = (++threads_spawned);

#if 1
  if (threads_spawned < 14)
    libn64_thread_create(thread_main, &fbtext, my_prio + 1);
#endif

  libn64_fbtext_puts(&fbtext, "App thread! Prio=");
  libn64_fbtext_putu32(&fbtext, my_prio - 1);
  libn64_fbtext_puts(&fbtext, "\n");
}

// Application entry point.
void main(void *unused __attribute__((unused))) {
  unsigned i;

  // Wipe FB to black.
  volatile uint16_t *fb = (volatile uint16_t *) 0xA0200000;

  for (i = 0; i < 320 * 240; i++)
    *(fb + i) = 0;

  // Enable the VI.
  vi_flush_state(&vi_state);

  libn64_fbtext_init(&fbtext, 0x200000, LIBN64_FBTEXT_COLOR_WHITE,
      LIBN64_FBTEXT_COLOR_BLACK, 0x140, LIBN64_FBTEXT_16BPP);

  threads_spawned = 1;
  libn64_thread child = libn64_thread_create(thread_main, &fbtext, 3);
  libn64_send_message(child, 2);

  libn64_fbtext_puts(&fbtext, "VI intrs=");

  // Deliver interrupt messages to this thread's message queue.
  libn64_thread_reg_intr(libn64_thread_self(), LIBN64_INTERRUPT_VI);

  // Display a '.' once per second, 20 times.
  for (i = 0; i < 20; i++) {
    libn64_fbtext_puts(&fbtext, ".");

    // Catch 60 VI interrupts (i.e., wait 1 sec)
    unsigned j;
    for (j=0; j<60; j++) {
      libn64_recv_message();
    }
  }
}

