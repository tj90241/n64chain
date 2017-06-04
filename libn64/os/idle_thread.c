//
// libn64/os/idle_thread.c: libn64 idle thread.
//
// n64chain: A (free) open-source N64 development toolchain.
// Copyright 2014-16 Tyler J. Stachecki <stachecki.tyler@gmail.com>
//
// This file is subject to the terms and conditions defined in
// 'LICENSE', which is part of this source code package.
//

#include <os/idle_thread.h>

void libn64_idle_thread(void) {
  while (1);
  __builtin_unreachable();
}

