//
// libgfx/src/init.c: Graphics initialization.
//
// n64chain: A (free) open-source N64 development toolchain.
// Copyright 2014-16 Tyler J. Stachecki <stachecki.tyler@gmail.com>
//
// This file is subject to the terms and conditions defined in
// 'LICENSE', which is part of this source code package.
//

#include <libgfx/init.h>
#include <rcp/sp.h>
#include <stdint.h>

void libgfx_init(void) {
  extern char libn64_ucode_gfx;
  static const char *ucode_ptr = &libn64_ucode_gfx;
  uint32_t ucode_address;

  libn64_rsp_set_status(RSP_STATUS_SET_HALT);

  // DMA the initialization ucode to the RSP and spin until its done.
  __builtin_memcpy(&ucode_address, &ucode_ptr, sizeof(ucode_address));
  ucode_address &= 0xFFFFFF;

  while (libn64_rsp_is_dma_pending());
  libn64_rsp_dma_to_rsp(0x04001000, ucode_address + 0x1000, 0xFFF);
  while (libn64_rsp_is_dma_pending());
}

