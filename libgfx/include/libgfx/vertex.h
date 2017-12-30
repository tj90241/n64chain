//
// libgfx/include/libgfx/vertex.h: Vertex structure.
//
// n64chain: A (free) open-source N64 development toolchain.
// Copyright 2014-16 Tyler J. Stachecki <stachecki.tyler@gmail.com>
//
// This file is subject to the terms and conditions defined in
// 'LICENSE', which is part of this source code package.
//

#ifndef LIBGFX_INCLUDE_VERTEX_H
#define LIBGFX_INCLUDE_VERTEX_H

#include <stdint.h>

// This is how vertices are stored within the DRAM cache.
//
// While it would be nice to have multiple vertex formats, consider:
//   * The RSP can only do 8-byte aligned transfers
//   * The RSP must quickly calc vertex offset (powers of two...)
//
// So a vertex structure /needs/ to be either 8 or 16-bytes...
struct libgfx_vertex {
  int16_t x, y;         // 10.2 screen coordinates
  int16_t s, t;         // 10.5 texture coordinates
  int32_t z;            // 15.16 z-buffer depth

  union {
    uint32_t rgba32;
    uint8_t rgba[4];
  } color;
} __attribute__((aligned(16)));

// Write back the contents of the vertex (in cache) to DRAM.
// If the line has already been flushed, this is a NO-OP.
static inline void libgfx_vertex_flush(struct libgfx_vertex *v) {
  __builtin_mips_cache(0x19, v);
}

// Flushes the cache line that vertex would live in if it is
// dirty. Unconditionally flags the resulting cache line as
// 'dirty' and sets the physical tag without loading the
// current contents of the memory pointed to by 'v'.
static inline void libgfx_vertex_init(struct libgfx_vertex *v) {
  __builtin_mips_cache(0xD, v);
}

#endif

