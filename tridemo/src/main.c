//
// tridemo/src/main.c: RSP triangle uCode demo (entry point).
//
// n64chain: A (free) open-source N64 development toolchain.
// Copyright 2014-16 Tyler J. Stachecki <stachecki.tyler@gmail.com>
//
// This file is subject to the terms and conditions defined in
// 'LICENSE', which is part of this source code package.
//

#include <fbtext.h>
#include <stdint.h>
#include <syscall.h>
#include <rcp/vi.h>

__attribute__((section(".cart.ucode")))
extern const uint32_t __ucode[];

// These pre-defined values are suitable for NTSC.
// TODO: Add support for PAL and PAL-M televisions.
static vi_state_t vi_state = {
  0x0000324E, // status
  0x00200000, // origin
  0x00000140, // width
  0x00000002, // intr
  0x00000000, // current
  0x03E52239, // burst
  0x0000020D, // v_sync
  0x00000C15, // h_sync
  0x0C150C15, // leap
  0x006C02EC, // h_start
  0x002501FF, // v_start
  0x000E0204, // v_burst
  0x00000200, // x_scale
  0x00000400, // y_scale
};

struct libn64_fbtext_context fbtext;

void main(void *opaque __attribute__((unused))) {
  unsigned i;

  for (i = 0; i < 320 * 240 * 2; i += 16) {
    __asm__ __volatile__(
      ".set gp=64\n\t"
      "cache 0xD, 0x0(%0)\n\t"
      "sd $zero, 0x0(%0)\n\t"
      "sd $zero, 0x8(%0)\n\t"
      "cache 0x19, 0x0(%0)\n\t"
      ".set gp=default\n\t"

      :: "r" (0x80000000 | (vi_state.origin + i))
      : "memory"
    );
  }

  vi_flush_state(&vi_state);

  libn64_thread_reg_intr(libn64_thread_self(), LIBN64_INTERRUPT_DP);
  libn64_thread_reg_intr(libn64_thread_self(), LIBN64_INTERRUPT_SP);
  libn64_thread_reg_intr(libn64_thread_self(), LIBN64_INTERRUPT_VI);

  libn64_fbtext_init(&fbtext, 0x200000, LIBN64_FBTEXT_COLOR_WHITE,
      LIBN64_FBTEXT_COLOR_BLACK, 0x140, LIBN64_FBTEXT_16BPP);

  fbtext.x = fbtext.y = 2;
  libn64_fbtext_puts(&fbtext, "Submitting uCode...\n");

  uint32_t *rspIMem = (uint32_t *) (0xA4001000);
  uint32_t *rspStatus = (uint32_t *) (0xA4040010);
  uint32_t *rspPC = (uint32_t *) (0xA4080000);

  for (i = 0; i < 0x400; i++) {
    //__asm__ __volatile__("move $a0, %0\n\tsw $zero, 0x0($zero)\n\t" :: "r"( __ucode[0x400]));
    rspIMem[i] = __ucode[i + 0x400];
  }

  *rspPC = 0x4001000;
  *rspStatus = 0x5;

  while (1) {
    int message = libn64_recv_message();

    if (message == -2) {
      uint32_t *rdpCurrent = (uint32_t *) (0xA4100008);

      libn64_fbtext_puts(&fbtext, "  RDP finished.\n");
      libn64_fbtext_puts(&fbtext, "  DP_CURRENT: ");
      libn64_fbtext_putu32(&fbtext, *rdpCurrent);
      libn64_fbtext_puts(&fbtext, "\n");
    }

    else if (message == -4) {
      libn64_fbtext_puts(&fbtext, "  RSP finished.\n");
    }

    vi_flush_state(&vi_state);
  }
}

