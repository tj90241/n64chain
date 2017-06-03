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
#include <os/thread.h>
#include <os/thread_table.h>

// Returns a pointer to the libn64_thread_table structure.
libn64func __attribute__((always_inline))
static inline struct libn64_thread_table *libn64_get_thread_table(void) {
  struct libn64_thread_table *thread_table;

  __asm__(
    "lui %0, 0x8000\n\t"
    "lw %0, 0x420(%0)\n\t"
    : "=r"(thread_table)
  );

  return thread_table;
}

// Initialize the thread table.
void libn64_thread_early_init(void) {
  struct libn64_thread_table *thread_table = libn64_get_thread_table();
  struct libn64_thread *self;
  unsigned i;

  // Initialize the thread stack.
  for (i = 0; i < LIBN64_THREADS_MAX; i++)
    thread_table->free_list[i] = thread_table->threads + i;

  thread_table->free_threads = LIBN64_THREADS_MAX - 1;

  // Initialize the ready thread queue and initial thread.
  self = thread_table->threads + thread_table->free_threads;
  thread_table->ready_queue.count = 1;

  thread_table->ready_queue.heap[0].priority = LIBN64_THREAD_MIN_PRIORITY;
  thread_table->ready_queue.heap[0].thread = self;
  self->priority = LIBN64_THREAD_MIN_PRIORITY;
}

