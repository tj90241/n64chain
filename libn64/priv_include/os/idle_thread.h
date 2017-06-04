//
// libn64/priv_include/os/idle_thread.h: libn64 idle thread.
//
// n64chain: A (free) open-source N64 development toolchain.
// Copyright 2014-16 Tyler J. Stachecki <stachecki.tyler@gmail.com>
//
// This file is subject to the terms and conditions defined in
// 'LICENSE', which is part of this source code package.
//

#include <libn64.h>

#ifndef LIBN64_PRIV_INCLUDE_OS_IDLE_THREAD_H
#define LIBN64_PRIV_INCLUDE_OS_IDLE_THREAD_H

libn64func __attribute__((noreturn))
void libn64_idle_thread(void);

#endif

