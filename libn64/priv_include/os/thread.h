//
// libn64/priv_include/os/thread.h: OS thread definitions.
//
// n64chain: A (free) open-source N64 development toolchain.
// Copyright 2014-16 Tyler J. Stachecki <stachecki.tyler@gmail.com>
//
// This file is subject to the terms and conditions defined in
// 'LICENSE', which is part of this source code package.
//

#ifndef LIBN64_PRIV_INCLUDE_OS_THREAD_H
#define LIBN64_PRIV_INCLUDE_OS_THREAD_H

#include <stdint.h>

#define LIBN64_THREAD_MIN_PRIORITY 0

struct libn64_thread_state {
  uint32_t regs[32];

  uint32_t cp0_status;
  uint32_t cp0_entryhi;
  uint32_t cp1_control;
  uint32_t mi_intr_reg;

  uint64_t fp_regs[32];
} __attribute__((aligned(16)));

struct libn64_thread {
  struct libn64_thread_state state;

  unsigned priority;
  uint32_t unused[11];

  uint16_t stack_pte[32];
};

// Initializes the threading subsystem.
libn64func
void libn64_thread_early_init(uint32_t ram_top);

#endif

