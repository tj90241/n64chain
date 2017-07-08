//
// libn64/os/main.c: libn64 C entry point.
//
// n64chain: A (free) open-source N64 development toolchain.
// Copyright 2014-16 Tyler J. Stachecki <stachecki.tyler@gmail.com>
//
// This file is subject to the terms and conditions defined in
// 'LICENSE', which is part of this source code package.
//

#include <libn64.h>
#include <os/idle_thread.h>
#include <os/mm.h>
#include <os/thread.h>
#include <stddef.h>
#include <syscall.h>

void main(void *);

// Entry point (invoked from IPL handler).
libn64func __attribute__((noreturn))
void libn64_main(uint32_t kernel_sp, uint32_t bss_end) {
  libn64_thread idle_thread = libn64_thread_early_init(kernel_sp);

  // Put the given physical memory region under control of the MM.
  libn64_mm_init(bss_end, kernel_sp - 256);

  // Hand control over to the application (in another thread).
  libn64_thread_create(main, NULL, LIBN64_THREAD_MIN_PRIORITY + 1);

  // This thread becomes the idle thread.
  libn64_idle_thread();
  __builtin_unreachable();
}

