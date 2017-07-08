//
// libn64/os/thread.c: OS thread functions.
//
// n64chain: A (free) open-source N64 development toolchain.
// Copyright 2014-16 Tyler J. Stachecki <stachecki.tyler@gmail.com>
//
// This file is subject to the terms and conditions defined in
// 'LICENSE', which is part of this source code package.
//

#include <libn64.h>
#include <os/mm.h>
#include <stdint.h>

// Returns a pointer to the libn64_mm structure.
libn64func __attribute__((always_inline))
static inline struct libn64_mm *libn64_get_mm(void) {
  struct libn64_mm *mm;

  __asm__ __volatile__(
    "lui %0, 0x8000\n\t"
    "addiu %0, %0, 0x410\n\t"
    : "=r"(mm)
  );

  return mm;
}

// Returns a pointer to the libn64_mm_page_list structure.
libn64func __attribute__((always_inline))
static inline libn64_mm_page_list *libn64_get_mm_page_list(void) {
  libn64_mm_page_list *mm_page_list;

  __asm__ __volatile__(
    "lui %0, 0x8000\n\t"
    "lw %0, 0x428(%0)\n\t"
    : "=r"(mm_page_list)
  );

  return mm_page_list;
}

// Initialize the memory manager table.
void libn64_mm_init(uint32_t physmem_bottom, uint32_t physmem_top) {
  struct libn64_mm *mm = libn64_get_mm();
  libn64_mm_page_list *mm_page_list;
  unsigned i;

  // Fill the page allocator.
  physmem_bottom = (physmem_bottom + 4095) & 0xFFFFF000;
  physmem_top = (physmem_top - 4095) & 0xFFFFF000;

  mm_page_list = (libn64_mm_page_list *) (physmem_top);
  physmem_top -= 0x1000;

  __asm__ __volatile__(
    ".set gp=64\n\t"
    "cache 0xD, (%0)\n\t"
    "sd $zero, 0x0(%0)\n\t"
    "sd $zero, 0x8(%0)\n\t"
    ".set gp=default\n\t"
    :: "r"(mm) : "memory"
  );

  do {
    unsigned bank = (physmem_bottom >> 20) & 0x7;
    unsigned idx = mm->free_page_idxs[bank]++;

    (*mm_page_list)[bank][idx] = physmem_bottom >> 12;
    physmem_bottom += 0x1000;
  } while (physmem_bottom != physmem_top);

  // Set the page allocator pointer.
  // Clear L2 stack entry list pointer.
  __asm__ __volatile__(
    ".set noat\n\t"
    "lui $at, 0x8000\n\t"
    "sw %0, 0x428($at)\n\t"
    "sw $zero, 0x42C($at)\n\t"
    ".set at\n\t"
    :: "r"(mm_page_list)
    : "memory"
  );

  // Invalidate all the TLB entries.
  for (i = 0; i < 32; i++) {
    __asm__ __volatile__(
      "mtc0 %0, $0\n\t" // Index
      "tlbwi\n\t"
      :: "r" (i)
    );
  }
}

