//
// libn64/include/time.h: time.
//
// n64chain: A (free) open-source N64 development toolchain.
// Copyright 2014-16 Tyler J. Stachecki <stachecki.tyler@gmail.com>
//
// This file is subject to the terms and conditions defined in
// 'LICENSE', which is part of this source code package.
//

#ifndef LIBN64_INCLUDE_TIME_H
#define LIBN64_INCLUDE_TIME_H

#include <stdint.h>

struct timeval {
  uint32_t tv_sec;
  uint32_t tv_usec;
};

#endif

