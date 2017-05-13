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

libn64_mm_page_list *libn64_mm_pages;
volatile void *libn64_mm_l2_stack_entries;
struct libn64_mm libn64_mm;

// Allocates a 4kB page from the first available bank.
void *libn64_mm_alloc(void) {
  unsigned i;

  for (i = 0; i < 8; i++) {
    if (libn64_mm.free_page_idxs[i]) {
      uint16_t page_index = --(libn64_mm.free_page_idxs[i]);
      uint16_t page_addr = (*libn64_mm_pages)[i][page_index];
      void *page;

      __asm__ __volatile__(
        "sll %1, %1, 0xC\n\t"
        "lui %0, 0x8000\n\t"
        "or %0, %0, %1\n\t"
        : "=&r"(page), "=&r"(page_addr)
        : "1"(page_addr)
      );

      return page;
    }
  }

  // Out of memory; force a crash for now...
  __asm__ __volatile__("sw $zero, 0x0($zero)\n\t");
  __builtin_unreachable();
}

// Initialize the memory manager table.
void libn64_mm_init(uint32_t physmem_bottom, uint32_t physmem_top) {
  unsigned i;

  // Fill the page allocator.
  physmem_bottom = (physmem_bottom + 4095) & 0xFFFFF000;
  physmem_top = (physmem_top - 4095) & 0xFFFFF000;

  libn64_mm_pages = (libn64_mm_page_list *) (physmem_top);
  physmem_top -= 0x1000;

  for (; physmem_bottom != physmem_top; physmem_bottom += 0x1000) {
    unsigned bank = (physmem_bottom >> 20) & 0x7;
    unsigned idx = libn64_mm.free_page_idxs[bank]++;

    (*libn64_mm_pages)[bank][idx] = physmem_bottom >> 12;
  }

  // Invalidate all the TLB entries.
  __asm__ __volatile__("mtc0 $zero, $5\n\t"  // PageMask
                       "mtc0 $zero, $10\n\t" // EntryHi
                       "mtc0 $zero, $2\n\t"  // EntryLo0
                       "mtc0 $zero, $3\n\t"  // EntryLo1
                       "mtc0 $zero, $6\n\t"  // Wired
  );

  for (i = 0; i < 32; i++) {
    __asm__ __volatile__("mtc0 %0, $0\n\t"   // Index
                         "tlbwi\n\t"
      :: "r" (i)
    );
  }
}

