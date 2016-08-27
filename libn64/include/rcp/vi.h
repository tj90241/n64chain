//
// libn64/include/rcp/vi.h: VI helper functions.
//
// n64chain: A (free) open-source N64 development toolchain.
// Copyright 2014-16 Tyler J. Stachecki <stachecki.tyler@gmail.com>
//
// This file is subject to the terms and conditions defined in
// 'LICENSE', which is part of this source code package.
//

#ifndef LIBN64_INCLUDE_RCP_VI_H
#define LIBN64_INCLUDE_RCP_VI_H

#include <libn64.h>
#include <stdint.h>

typedef struct vi_state_t {
  uint32_t status;
  uint32_t origin;
  uint32_t width;
  uint32_t intr;
  uint32_t current;
  uint32_t burst;
  uint32_t v_sync;
  uint32_t h_sync;
  uint32_t leap;
  uint32_t h_start;
  uint32_t v_start;
  uint32_t v_burst;
  uint32_t x_scale;
  uint32_t y_scale;
} vi_state_t __attribute__ ((aligned (8)));

// Flushes the register data to hardware registers.
//
// - Caller is responsible for disabling interrupts.
// - state _must_ pointed to cached memory (mapped or not).
libn64func
void vi_flush_state(const vi_state_t *state);

#endif

