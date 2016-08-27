//
// libn64/rcp/vi.c: VI helper functions.
//
// n64chain: A (free) open-source N64 development toolchain.
// Copyright 2014-16 Tyler J. Stachecki <stachecki.tyler@gmail.com>
//
// This file is subject to the terms and conditions defined in
// 'LICENSE', which is part of this source code package.
//

#include <rcp/vi.h>

void vi_flush_state(const vi_state_t *state) {
  uint32_t vi_region = 0xA4400000U;
  uint32_t data, end_ptr;

  __asm__ __volatile__(
    ".set noreorder\n\t"
    ".set gp=64\n\t"
    "addiu %[end_ptr], %[vi_region], 0x30\n"

    "1:\n\t"
    "ld %[data], 0x30(%[state])\n\t"
    "addiu %[state], %[state], -0x8\n\t"
    "sw %[data], 0x4(%[end_ptr])\n\t"
    "dsrl32 %[data], %[data], 0x0\n\t"
    "sw %[data], 0x0(%[end_ptr])\n\t"
    "bne %[vi_region], %[end_ptr], 1b\n\t"
    "addiu %[end_ptr], %[end_ptr], -0x8\n\t"
    ".set gp=default\n\t"
    ".set reorder\n\t"

    : [state] "=&r" (state), [data] "=&r" (data),
      [end_ptr] "=&r" (end_ptr), [vi_region] "=&r" (vi_region)
    : "0" (state), "3" (vi_region)
  );
}

