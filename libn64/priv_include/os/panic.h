//
// libn64/priv_include/os/panic.h: Fatal crash handler.
//
// n64chain: A (free) open-source N64 development toolchain.
// Copyright 2014-16 Tyler J. Stachecki <stachecki.tyler@gmail.com>
//
// This file is subject to the terms and conditions defined in
// 'LICENSE', which is part of this source code package.
//

#ifndef LIBN64_PRIV_INCLUDE_OS_PANIC_H
#define LIBN64_PRIV_INCLUDE_OS_PANIC_H

#include <libn64.h>
#include <os/thread.h>

libn64func __attribute__((noreturn))
void libn64_panic_from_isr(void);

#endif

