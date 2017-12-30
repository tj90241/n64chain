//
// libgfx/include/libgfx/rdpcmd.h: RDP commands.
//
// n64chain: A (free) open-source N64 development toolchain.
// Copyright 2014-16 Tyler J. Stachecki <stachecki.tyler@gmail.com>
//
// This file is subject to the terms and conditions defined in
// 'LICENSE', which is part of this source code package.
//

#ifndef LIBGFX_INCLUDE_LIBGFX_RDPCMD_H
#define LIBGFX_INCLUDE_LIBGFX_RDPCMD_H

#include <libgfx/rspbuf.h>
#include <stdint.h>

// This file provides a bunch of functions which pack RAW RDP fields.
// An interface that is a bit more intelligent about cramming values
// should come later...
#define COLOR_ELEMENT_4B          0
#define COLOR_ELEMENT_8B          1
#define COLOR_ELEMENT_16B         2
#define COLOR_ELEMENT_32B         3

#define CYCLE_TYPE_1CYCLE         (0ULL << 52)
#define CYCLE_TYPE_2CYCLE         (1ULL << 52)
#define CYCLE_TYPE_COP            (2ULL << 52)
#define CYCLE_TYPE_FILL           (3ULL << 52)

#define FORMAT_RGBA               0
#define FORMAT_YUV                1
#define FORMAT_COLOR_INDEX        2
#define FORMAT_IA                 3
#define FORMAT_I                  4

#define SCISSOR_NO_INTERLACED     0
#define SCISSOR_INTERLACED        1
#define SCISSOR_DONTCARE          0
#define SCISSOR_KEEPEVEN          0
#define SCISSOR_KEEPODD           1

static inline uint32_t rdp_rgba16(
    uint8_t r, uint8_t g, uint8_t b, uint8_t a) {
  uint16_t rgba16 = (r << 8) | ((g & 0xF8) << 3) | ((b & 0xF8) >> 2) | (a >> 7);
  return (rgba16 << 16) | rgba16;
}

static inline uint32_t rdp_rgba32(
    uint8_t r, uint8_t g, uint8_t b, uint8_t a) {
  return (r << 24) | (g << 16) | (b << 8) | a;
}

static inline void rsp_finalize(struct libgfx_rspbuf *rspbuf) {
  libgfx_rspbuf_append(rspbuf, 0x00000000);
}

// The commands that follow are uCode commands. uCode commands can
// be quickly identified because they are always signed number (i.e.,
// bit 31 is signed).

// RSP Draw Triangle (given three vertex cache indexes)
static inline void rsp_draw_triangle(struct libgfx_rspbuf *rspbuf,
    unsigned vert1, unsigned vert2, unsigned vert3) {

  libgfx_rspbuf_append(rspbuf, 0xF000000F |
      (vert1 << 20) | (vert2 << 12) | (vert3 << 4));
}

// The commands that follow are RDP commands. An interface which is
// a bit more intelligent about packing them should come in the future...

// RDP Fill Rectangle
static inline void rdp_fill_rectangle(struct libgfx_rspbuf *rspbuf,
    float xh, float yh, float xl, float yl) {

  uint16_t xhsc = xh * 4;
  uint16_t yhsc = yh * 4;
  uint16_t xlsc = xl * 4;
  uint16_t ylsc = yl * 4;

  libgfx_rspbuf_append(rspbuf, 0x36000000 |
      ((xlsc & 0xFFF) << 12) | (ylsc & 0xFFF));

  libgfx_rspbuf_append(rspbuf,
      ((xhsc & 0xFFF) << 12) | (yhsc & 0xFFF));
}

// RDP Set Blend Color
static inline void rdp_set_blend_color(struct libgfx_rspbuf *rspbuf,
    uint32_t rgba32) {

  libgfx_rspbuf_append(rspbuf, 0x39000000);
  libgfx_rspbuf_append(rspbuf, rgba32);
}

// RDP Set Color Image
static inline void rdp_set_color_image(struct libgfx_rspbuf *rspbuf,
    unsigned format, unsigned size, unsigned width, uint32_t fb_origin) {

  libgfx_rspbuf_append(rspbuf, 0x3F000000 |
      (format << 21) | (size << 19) | width);

  libgfx_rspbuf_append(rspbuf, fb_origin);
}

// RDP Set Fill Color
static inline void rdp_set_fill_color(struct libgfx_rspbuf *rspbuf,
    uint32_t rgba) {

  libgfx_rspbuf_append(rspbuf, 0x37000000);
  libgfx_rspbuf_append(rspbuf, rgba);
}

// RDP Set Other Modes
static inline void rdp_set_other_modes(struct libgfx_rspbuf *rspbuf,
    uint64_t bits) {

  uint32_t bits_hi = bits >> 32;
  uint32_t bits_lo = bits;

  libgfx_rspbuf_append(rspbuf, 0x2F000000 | bits_hi);
  libgfx_rspbuf_append(rspbuf, bits_lo);
}

// RDP Set Scissor
static inline void rdp_set_scissor(struct libgfx_rspbuf *rspbuf,
    float xh, float yh, float xl, float yl, unsigned f, unsigned o) {

  uint16_t xhsc = xh * 4;
  uint16_t yhsc = yh * 4;
  uint16_t xlsc = xl * 4;
  uint16_t ylsc = yl * 4;

  libgfx_rspbuf_append(rspbuf, 0x2D000000 |
      ((xhsc & 0xFFF) << 12) | (yhsc & 0xFFF));

  libgfx_rspbuf_append(rspbuf, (f << 25) | (o << 24) |
      ((xlsc & 0xFFF) << 12) | (ylsc & 0xFFF));
}

// RDP Sync Full
static inline void rdp_sync_full(struct libgfx_rspbuf *rspbuf) {
  libgfx_rspbuf_append(rspbuf, 0x29000000);
  libgfx_rspbuf_append(rspbuf, 0x00000000);
}

// RDP Sync Load
static inline void rdp_sync_load(struct libgfx_rspbuf *rspbuf) {
  libgfx_rspbuf_append(rspbuf, 0x26000000);
  libgfx_rspbuf_append(rspbuf, 0x00000000);
}

// RDP Sync Pipe
static inline void rdp_sync_pipe(struct libgfx_rspbuf *rspbuf) {
  libgfx_rspbuf_append(rspbuf, 0x27000000);
  libgfx_rspbuf_append(rspbuf, 0x00000000);
}

// RDP Sync Tile
static inline void rdp_sync_tile(struct libgfx_rspbuf *rspbuf) {
  libgfx_rspbuf_append(rspbuf, 0x28000000);
  libgfx_rspbuf_append(rspbuf, 0x00000000);
}

#endif

