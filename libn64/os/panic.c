//
// libn64/os/panic.c: Fatal crash handler.
//
// n64chain: A (free) open-source N64 development toolchain.
// Copyright 2014-16 Tyler J. Stachecki <stachecki.tyler@gmail.com>
//
// This file is subject to the terms and conditions defined in
// 'LICENSE', which is part of this source code package.
//

#include <libn64.h>
#include <os/fbtext.h>
#include <os/panic.h>
#include <rcp/vi.h>

// TODO: Detect non-NTSC N64s and adjust accordingly...
static const vi_state_t libn64_panic_vi_state = {
  0x0000324E, // status
  0x003DA800, // origin
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

void libn64_panic_from_isr(void) {
  __attribute__((section(".cart.libn64.panicstrs")))
  static const char *exception_strings[34] = {
    "Interrupt",
    "TLB Modification",
    "TLB Miss (Load/Fetch)",
    "TLB Miss (Store)",
    "Address Error (Load/Fetch)",
    "Address Error (Store)",
    "Bus Error (Fetch)",
    "Bus Error (Load/Store)",
    "Syscall",
    "Breakpoint",
    "Reserved Instruction",
    "Coprocessor Unusable",
    "Arithmetic Overflow",
    "Trap",
    "Reserved",
    "Floating-Point",
    "Reserved",
    "Reserved",
    "Reserved",
    "Reserved",
    "Reserved",
    "Reserved",
    "Reserved",
    "Watch",
    "Reserved",
    "Reserved",
    "Reserved",
    "Reserved",
    "Reserved",
    "Reserved",
    "Reserved",
    "Reserved",

    "libn64: Bad Virtual Address",
    "libn64: Exhausted Memory"
  };

  __attribute__((section(".cart.libn64.gprstrs")))
  static const char *gp_register_strs[] = {
    "$at:",
    " $v0:",
    " $v1:",
    "\n $a0:",

    " $a1:",
    " $a2:",
    "\n $a3:",
    " $t0:",

    " $t1:",
    "\n $t2:",
    " $t3:",
    " $t4:",

    "\n $t5:",
    " $t6:",
    " $t7:",
    "\n $s0:",

    " $s1:",
    " $s2:",
    "\n $s3:",
    " $s4:",

    " $s5:",
    "\n $s6:",
    " $s7:",
    " $t8:",

    "\n $t9:",
    " $gp:",
    " $sp:",
    "\n $fp:",

    " $ra:",
    " $pc:"
  };

  struct libn64_fbtext_context fbtext;
  uint32_t fb_cur, fb_end, i;

  register uint32_t sr __asm__ ("$k0");
  register uint32_t *context __asm__("$k1");

  // Wipe the framebuffer to black.
  fb_cur = libn64_panic_vi_state.origin | 0x80000000;
  fb_end = fb_cur + 2 * 320 * 240;

  __asm__ __volatile__(
    ".set gp=64\n\t"
    ".set noreorder\n\t"
    "subu %[fb_end], %[fb_end], %[fb_cur]\n\t"
    "1:\n\t"
      "addiu %[fb_end], %[fb_end], -0x10\n\t"
      "cache 0xD, 0x0(%[fb_cur])\n\t"
      "addiu %[fb_cur], %[fb_cur], 0x10\n\t"
      "sd $zero, -0x10(%[fb_cur])\n\t"
      "sd $zero, -0x8(%[fb_cur])\n\t"
      "bne %[fb_end], $zero, 1b\n\t"
      "cache 0x19, -0x10(%[fb_cur])\n\t"
    ".set reorder\n\t"
    ".set gp=default\n\t"

    : [fb_cur] "=&r" (fb_cur), [fb_end] "=&r" (fb_end)
    : "0" (fb_cur), "1" (fb_end)
    : "memory"
  );

  // Dump fault and state of the processor to the framebuffer.
  libn64_fbtext_init(&fbtext, 0x3DA800, LIBN64_FBTEXT_COLOR_WHITE,
      LIBN64_FBTEXT_COLOR_BLACK, 0x140, LIBN64_FBTEXT_16BPP);

  fbtext.x = 1; fbtext.y = 1;
  libn64_fbtext_puts(&fbtext, exception_strings[(sr & 0xFC) >> 2]);
  libn64_fbtext_puts(&fbtext, " Exception!\n\n ");

  // Swap a0 and t0...
  i = context[3];
  context[3] = context[7];
  context[7] = i;

  for (i = 0; i < sizeof(gp_register_strs) / sizeof(*gp_register_strs); i++) {
    libn64_fbtext_puts(&fbtext, gp_register_strs[i]);
    libn64_fbtext_putu32(&fbtext, context[i]);
  }

  // Flush the VI state and tie up the system.
  vi_flush_state(&libn64_panic_vi_state);

  while (1);
  __builtin_unreachable();
}

