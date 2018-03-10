//
// libn64/include/sp.h: Signal processor engine interface.
//
// n64chain: A (free) open-source N64 development toolchain.
// Copyright 2014-16 Tyler J. Stachecki <stachecki.tyler@gmail.com>
//
// This file is subject to the terms and conditions defined in
// 'LICENSE', which is part of this source code package.
//

#ifndef LIBN64_INCLUDE_SP_H
#define LIBN64_INCLUDE_SP_H

#include <libn64.h>
#include <mq.h>
#include <syscall.h>

enum libn64_sp_command {
  LIBN64_SP_RCP_INTERRUPT = -4,
  LIBN64_SP_CMD_INVALID = -1,
};

enum libn64_sp_response {
  LIBN64_SP_RESP_OK = 1,
  LIBN64_SP_RESP_ERROR = -1,
};

struct libn64_sp_request {
  uint32_t dest_address;
  uint32_t src_address;
  uint32_t size;

  struct libn64_mq *mq;
};

libn64func
static inline libn64_thread libn64_sp_get_thread(void) {
  libn64_thread sp_thread;

  __asm__ __volatile__(
    ".set noat\n\t"
    ".set gp=64\n\t"
    "lui $at, 0x8000\n\t"
    "lw %0, 0x458($at)\n\t"
    ".set gp=default\n\t"
    ".set at\n\t"

    : "=r" (sp_thread)
  );

  return sp_thread;
}

libn64func
static inline void libn64_sp_pack_request(struct libn64_sp_request *req,
    struct libn64_mq *mq, uint32_t dest_address, uint32_t src_address,
    uint32_t size) {
  req->dest_address = dest_address;
  req->src_address = src_address;
  req->size = size;
  req->mq = mq;
}

libn64func
static inline void libn64_sp_submit(uint32_t command,
    struct libn64_sp_request *request) {
  libn64_thread sp_thread = libn64_sp_get_thread();
  libn64_sendt_message1(sp_thread, command, request);
}

#endif

