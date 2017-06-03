//
// libn64/include/os/thread_table.h: OS thread table definitions.
//
// n64chain: A (free) open-source N64 development toolchain.
// Copyright 2014-16 Tyler J. Stachecki <stachecki.tyler@gmail.com>
//
// This file is subject to the terms and conditions defined in
// 'LICENSE', which is part of this source code package.
//

#ifndef LIBN64_INCLUDE_OS_THREAD_TABLE_H
#define LIBN64_INCLUDE_OS_THREAD_TABLE_H

#include <libn64.h>
#include <os/thread.h>
#include <os/thread_queue.h>

struct libn64_thread_table {
  struct libn64_thread_queue ready_queue;
  struct libn64_thread *blocked_list;
  unsigned free_threads;

  struct libn64_thread *free_list[LIBN64_THREADS_MAX];
  struct libn64_thread threads[LIBN64_THREADS_MAX];
};

#endif

