//
// libgfx/include/libgfx/init.h: Graphics initialization.
//
// n64chain: A (free) open-source N64 development toolchain.
// Copyright 2014-16 Tyler J. Stachecki <stachecki.tyler@gmail.com>
//
// This file is subject to the terms and conditions defined in
// 'LICENSE', which is part of this source code package.
//

#ifndef LIBGFX_INCLUDE_LIBGFX_INIT_H
#define LIBGFX_INCLUDE_LIBGFX_INIT_H

#include <rcp/sp.h>
#include <stdint.h>

// Initialize the libgfx components.
void libgfx_init(void);

// Run the microcode and wait for it to complete.
static inline void libgfx_run(void) {
  libn64_rsp_set_pc(0x04001000);
  libn64_rsp_set_status(RSP_STATUS_CLEAR_HALT | RSP_STATUS_CLEAR_BROKE);
}

#endif

