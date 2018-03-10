//
// libn64/sp/init.c: Signal processor (SP) initialization.
//
// n64chain: A (free) open-source N64 development toolchain.
// Copyright 2014-16 Tyler J. Stachecki <stachecki.tyler@gmail.com>
//
// This file is subject to the terms and conditions defined in
// 'LICENSE', which is part of this source code package.
//

#include <libn64.h>
#include <rcp/sp.h>
#include <sp/init.h>
#include <sp/sp_thread.h>
#include <stddef.h>
#include <syscall.h>

libn64func
void libn64_sp_init(void) {
  extern char libn64_ucode_init;
  static const char *ucode_ptr = &libn64_ucode_init;
  uint32_t ucode_address;

  libn64_rsp_set_status(RSP_STATUS_SET_HALT);
  while (libn64_rsp_is_dma_pending());

  // DMA the initialization ucode to the RSP and spin until its done.
  __builtin_memcpy(&ucode_address, &ucode_ptr, sizeof(ucode_address));
  ucode_address &= 0xFFFFFF;

  libn64_rsp_dma_to_rsp(0x04000000, ucode_address + 0x0000, 0xFFF);
  while (libn64_rsp_is_dma_pending());

  libn64_rsp_dma_to_rsp(0x04001000, ucode_address + 0x1000, 0xFFF);
  while (libn64_rsp_is_dma_pending());

  // Execute the ucode and wait for it to complete.
  libn64_rsp_set_pc(0x04001000);
  libn64_rsp_set_status(RSP_STATUS_CLEAR_HALT | RSP_STATUS_CLEAR_BROKE);
  while ((libn64_rsp_get_status() & 0x3) != 0x3);

  // Start thread
  libn64_thread sp_thread;

  sp_thread = libn64_thread_create(libn64_sp_thread, NULL,
      LIBN64_THREAD_MAX_PRIORITY);

  // Store the thread address in the global block.
  __asm__ __volatile__(
    ".set noat\n\t"
    ".set gp=64\n\t"
    "lui $at, 0x8000\n\t"
    "sw %0, 0x458($at)\n\t"
    ".set gp=default\n\t"
    ".set at\n\t"

    :: "r" (sp_thread)
    : "memory"
  );
}

