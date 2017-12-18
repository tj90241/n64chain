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
#include <io/thread.h>
#include <os/idle_thread.h>
#include <os/mm.h>
#include <os/thread.h>
#include <os/time.h>
#include <stddef.h>
#include <syscall.h>

void main(void *);

// Entry point (invoked from IPL handler).
libn64func __attribute__((noreturn))
void libn64_main(uint32_t kernel_sp, uint32_t bss_end) {

  // Initialize critical subsystems.
  libn64_time_init();
  libn64_thread_early_init(kernel_sp);

  // Give libn64 the minimum amount of memory allowable (~24KiB)
  // right above the thread table (at the top of RAM).
  //
  // 4/8MiB -----------------+
  //   |                     |
  //   | libn64 thread block |  <- ~512b/thread
  //   |_____________________|
  //   |                     |
  //   |     libn64 heap     |  <- 32kiB
  //   |                     |
  //   +---------------------+
  //   |                     |
  //   |       (free)        |  <- RDRAM - (32KiB - thread space)
  //   |_____________________|
  //   |                     |
  //   |    libn64 kernel    |  <- 8KiB
  //   |                     |
  // 0 MiB-------------------+
  //
  //
  // The user can grow this themselves if they want more memory.
  // This is kind of "dangerous" in the sense that we allocate
  // pages on top of our active stack, but it's fine for now...
  libn64_mm_init(kernel_sp - 4096 * 8, kernel_sp);

  // Kickoff the IO engine.
  libn64_io_init();

  // This thread invokes main() and becomes the idle thread.
  libn64_idle_thread();
  __builtin_unreachable();
}

