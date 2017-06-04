//
// libn64/priv_include/os/mm.h: OS memory manager definitions.
//
// n64chain: A (free) open-source N64 development toolchain.
// Copyright 2014-16 Tyler J. Stachecki <stachecki.tyler@gmail.com>
//
// This file is subject to the terms and conditions defined in
// 'LICENSE', which is part of this source code package.
//

#ifndef LIBN64_PRIV_INCLUDE_OS_MM_H
#define LIBN64_PRIV_INCLUDE_OS_MM_H

#include <stdint.h>

// The RDRAM modules are organized as (up to) 8 banks of 1MB each.
// The smallest 'block' of memory that libn64 works on is a 4kB page.
// This makes for about 256 * 4KB pages per bank of RDRAM.
typedef uint16_t libn64_mm_page_list[8][256];

struct libn64_mm {
  uint16_t free_page_idxs[8];
} __attribute__((aligned(16)));

// Initializes the memory management (mm) subsystem.
libn64func
void libn64_mm_init(uint32_t physmem_bottom, uint32_t physmem_top);

#endif

