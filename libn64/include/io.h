//
// libn64/include/io.h: I/O processing.
//
// n64chain: A (free) open-source N64 development toolchain.
// Copyright 2014-16 Tyler J. Stachecki <stachecki.tyler@gmail.com>
//
// This file is subject to the terms and conditions defined in
// 'LICENSE', which is part of this source code package.
//

#ifndef LIBN64_INCLUDE_IO_H
#define LIBN64_INCLUDE_IO_H

#include <libn64.h>
#include <mq.h>
#include <syscall.h>

enum libn64_io_command {
  LIBN64_IO_CMD_CART2RAM = 1,
  LIBN64_IO_RCP_INTERRUPT = -3,
  LIBN64_IO_CMD_INVALID = -1,
};

enum libn64_io_response {
  LIBN64_IO_RESP_OK = 1,
  LIBN64_IO_RESP_ERROR = -1,
};

struct libn64_io_request {
  uint32_t dest_address;
  uint32_t src_address;
  uint32_t size;

  struct libn64_mq *mq;
};

libn64func
static inline libn64_thread libn64_io_get_thread(void) {
  libn64_thread io_thread;

  __asm__ __volatile__(
    ".set noat\n\t"
    ".set gp=64\n\t"
    "lui $at, 0x8000\n\t"
    "lw %0, 0x450($at)\n\t"
    ".set gp=default\n\t"
    ".set at\n\t"

    : "=r" (io_thread)
  );

  return io_thread;
}

libn64func
static inline void libn64_io_pack_request(struct libn64_io_request *req,
    struct libn64_mq *mq, uint32_t dest_address, uint32_t src_address,
    uint32_t size) {
  req->dest_address = dest_address;
  req->src_address = src_address;
  req->size = size;
  req->mq = mq;
}

libn64func
static inline void libn64_io_submit(uint32_t command,
    struct libn64_io_request *request) {
  libn64_thread io_thread = libn64_io_get_thread();
  libn64_sendt_message1(io_thread, command, request);
}

#endif

