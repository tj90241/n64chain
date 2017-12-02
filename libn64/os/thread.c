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
#include <stddef.h>

// Initialize the thread table.
libn64_thread libn64_thread_early_init(uint32_t kernel_sp) {
  struct libn64_thread_internal *self;
  unsigned i;

  // Determine the address of the thread table and thread block.
  struct libn64_thread_table *thread_table =
    (struct libn64_thread_table *) kernel_sp;

  struct libn64_thread_internal *thread_block =
    (struct libn64_thread_internal*) (kernel_sp + LIBN64_THREADS_MAX * 0x10);

  // Invalidate the thread message cache and interrupt chains.
  __asm__ __volatile__(
    ".set noat\n\t"
    ".set gp=64\n\t"
    "lui $at, 0x8000\n\t"
    "sw $zero, 0x424($at)\n\t"
    "sd $zero, 0x430($at)\n\t"
    "sd $zero, 0x438($at)\n\t"
    "sd $zero, 0x440($at)\n\t"
    ".set gp=default\n\t"
    ".set at\n\t"
  );

  // Initialize the thread stack.
  for (i = 0; i < LIBN64_THREADS_MAX; i++)
    thread_table->free_list[i] = thread_block + i;

  unsigned free_threads = LIBN64_THREADS_MAX - 1;
  thread_table->free_thread_offset_in_bytes = free_threads << 2;

  // Initialize the ready thread queue and initial thread.
  self = thread_block + free_threads;
  thread_table->ready_queue.tail_offset_in_bytes = 1 << 3;

  thread_table->ready_queue.heap[0].priority = LIBN64_THREAD_MIN_PRIORITY;
  thread_table->ready_queue.heap[0].thread = self;

  self->priority = LIBN64_THREAD_MIN_PRIORITY;
  self->messages_head = self->messages_tail = NULL;
  self->state.mi_intr_reg = 0xAAA;

  // Enable interrupts going forward.
  __asm__ __volatile__(
    ".set noat\n\t"
    ".set noreorder\n\t"
    "srl $at, %0, 0x9\n\t"
    "andi $at, $at, 0xFF\n\t"
    "mtc0 $at, $10\n\t"
    "xori $at, $zero, 0x8401\n\t"
    "mtc0 $at, $12\n\t"
    ".set reorder\n\t"
    ".set at\n\t"

    :: "r"(self)
    : "memory"
  );

  return self;
}

