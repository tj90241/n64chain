//
// libgfx/src/rspbuf.c: RSP buffer.
//
// n64chain: A (free) open-source N64 development toolchain.
// Copyright 2014-16 Tyler J. Stachecki <stachecki.tyler@gmail.com>
//
// This file is subject to the terms and conditions defined in
// 'LICENSE', which is part of this source code package.
//

#include <libgfx/rspbuf.h>
#include <libgfx/vertex.h>
#include <rcp/sp.h>
#include <stdint.h>

// Flushes the RSP buffer structure to RSP DRAM.
void libgfx_rspbuf_flush(struct libgfx_rspbuf *rspbuf) {
  unsigned len = (rspbuf->tail - rspbuf->head + 0xF) & ~0xF;

  if (len > 0) {
    uint32_t dram_address;
    unsigned i;

    for (i = 0; i < len; i += 16)
      __builtin_mips_cache(0x19, rspbuf->buf + i);

    // Spin up an RSP DMA from RDRAM to DMEM.
    // TODO: In the future, this should be queued up or done from RSP.
    __builtin_memcpy(&dram_address, &rspbuf, sizeof(dram_address));
    dram_address = (dram_address & 0xFFFFFF) + rspbuf->head + 0x10;

    libn64_rsp_dma_to_rsp(0x04000010 + rspbuf->head, dram_address, len - 1);
    while (libn64_rsp_is_dma_pending());

    // Flushed everything; reset the buffer pointer;
    rspbuf->head = rspbuf->tail = 0;
  }
}

// Flushes the RSP vertices buffer structure to RSP DRAM.
void libgfx_rspbuf_flush_vertices(struct libgfx_rspbuf *rspbuf) {
  uint32_t dram_address;
  unsigned i;

  for (i = 0; i < sizeof(rspbuf->vertices) / sizeof(*rspbuf->vertices); i++)
    libgfx_vertex_flush(rspbuf->vertices + i);

  // Spin up an RSP DMA from RDRAM to DMEM.
  // TODO: In the future, this should be queued up or done from RSP.
  __builtin_memcpy(&dram_address, &rspbuf, sizeof(dram_address));
  dram_address = (dram_address & 0xFFFFFF) + 0xC00;

  libn64_rsp_dma_to_rsp(0x0400C00, dram_address, 0x400 - 1);
  while (libn64_rsp_is_dma_pending());
}

