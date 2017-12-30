//
// libgfx/include/libgfx/rspbuf.h: RSP buffer.
//
// n64chain: A (free) open-source N64 development toolchain.
// Copyright 2014-16 Tyler J. Stachecki <stachecki.tyler@gmail.com>
//
// This file is subject to the terms and conditions defined in
// 'LICENSE', which is part of this source code package.
//

#ifndef LIBGFX_INCLUDE_LIBGFX_RSPBUF_H
#define LIBGFX_INCLUDE_LIBGFX_RSPBUF_H

#include <libgfx/vertex.h>
#include <stdint.h>
#include <syscall.h>

// Both the VR4300 and RSP use the first 16 bytes for "private"
// storage. The rest of the struct is mapped 1:1 to RSP DMEM.
struct libgfx_rspbuf {
  uint16_t head, tail;
  uint32_t unused[3];

  uint8_t buf[0xC00 - 0x10];
  struct libgfx_vertex vertices[0x400 / sizeof(struct libgfx_vertex)];
};

// Appends a word to the RSP buffer structure.
// Does NOT check for overflow of the buffer.
static inline void libgfx_rspbuf_append(
    struct libgfx_rspbuf *rspbuf, uint32_t word) {
  uint8_t *dest = rspbuf->buf + rspbuf->tail;

  __builtin_memcpy(dest, &word, sizeof(word));
  rspbuf->tail += sizeof(word);
}

// Allocates and initializes a new RSP buffer structure.
static inline struct libgfx_rspbuf *libgfx_rspbuf_create(void) {
  void *page = libn64_page_alloc();
  struct libgfx_rspbuf *rspbuf = (struct libgfx_rspbuf *) page;

  rspbuf->head = rspbuf->tail = 0;
  return rspbuf;
}

// Frees an existing RSP buffer structure.
static inline void libgfx_rspbuf_destroy(struct libgfx_rspbuf *rspbuf) {
  libn64_page_free(rspbuf);
}

// Flushes the RSP buffer structure to RSP DRAM.
void libgfx_rspbuf_flush(struct libgfx_rspbuf *rspbuf);
void libgfx_rspbuf_flush_vertices(struct libgfx_rspbuf *rspbuf);

#endif

