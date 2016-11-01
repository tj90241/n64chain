//
// libn64/vr4300/cp0.h: VR4300/CP0 helper functions.
//
// n64chain: A (free) open-source N64 development toolchain.
// Copyright 2014-16 Tyler J. Stachecki <stachecki.tyler@gmail.com>
//
// This file is subject to the terms and conditions defined in
// 'LICENSE', which is part of this source code package.
//

#ifndef LIBN64_INCLUDE_VR4300_CP0_H
#define LIBN64_INCLUDE_VR4300_CP0_H

#include <stdint.h>

// Disables VR4300 interrupts.
static inline void vr4300_cp0_disable_interrupts(void) {
  uint32_t status;

  __asm__ __volatile__(
    "mfc0 %[status], $12\n\t"
    "srl %[status], %[status], 0x1\n\t"
    "sll %[status], %[status], 0x1\n\t"
    "mtc0 %[status], $12\n\t"

    : [status] "=r"(status)
  );
}

// Enables VR4300 interrupts.
static inline void vr4300_cp0_enable_interrupts(void) {
  uint32_t status;

  __asm__ __volatile__(
    "mfc0 %[status], $12\n\t"
    "ori %[status], %[status], 0x1\n\t"
    "mtc0 %[status], $12\n\t"

    : [status] "=r"(status)
  );
}

#endif

