//
// libn64/include/rcp/sp.h: Signal process (SP) helper functions.
//
// n64chain: A (free) open-source N64 development toolchain.
// Copyright 2014-16 Tyler J. Stachecki <stachecki.tyler@gmail.com>
//
// This file is subject to the terms and conditions defined in
// 'LICENSE', which is part of this source code package.
//

#ifndef LIBN64_INCLUDE_RCP_SP_H
#define LIBN64_INCLUDE_RCP_SP_H

#include <libn64.h>
#include <stddef.h>
#include <stdint.h>

#define RSP_STATUS_CLEAR_HALT               (1 <<  0)
#define RSP_STATUS_SET_HALT                 (1 <<  1)
#define RSP_STATUS_CLEAR_BROKE              (1 <<  2)
#define RSP_STATUS_CLEAR_INTR               (1 <<  3)
#define RSP_STATUS_SET_INTR                 (1 <<  4)
#define RSP_STATUS_CLEAR_SSTEP              (1 <<  5)
#define RSP_STATUS_SET_SSTEP                (1 <<  6)
#define RSP_STATUS_CLEAR_INTR_ON_BREAK      (1 <<  7)
#define RSP_STATUS_SET_INTR_ON_BREAK        (1 <<  8)
#define RSP_STATUS_CLEAR_SIGNAL_0           (1 <<  9)
#define RSP_STATUS_SET_SIGNAL_0             (1 << 10)
#define RSP_STATUS_CLEAR_SIGNAL_1           (1 << 11)
#define RSP_STATUS_SET_SIGNAL_1             (1 << 12)
#define RSP_STATUS_CLEAR_SIGNAL_2           (1 << 13)
#define RSP_STATUS_SET_SIGNAL_2             (1 << 14)
#define RSP_STATUS_CLEAR_SIGNAL_3           (1 << 15)
#define RSP_STATUS_SET_SIGNAL_3             (1 << 16)
#define RSP_STATUS_CLEAR_SIGNAL_4           (1 << 17)
#define RSP_STATUS_SET_SIGNAL_4             (1 << 18)
#define RSP_STATUS_CLEAR_SIGNAL_5           (1 << 19)
#define RSP_STATUS_SET_SIGNAL_5             (1 << 20)
#define RSP_STATUS_CLEAR_SIGNAL_6           (1 << 21)
#define RSP_STATUS_SET_SIGNAL_6             (1 << 22)
#define RSP_STATUS_CLEAR_SIGNAL_7           (1 << 23)
#define RSP_STATUS_SET_SIGNAL_7             (1 << 24)

// Issues a DMA to the RSP.
//
// Does NOT perform any checks. Be sure that you grab the
// semaphore as needed, adjust the length (subtract one from
// the amount you actually want copied), etc.
libn64func
static inline void libn64_rsp_dma_to_rsp(
  uint32_t sp_addr, uint32_t paddr, size_t len) {
  __asm__ __volatile__(
    "sw %[sp_addr], 0x00(%[sp_region])\n\t"
    "sw %[paddr],   0x04(%[sp_region])\n\t"
    "sw %[len],     0x08(%[sp_region])\n\t"

    :: [sp_addr] "r" (sp_addr), [paddr] "r" (paddr), [len] "r" (len),
       [sp_region] "r" (0xA4040000U)
  );
}

// Checks for a pending RSP DMA.
//
// Returns 1 if a DMA is pending, 0 otherwise.
libn64func
static inline uint32_t libn64_rsp_is_dma_pending(void) {
  return *(volatile const uint32_t *) 0xA4040018;
}

// Returns the RSP status register.
libn64func
static inline uint32_t libn64_rsp_get_status(void) {
  return *(volatile const uint32_t *) 0xA4040010;
}

// Sets the RSP PC register.
libn64func
static inline void libn64_rsp_set_pc(uint32_t pc) {
  *(volatile uint32_t *) 0xA4080000 = pc;
}

// Updates the RSP status flags according to mask.
//
// (see RSP_STATUS_CLEAR_* and RSP_STATUS_SET_*)
libn64func
static inline void libn64_rsp_set_status(uint32_t mask) {
  *(volatile uint32_t *) 0xA4040010 = mask;
}

#endif

