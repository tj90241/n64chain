//
// libn64/include/syscall.h: System call definitions.
//
// n64chain: A (free) open-source N64 development toolchain.
// Copyright 2014-16 Tyler J. Stachecki <stachecki.tyler@gmail.com>
//
// This file is subject to the terms and conditions defined in
// 'LICENSE', which is part of this source code package.
//

#ifndef LIBN64_INCLUDE_OS_SYSCALL_H
#define LIBN64_INCLUDE_OS_SYSCALL_H

// Syscall numbers.
#define LIBN64_SYSCALL_CREATE_THREAD 0
#define LIBN64_SYSCALL_EXIT_THREAD   1
#define LIBN64_SYSCALL_PAGE_ALLOC    2
#define LIBN64_SYSCALL_PAGE_FREE     3
#define LIBN64_SYSCALL_INVALID       4

#ifndef __ASSEMBLER__
#include <stdint.h>

// Spawns a new thread with a given priority (which receives arg). If
// the new thread has a higher priority than the current thread, a
// context switch will result from this function call.
//
// If an insufficient number of threads are available (due to a hard
// limit, or if too many threads are waiting to be reaped), a non-zero
// value is returned. Otherwise, zero is returned to indicate success.
libn64func __attribute__((always_inline))
static inline int libn64_thread_create(void (*entrypoint)(void *),
    void *arg, unsigned priority) {
  register uint32_t rv __asm__("$v0");
  register void (*a0)(void *) __asm__("$a0") = entrypoint;
  register void *a1 __asm__("$a1") = arg;
  register unsigned a2 __asm__("$a2") = priority;

  __asm__ __volatile__(
    ".set noreorder\n\t"
    ".set noat\n\t"
    "li $at, %4\n\t"
    "syscall\n\t"
    ".set reorder\n\t"
    ".set at\n\t"

    : "=r" (rv)
    : "r" (a0), "r" (a1), "r" (a2), "K" (LIBN64_SYSCALL_CREATE_THREAD)
    : "memory"
  );

  return rv;
}

// Terminates the currently running thread.
libn64func __attribute__((always_inline)) __attribute__((noreturn))
static inline void libn64_thread_exit(void) {
  __asm__ __volatile__(
    ".set noreorder\n\t"
    ".set noat\n\t"
    "li $at, %0\n\t"
    "syscall\n\t"
    ".set reorder\n\t"
    ".set at\n\t"

    :: "K" (LIBN64_SYSCALL_EXIT_THREAD)
    : "memory"
  );

  __builtin_unreachable();
}

// Allocates a page (4kB) of memory. If there are no available/free pages in
// the page allocator, then a fatal exception is raised as a result.
libn64func __attribute__((always_inline))
static inline void *libn64_page_alloc(void) {
  void *p;

  __asm__ __volatile__(
    ".set noreorder\n\t"
    ".set noat\n\t"
    "li $at, %1\n\t"
    "syscall\n\t"
    "addu %0, $at, $zero\n\t"
    ".set reorder\n\t"
    ".set at\n\t"

    : "=r"(p)
    : "K" (LIBN64_SYSCALL_PAGE_ALLOC)
    : "memory"
  );

  return p;
}

// Frees a page of memory allocated with libn64_page_alloc().
libn64func __attribute__((always_inline))
static inline void libn64_page_free(void *p) {
  register void *a0 __asm__("$a0") = p;

  __asm__ __volatile__(
    ".set noreorder\n\t"
    ".set noat\n\t"
    "li $at, %1\n\t"
    "syscall\n\t"
    ".set reorder\n\t"
    ".set at\n\t"

    :: "r" (a0), "K" (LIBN64_SYSCALL_PAGE_FREE)
    : "memory"
  );
}

#endif
#endif

