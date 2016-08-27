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

struct libn64_thread_table *libn64_thread_table;

// Initialize the thread table.
void libn64_thread_init(void) {
  struct libn64_thread *self;
  unsigned i;

  // Initialize the thread stack.
  for (i = 0; i < LIBN64_THREADS_MAX; i++)
    libn64_thread_table->free_list[i] = libn64_thread_table->threads + i;

  libn64_thread_table->free_threads = LIBN64_THREADS_MAX - 1;

  // Initialize the ready thread queue and initial thread.
  self = libn64_thread_table->threads + libn64_thread_table->free_threads;
  libn64_thread_table->ready_queue.count = 1;

  libn64_thread_table->ready_queue.heap[0].priority = LIBN64_THREAD_MIN_PRIORITY;
  libn64_thread_table->ready_queue.heap[0].thread = self;
  self->priority = LIBN64_THREAD_MIN_PRIORITY;
}

