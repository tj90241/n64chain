//
// libn64/os/main.c: libn64 C entry point.
//
// n64chain: A (free) open-source N64 development toolchain.
// Copyright 2014-16 Tyler J. Stachecki <stachecki.tyler@gmail.com>
//
// This file is subject to the terms and conditions defined in
// 'LICENSE', which is part of this source code package.
//

#include <libn64.h>
#include <os/kthread.h>
#include <os/mm.h>
#include <os/thread.h>
#include <os/thread_table.h>
#include <os/syscall.h>
#include <stddef.h>

extern uint8_t __bss_start;
extern uint8_t __bss_end;
void main(void *);

libn64func
static void clear_bss(void);

// Effectively a memset(__bss_start, 0, __bss_end - __bss_start).
// This is necessary as the compiler expects BSS is initially zero.
void clear_bss(void) {
  uint32_t bss_start, bss_end;

  bss_start = (uint32_t) (&__bss_start);
  bss_end = (uint32_t) (&__bss_end);

  __asm__ __volatile__(
    "1:\n\t"
      ".set gp=64\n\t"
      ".set noreorder\n\t"
      "cache 0xD, 0x0(%0)\n\t"
      "addiu %0, %0, 0x10\n\t"
      "sd $zero, -0x10(%0)\n\t"
      "bne %0, %1, 1b\n\t"
      "sd $zero, -0x8(%0)\n\t"
      ".set reorder\n\t"
      ".set gp=default\n\t"

    : "=r" (bss_end)
    : "0" (bss_end), "r" (bss_start)
    : "memory"
  );
}

libn64func
__attribute__((noreturn)) void libn64_main(uint32_t kernel_sp) {
  uint32_t physmem_bottom, physmem_top;

  clear_bss();

  // Put the given physical memory region under control of the MM.
  // Both the top and the bottom addresses must be 4k aligned.
  physmem_bottom = (uint32_t) (&__bss_end) + 0x480 + 4095;
  physmem_top = kernel_sp - 4096;

  libn64_mm_init(physmem_bottom, physmem_top);

  // Hand control over to the application.
  // Set default thread stack addresses until we get something better.
  // Give each thread a 4K stack starting at 1MB (except the current thread).
  libn64_thread_init();

  libn64_thread_table->free_list[LIBN64_THREADS_MAX - 2]->state.regs[0x68/4] = 0x80180000;
  libn64_thread_create(main, NULL, LIBN64_THREAD_MIN_PRIORITY + 1);

  // This thread becomes the kernel thread.
  libn64_kthread();
}

