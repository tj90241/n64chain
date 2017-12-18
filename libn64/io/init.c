//
// libn64/io/init.c: Parallel and serial I/O initialization.
//
// n64chain: A (free) open-source N64 development toolchain.
// Copyright 2014-16 Tyler J. Stachecki <stachecki.tyler@gmail.com>
//
// This file is subject to the terms and conditions defined in
// 'LICENSE', which is part of this source code package.
//

#include <io/thread.h>
#include <libn64.h>
#include <stddef.h>
#include <syscall.h>

libn64func
void libn64_io_init(void) {
  libn64_thread io_thread;

  io_thread = libn64_thread_create(libn64_io_thread, NULL,
      LIBN64_THREAD_MAX_PRIORITY);

  // Store the thread address in the global block.
  __asm__ __volatile__(
    ".set noat\n\t"
    ".set gp=64\n\t"
    "lui $at, 0x8000\n\t"
    "sw %0, 0x450($at)\n\t"
    ".set gp=default\n\t"
    ".set at\n\t"

    :: "r" (io_thread)
    : "memory"
  );
}

