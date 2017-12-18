//
// libn64/include/mq.h: Message queues.
//
// n64chain: A (free) open-source N64 development toolchain.
// Copyright 2014-16 Tyler J. Stachecki <stachecki.tyler@gmail.com>
//
// This file is subject to the terms and conditions defined in
// 'LICENSE', which is part of this source code package.
//

#ifndef LIBN64_INCLUDE_MQ_H
#define LIBN64_INCLUDE_MQ_H

#include <libn64.h>
#include <stddef.h>
#include <stdint.h>
#include <syscall.h>

struct libn64_message {
  struct libn64_message *next;
  struct libn64_message *prev;
  uint32_t message;
  void *data;
} __attribute__((aligned(16)));

struct libn64_mq {
  struct libn64_message *tail;
  struct libn64_message *head;
  libn64_thread waiter;
  void *arg;
} __attribute__((aligned(16)));

libn64func
static inline struct libn64_mq *libn64_mq_create(void) {
  void *opaque = libn64_mq_alloc();
  struct libn64_mq *mq = (struct libn64_mq *) opaque;
  __builtin_mips_cache(0xD, mq);

  mq->head = mq->tail = NULL;
  mq->waiter = libn64_thread_self();
  return mq;
}

libn64func
static inline void libn64_mq_destroy(struct libn64_mq *mq) {
  struct libn64_message *message, *next;

  for (message = mq->head; message != mq->tail; message = next) {
    next = message->next;
    libn64_mq_free(message);
  }

  libn64_mq_free(mq);
}

#endif

