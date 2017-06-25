//
// libn64/priv_include/os/thread_queue.h: OS thread queue definitions.
//
// n64chain: A (free) open-source N64 development toolchain.
// Copyright 2014-16 Tyler J. Stachecki <stachecki.tyler@gmail.com>
//
// This file is subject to the terms and conditions defined in
// 'LICENSE', which is part of this source code package.
//

#ifndef LIBN64_PRIV_INCLUDE_OS_THREAD_QUEUE_H
#define LIBN64_PRIV_INCLUDE_OS_THREAD_QUEUE_H

#include <libn64.h>
#include <os/thread.h>

struct libn64_thread_queue_entry {
  struct libn64_thread_internal *thread;
  unsigned priority;
} __attribute__((aligned(8)));

// Metadata for the queue is kept in the first 8 bytes.
// The root of the tree also sits in the same cache line.
struct libn64_thread_queue {
  uint32_t tail_offset_in_bytes;

  // This field is volatile/reserved for use by the ISR.
  uint32_t isr_temp;

  // Each queue entry is 8b and each cache line is 16b. Make
  // sure the root is at +8b offset into the cache line so the
  // left/right children are wholly contained within a cache
  // line (to reduce the # of cache misses during dequeues).
  struct libn64_thread_queue_entry heap[LIBN64_THREADS_MAX];
} __attribute__((aligned(16)));

#endif

