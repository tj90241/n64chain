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

#include <libn64.h>
#include <mq.h>
#include <stdint.h>

struct libn64_thread_state {
  uint32_t regs[32];

  uint32_t cp0_status;
  uint32_t cp0_entryhi;
  uint32_t cp1_control;
  uint32_t mi_intr_reg;

  uint64_t fp_regs[32];
} __attribute__((aligned(16)));

struct libn64_thread_internal {
  struct libn64_thread_state state;

  struct libn64_message *messages_tail;
  struct libn64_message *messages_head;
  uint32_t priority;
  uint32_t blocked;

  void *ai_intr_chain;
  void *dp_intr_chain;
  void *pi_intr_chain;
  void *sp_intr_chain;
  void *si_intr_chain;
  void *vi_intr_chain;
  uint32_t count[2];

  uint16_t stack_pte[32];
};

// Initializes the threading subsystem.
libn64func
libn64_thread libn64_thread_early_init(uint32_t ram_top);

#endif

