//
// libn64/os/kthread.c: libn64 kernel thread.
//
// n64chain: A (free) open-source N64 development toolchain.
// Copyright 2014-16 Tyler J. Stachecki <stachecki.tyler@gmail.com>
//
// This file is subject to the terms and conditions defined in
// 'LICENSE', which is part of this source code package.
//

#include <os/kthread.h>

void libn64_kthread(void) {
  while (1);
  __builtin_unreachable();
}

