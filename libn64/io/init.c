//
// libn64/io/init.c: Parallel and serial I/O initialization.
//
// n64chain: A (free) open-source N64 development toolchain.
// Copyright 2014-16 Tyler J. Stachecki <stachecki.tyler@gmail.com>
//
// This file is subject to the terms and conditions defined in
// 'LICENSE', which is part of this source code package.
//

#include <io/init.h>
#include <io/pi_thread.h>
#include <io/si_thread.h>
#include <libn64.h>
#include <stddef.h>
#include <syscall.h>

void libn64_io_init(void) {
  libn64_thread pi_thread, si_thread;

  pi_thread = libn64_thread_create(libn64_pi_thread, NULL,
      LIBN64_THREAD_MAX_PRIORITY);

  si_thread = libn64_thread_create(libn64_si_thread, NULL,
      LIBN64_THREAD_MAX_PRIORITY);

  // Store the thread address in the global block.
  __asm__ __volatile__(
    ".set noat\n\t"
    ".set gp=64\n\t"
    "lui $at, 0x8000\n\t"
    "sw %0, 0x450($at)\n\t"
    "sw %1, 0x454($at)\n\t"
    ".set gp=default\n\t"
    ".set at\n\t"

    :: "r" (pi_thread), "r" (si_thread)
    : "memory"
  );
}

