/*
 * libn64/include/libn64.h: Global definitions.
 *
 * n64chain: A (free) open-source N64 development toolchain.
 * Copyright 2014-16 Tyler J. Stachecki <stachecki.tyler@gmail.com>
 *
 * This file is subject to the terms and conditions defined in
 * 'LICENSE', which is part of this source code package.
 */

#ifndef LIBN64_INCLUDE_LIBN64_H
#define LIBN64_INCLUDE_LIBN64_H

#ifndef __ASSEMBLER__
#define libn64func __attribute__ ((section (".text.libn64")))
#endif

#define LIBN64_THREADS_MAX 15

#endif

