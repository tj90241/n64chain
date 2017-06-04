//
// libn64/priv_include/os/message.h: OS message definition.
//
// n64chain: A (free) open-source N64 development toolchain.
// Copyright 2014-16 Tyler J. Stachecki <stachecki.tyler@gmail.com>
//
// This file is subject to the terms and conditions defined in
// 'LICENSE', which is part of this source code package.
//

#ifndef LIBN64_PRIV_INCLUDE_OS_MESSAGE_H
#define LIBN64_PRIV_INCLUDE_OS_MESSAGE_H

#include <stdint.h>

struct libn64_message {
  struct libn64_message *next;
  uint32_t message;
  uint32_t data[2];
} __attribute__((aligned(16)));

#endif

