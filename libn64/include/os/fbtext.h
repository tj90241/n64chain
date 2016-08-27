//
// libn64/include/os/fbtext.h: Framebuffer text routines.
//
// n64chain: A (free) open-source N64 development toolchain.
// Copyright 2014-16 Tyler J. Stachecki <stachecki.tyler@gmail.com>
//
// This file is subject to the terms and conditions defined in
// 'LICENSE', which is part of this source code package.
//

#ifndef LIBN64_INCLUDE_OS_FBTEXT_H
#define LIBN64_INCLUDE_OS_FBTEXT_H

#include <libn64.h>
#include <stdint.h>

#define LIBN64_FBTEXT_COLOR_BG 0
#define LIBN64_FBTEXT_COLOR_FG 1

#define LIBN64_FBTEXT_COLOR_BLACK (0)
#define LIBN64_FBTEXT_COLOR_WHITE (~0)

enum libn64_fbtext_mode {
  LIBN64_FBTEXT_16BPP,
  LIBN64_FBTEXT_32BPP
};

struct libn64_fbtext_context {
  unsigned (*render_char)(const struct libn64_fbtext_context *, uint32_t, char);

  uint32_t colors[2];
  uint32_t fb_origin;
  uint16_t fb_width;
  uint8_t x, y;
};

// Initializes a framebuffer text rendering component.
//
// The framebuffer origin must be in the form of a physical
// address and aligned to 16 bytes (i.e., 0x100000 = FB @ 1MiB)
// The framebuffer width should be specified in terms of pixels.
libn64func
void libn64_fbtext_init(struct libn64_fbtext_context *context,
    uint32_t fb_origin, uint32_t fg_color, uint32_t bg_color,
    uint16_t fb_width, enum libn64_fbtext_mode mode);

// Methods for rendering values to the framebuffer at the context's current
// x and y position. Handles escape characters. Currently will write past the
// end of the framebuffer instead of scrolling it up (as probably expected).
libn64func
void libn64_fbtext_puts(struct libn64_fbtext_context *context,
    const char *string);

libn64func
void libn64_fbtext_putu32(struct libn64_fbtext_context *context,
    uint32_t u32);

#endif

