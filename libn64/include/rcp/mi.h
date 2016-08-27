//
// libn64/include/rcp/mi.h: MI helper functions.
//
// n64chain: A (free) open-source N64 development toolchain.
// Copyright 2014-16 Tyler J. Stachecki <stachecki.tyler@gmail.com>
//
// This file is subject to the terms and conditions defined in
// 'LICENSE', which is part of this source code package.
//

#ifndef LIBN64_INCLUDE_RCP_MI_H
#define LIBN64_INCLUDE_RCP_MI_H

#include <libn64.h>
#include <stdint.h>

#define MI_INTR_CLEAR_SP                    (1 <<  0)
#define MI_INTR_SET_SP                      (1 <<  1)
#define MI_INTR_CLEAR_SI                    (1 <<  2)
#define MI_INTR_SET_SI                      (1 <<  3)
#define MI_INTR_CLEAR_AI                    (1 <<  4)
#define MI_INTR_SET_AI                      (1 <<  5)
#define MI_INTR_CLEAR_VI                    (1 <<  6)
#define MI_INTR_SET_VI                      (1 <<  7)
#define MI_INTR_CLEAR_PI                    (1 <<  8)
#define MI_INTR_SET_PI                      (1 <<  9)
#define MI_INTR_CLEAR_DP                    (1 << 10)
#define MI_INTR_SET_DP                      (1 << 11)

// Sets the MI_INTR_MASK_REG bits.
//
// (see MI_INTR_CLEAR_* and MI_INTR_SET_*)
libn64func
static inline void mi_set_intr_mask(uint32_t mask) {
  *(volatile uint32_t *) 0xA430000C = mask;
}

#endif

