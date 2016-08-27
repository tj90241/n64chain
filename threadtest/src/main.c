//
// fputest/src/main.c: FPU test cart (entry point).
//
// n64chain: A (free) open-source N64 development toolchain.
// Copyright 2014-16 Tyler J. Stachecki <stachecki.tyler@gmail.com>
//
// This file is subject to the terms and conditions defined in
// 'LICENSE', which is part of this source code package.
//

#include <os/fbtext.h>
#include <os/syscall.h>
#include <os/thread_table.h>
#include <rcp/vi.h>
#include <stdint.h>

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

// Higher priority thread.
volatile unsigned threads_spawned;

void thread_main(void *arg) {
  struct libn64_fbtext_context *fbtext = (struct libn64_fbtext_context *) arg;
  uint32_t mythreadaddr;
  unsigned my_prio;

  my_prio = (++threads_spawned);

  __asm__ __volatile__("addu %0, $k1, $zero\n\t" : "=r" (mythreadaddr));

#if 1
  if (threads_spawned < 15)
    libn64_thread_create(thread_main, fbtext, my_prio + 1);
#endif

  libn64_fbtext_puts(fbtext, "App thread! Thr=");
  libn64_fbtext_putu32(fbtext, mythreadaddr);
  libn64_fbtext_puts(fbtext, ",Prio=");
  libn64_fbtext_putu32(fbtext, my_prio - 1);
  libn64_fbtext_puts(fbtext, "\n");
  libn64_thread_exit();
}

// Application entry point.
void main(void *unused __attribute__((unused))) {
  unsigned i;

  // Set default thread stack addresses until we get something better.
  // Give each thread a 4K stack starting at 1MB (except the current thread).
  for (i = 0; i < LIBN64_THREADS_MAX - 2; i++)
    libn64_thread_table->free_list[i]->state.regs[0x68/4] = 0x80100000 + 0x1000 * i;

  // Wipe FB to black.
  volatile uint16_t *fb = (volatile uint16_t *) 0xA0200000;

  for (i = 0; i < 320 * 240; i++)
    *(fb + i) = 0;

  // Enable the VI.
  vi_flush_state(&vi_state);

  struct libn64_fbtext_context fbtext;
  libn64_fbtext_init(&fbtext, 0x200000, LIBN64_FBTEXT_COLOR_WHITE,
      LIBN64_FBTEXT_COLOR_BLACK, 0x140, LIBN64_FBTEXT_16BPP);

#if 1
  threads_spawned = 1;
  libn64_thread_create(thread_main, &fbtext, 2);
#endif

  libn64_fbtext_puts(&fbtext, "Idle thread!\n");
}

