//
// libn64/os/fbtext.c: Framebuffer text routines.
//
// n64chain: A (free) open-source N64 development toolchain.
// Copyright 2014-16 Tyler J. Stachecki <stachecki.tyler@gmail.com>
//
// This file is subject to the terms and conditions defined in
// 'LICENSE', which is part of this source code package.
//

#include <libn64.h>
#include <os/fbtext.h>
#include <stdint.h>

// Methods for rendering a character to a 16 or 32-bit RGBA framebuffer.
// These methods do not handle escape characters and do not advance x/y.
// They are strictly intended to render a character.
libn64func
static inline const uint32_t *get_font_data(char c);

libn64func
static unsigned libn64_fbchar16(const struct libn64_fbtext_context *context,
    uint32_t fb_address, char c);

#ifdef LIBN64_FBTEXT_32BPP
libn64func
static unsigned libn64_fbchar32(const struct libn64_fbtext_context *context,
    uint32_t fb_address, char c);
#endif

// 95 member font table; starts with the ' ' char.
// This blob is licensed under the public domain.
const uint32_t *get_font_data(char c) {
  __attribute__((section(".cart.libn64.font")))
  static const uint32_t libn64_font_table[] __attribute__((aligned(4))) = {
    0x00000000,0x00000000,0x00000000,0x00000000, // <space>
    0x10101010,0x10101010,0x00001010,0x00001010, // '!'
    0x24242424,0x00002424,0x00000000,0x00000000, // '"'
    0x24242424,0x24247E7E,0x24247E7E,0x00002424, // '#'
    0x3C3C1010,0x38385050,0x78781414,0x00001010, // '$'
    0x62620000,0x08086464,0x26261010,0x00004646, // '%'
    0x48483030,0x30304848,0x44444A4A,0x00003A3A, // "&'
    0x10101010,0x00001010,0x00000000,0x00000000, // '''
    0x20201010,0x40404040,0x20204040,0x00001010, // '('
    0x08081010,0x04040404,0x08080404,0x00001010, // ')'
    0x54541010,0x10103838,0x54543838,0x00001010, // '*'
    0x10100000,0x7C7C1010,0x10101010,0x00000000, // '+'
    0x00000000,0x00000000,0x08080808,0x00001010, // ','
    0x00000000,0x7E7E0000,0x00000000,0x00000000, // '-'
    0x00000000,0x00000000,0x00000000,0x00001010, // '.'
    0x02020000,0x08080404,0x20201010,0x00004040, // '/'
    0x42423C3C,0x5A5A4646,0x42426262,0x00003C3C, // '0'
    0x18180808,0x08080808,0x08080808,0x00001C1C, // '1'
    0x42423C3C,0x1C1C0202,0x40402020,0x00007E7E, // '2'
    0x02027E7E,0x1C1C0404,0x42420202,0x00003C3C, // '3'
    0x0C0C0404,0x24241414,0x04047E7E,0x00000404, // '4'
    0x40407E7E,0x02027C7C,0x42420202,0x00003C3C, // '5'
    0x20201E1E,0x7C7C4040,0x42424242,0x00003C3C, // '6'
    0x02027E7E,0x08080404,0x10101010,0x00001010, // '7'
    0x42423C3C,0x3C3C4242,0x42424242,0x00003C3C, // '8'
    0x42423C3C,0x3E3E4242,0x04040202,0x00007878, // '9'
    0x00000000,0x00001010,0x00001010,0x00000000, // ':'
    0x00000000,0x00000808,0x08080808,0x00000010, // ';'
    0x08080404,0x20201010,0x08081010,0x00000404, // '<'
    0x00000000,0x00007E7E,0x00007E7E,0x00000000, // '='
    0x10102020,0x04040808,0x10100808,0x00002020, // '>'
    0x40402020,0x00000000,0x00000000,0x00000000, // '?'
    0x42423C3C,0x56564A4A,0x40404C4C,0x00003E3E, // '@'
    0x24241818,0x42424242,0x42427E7E,0x00004242, // 'A'
    0x42427C7C,0x7C7C4242,0x42424242,0x00007C7C, // 'B'
    0x42423C3C,0x40404040,0x42424040,0x00003C3C, // 'C'
    0x42427C7C,0x42424242,0x42424242,0x00007C7C, // 'D'
    0x40407E7E,0x7C7C4040,0x40404040,0x00007E7E, // 'E'
    0x40407E7E,0x7C7C4040,0x40404040,0x00004040, // 'F'
    0x40403E3E,0x4E404040,0x4242424E,0x00003E3E, // 'G'
    0x42424242,0x7E7E4242,0x42424242,0x00004242, // 'H'
    0x10103838,0x10101010,0x10101010,0x00003838, // 'I'
    0x02020202,0x02020202,0x42420202,0x00003C3C, // 'J'
    0x44444242,0x70704848,0x44444848,0x00004242, // 'K'
    0x40404040,0x40404040,0x40404040,0x00007E7E, // 'L'
    0x66664242,0x5A5A5A5A,0x42424242,0x00004242, // 'M'
    0x62624242,0x5A5A5252,0x46464A4A,0x00004242, // 'N'
    0x42423C3C,0x42424242,0x42424242,0x00003C3C, // 'O'
    0x42427C7C,0x7C7C4242,0x40404040,0x00004040, // 'P'
    0x42423C3C,0x42424242,0x44444A4A,0x00003A3A, // 'Q'
    0x42427C7C,0x7C7C4242,0x44444848,0x00004242, // 'R'
    0x42423C3C,0x3C3C4040,0x42420202,0x00003C3C, // 'S'
    0x10107C7C,0x10101010,0x10101010,0x00001010, // 'T'
    0x42424242,0x42424242,0x42424242,0x00003C3C, // 'U'
    0x42424242,0x42424242,0x24244242,0x00001818, // 'V'
    0x42424242,0x5A5A4242,0x66665A5A,0x00004242, // 'W'
    0x42424242,0x18182424,0x42422424,0x00004242, // 'X'
    0x44444444,0x10102828,0x10101010,0x00001010, // 'Y'
    0x02027E7E,0x18180404,0x40402020,0x00007E7E, // 'Z'
    0x60607E7E,0x60606060,0x60606060,0x00007E7E, // '['
    0x40400000,0x10102020,0x04040808,0x00000202, // '\'
    0x06067E7E,0x06060606,0x06060606,0x00007E7E, // ']'
    0x00000000,0x28281010,0x00004444,0x00000000, // '^'
    0x00000000,0x00000000,0x00000000,0x00007E7E, // '_'
    0x10102020,0x00000808,0x00000000,0x00000000, // '`'
    0x00000000,0x02023C3C,0x42423E3E,0x00003E3E, // 'a'
    0x40404040,0x42427C7C,0x42424242,0x00007C7C, // 'b'
    0x00000000,0x40403E3E,0x40404040,0x00003E3E, // 'c'
    0x02020202,0x42423E3E,0x42424242,0x00003E3E, // 'd'
    0x00000000,0x42423C3C,0x40407E7E,0x00003E3E, // 'e'
    0x22221C1C,0x7C7C2020,0x20202020,0x00002020, // 'f'
    0x00000000,0x42423C3C,0x3E3E4242,0x3C3C0202, // 'g'
    0x40404040,0x42427C7C,0x42424242,0x00004242, // 'h'
    0x00001010,0x10103030,0x10101010,0x00003838, // 'i'
    0x00000404,0x04043C3C,0x04040404,0x38384444, // 'j'
    0x40404040,0x44444242,0x44447878,0x00004242, // 'k'
    0x10103030,0x10101010,0x10101010,0x00003838, // 'l'
    0x00000000,0x5A5A6666,0x5A5A5A5A,0x00004242, // 'm'
    0x00000000,0x42427C7C,0x42424242,0x00004242, // 'n'
    0x00000000,0x42423C3C,0x42424242,0x00003C3C, // 'o'
    0x00000000,0x42427C7C,0x7C7C4242,0x40404040, // 'p'
    0x00000000,0x42423E3E,0x3E3E4242,0x02020202, // 'q'
    0x00000000,0x60605E5E,0x40404040,0x00004040, // 'r'
    0x00000000,0x40403E3E,0x02023C3C,0x00007C7C, // 's'
    0x10101010,0x10107C7C,0x12121010,0x00000C0C, // 't'
    0x00000000,0x42424242,0x46464242,0x00003A3A, // 'u'
    0x00000000,0x42424242,0x24244242,0x00001818, // 'v'
    0x00000000,0x42424242,0x5A5A5A5A,0x00006666, // 'w'
    0x00000000,0x24244242,0x24241818,0x00004242, // 'x'
    0x00000000,0x42424242,0x3E3E4242,0x3C3C0202, // 'y'
    0x00000000,0x04047E7E,0x20201818,0x00007E7E, // 'z'
    0x18180E0E,0x70701818,0x18181818,0x00000E0E, // '{'
    0x10101010,0x10101010,0x10101010,0x00001010, // '|'
    0x18187070,0x0E0E1818,0x18181818,0x00007070, // '}'
    0x00000000,0x54542424,0x00004848,0x00000000, // '-'
  };
  
  return libn64_font_table + ((c - ' ') << 2);
}

unsigned libn64_fbchar16(const struct libn64_fbtext_context *context,
    uint32_t fb_address, char c) {
  const uint32_t *font = get_font_data(c);
  unsigned i, j;

  fb_address += (context->x << 4);

  for (i = 0; i < 16; i++) {
    uint32_t bitmask = bitmask;

    if ((i & 3) == 0)
      bitmask = font[i >> 2];

    // Flush line contents if valid; otherwise flag it dirty.
    // This prevents an otherwise redundant read from memory.
    __builtin_mips_cache(0xD, (volatile void *) fb_address);

    for (fb_address += 8 << 1, j = 0; j < 8; j++, bitmask >>= 1) {
      __asm__ __volatile__(
        "sh %1, -2(%2)\n\t"
        "addiu %0, %2, -0x2"
        : "=r" (fb_address)
        : "r"(context->colors[bitmask & 0x1]), "0"(fb_address)
        : "memory"
      );
    }

    // Ensure the line gets written to memory.
    __builtin_mips_cache(0x19, (volatile void *) fb_address);
    fb_address += context->fb_width;
  }

  return 1;
}

#ifdef LIBN64_FBTEXT_32BPP
unsigned libn64_fbchar32(const struct libn64_fbtext_context *context,
    uint32_t fb_address, char c) {
  const uint32_t *font = get_font_data(c);
  unsigned i, j;

  fb_address += (context->x << 5);

  for (i = 0; i < 16; i++) {
    uint32_t bitmask = bitmask;

    if ((i & 3) == 0)
      bitmask = font[i >> 2];

    // Flush line contents if valid; otherwise flag it dirty.
    // This prevents an otherwise redundant read from memory.
    __builtin_mips_cache(0xD, (volatile void *) fb_address);
    __builtin_mips_cache(0xD, (volatile void *) (fb_address + 0x10));

    for (fb_address += 8 << 2, j = 0; j < 8; j++, bitmask >>= 1) {
      __asm__ __volatile__(
        "sw %1, -4(%2)\n\t"
        "addiu %0, %2, -0x4"
        : "=r" (fb_address)
        : "r"(context->colors[bitmask & 0x1]), "0"(fb_address)
        : "memory"
      );
    }

    // Ensure the line gets written to memory.
    __builtin_mips_cache(0x19, (volatile void *) fb_address);
    __builtin_mips_cache(0x19, (volatile void *) (fb_address + 0x10));

    fb_address += context->fb_width;
  }

  return 2;
}
#endif

void libn64_fbtext_init(struct libn64_fbtext_context *context,
    uint32_t fb_origin, uint32_t fg_color, uint32_t bg_color,
    uint16_t fb_width, enum libn64_fbtext_mode mode) {
  context->colors[LIBN64_FBTEXT_COLOR_BG] = bg_color;
  context->colors[LIBN64_FBTEXT_COLOR_FG] = fg_color;

  context->fb_origin = fb_origin | 0x80000000;
  context->fb_width = fb_width << mode;
  context->x = 0;
  context->y = 0;

#ifdef LIBN64_FBTEXT_32BPP
  context->render_char = mode == LIBN64_FBTEXT_16BPP
    ? libn64_fbchar16
    : libn64_fbchar32;
#endif
}

void libn64_fbtext_puts(struct libn64_fbtext_context *context,
    const char *string) {
  uint32_t fb_address;
  unsigned i;

  for (i = 0; string[i] != '\0'; i++) {
    unsigned mode;

    switch (string[i]) {
      case '\n':
        context->x = 0;
        context->y++;
        continue;

      default:
        break;
    }

    fb_address = context->fb_origin + context->fb_width * (context->y << 4);
#ifdef LIBN64_FBTEXT_32BPP
    mode = context->render_char(context, fb_address, string[i]);
#else
    mode = libn64_fbchar16(context, fb_address, string[i]);
#endif
    context->x++;

    if ((context->x + 1) > context->fb_width / (8 << mode)) {
      context->x = 0;
      context->y++;
    }
  }
}

void libn64_fbtext_putu32(struct libn64_fbtext_context *context,
    uint32_t u32) {
  char string[9];
  int i;

  for (string[8] = '\0', i = 7; i >= 0; i--) {
    uint8_t byte = u32 & 0xF;

    string[i] = byte + (byte < 0xA ? '0' : ('A' - 10));
    u32 >>= 4;
  }

  libn64_fbtext_puts(context, string);
}

