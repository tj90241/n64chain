/*
 * libn64/rom.ld: Linker script for ROMs.
 *
 * n64chain: A (free) open-source N64 development toolchain.
 * Copyright 2014-16 Tyler J. Stachecki <stachecki.tyler@gmail.com>
 *
 * This file is subject to the terms and conditions defined in
 * 'LICENSE', which is part of this source code package.
 */

OUTPUT_FORMAT("elf32-bigmips")
OUTPUT_ARCH(mips)

EXTERN(libn64_ipl)
ENTRY(libn64_ipl)

SECTIONS {
  .text 0xFFFFFFFF80000400 : AT (0xFFFFFFFF80000400) {
    *(.text.libn64.ipl)
    *(.text.libn64.asm)
    *(.text.libn64);

    . = ALIGN(32);
    _ftext = .;

    *(.text.startup .text.startup.*)
    *(.text.unlikely .text.*_unlikely .text.unlikely.*)
    *(.text.exit .text.exit.*)
    *(.text.hot .text.hot.*)
    *(.text .stub .text.* .gnu.linkonce.t.*)
    *(.mips16.fn.*) *(.mips16.call.*)
  }

  .fini : {
    KEEP (*(SORT_NONE(.fini)))
  }

  . = ALIGN(32);
  PROVIDE (__etext = .);
  PROVIDE (_etext = .);
  PROVIDE (etext = .);

  /* DATA */
  .rodata : {
    *(.rodata.libn64 .rodata .rodata.* .gnu.linkonce.r.*)
  }

  .rodata1 : {
    *(.rodata1)
  }

  .sdata2 : {
    *(.sdata2 .sdata2.* .gnu.linkonce.s2.*)
  }

  __sbss2_start = .;
  .sbss2 : {
    *(.sbss2 .sbss2.* .gnu.linkonce.sb2.*)
  }
  __sbss2_end = .;

  .jcr : {
    KEEP (*(.jcr))
  }

  .data.rel.ro : {
    *(.data.rel.ro.local* .gnu.linkonce.d.rel.ro.local.*) *(.data.rel.ro .data.rel.ro.* .gnu.linkonce.d.rel.ro.*)
  }

  .data : {
    _fdata = . ;
    *(.data .data.* .gnu.linkonce.d.*)
  }

  .data1 : {
    *(.data1)
  }

  .got.plt : {
    *(.got.plt)
  }

  HIDDEN (_gp = ALIGN(16) + 0x7FF0);

  .got : {
    *(.got)
  }

  .sdata : {
    *(.sdata .sdata.* .gnu.linkonce.s.*)
  }

  .lit8 : {
    *(.lit8)
  }

  .lit4 : {
    *(.lit4)
  }

  . = ALIGN(16);
  _edata = .; PROVIDE (edata = .);

  /* BSS */
  . = ALIGN(16);
  __bss_start = .;
  _fbss = .;

  .sbss : {
    *(.dynsbss)
    *(.sbss .sbss.* .gnu.linkonce.sb.*)
    *(.scommon)
  }

  .bss : {
    *(.dynbss)
    *(.bss .bss.* .gnu.linkonce.b.*)
    *(COMMON)
  }

  . = ALIGN(16);
  __bss_end = .;

  /* Place the exception handlers that get DMA'd over the MIPS */
  /* exceptions vector immediately after the end of the BSS. */
  .exception 0xFFFFFFFF80000000 : AT (__bss_end) {
    *(.exception.tlbmiss)
    *(.exception.general)
    *(.exception.routines)
  }

  /* And, following that, any resources which live on the cart. */
  __exception_end = __bss_end + 0x400;

  .cart 0x30000C00 + __exception_end : AT(__exception_end) {
    *(.cart.libn64.*)
  }

  /* Everything is statically linked, so discard PLTs. */
  /DISCARD/ : { *(.rel.iplt) *(.rela.iplt) *(.rel.plt) *(.rela.plt) *(.plt) *(.iplt) }

  /* We don't make use of debugging information, so drop that, too. */
  /DISCARD/ : { *(.debug) *(.debug_srcinfo) *(.debug_sfnames) *(.debug_aranges) *(.debug_pubnames) *(.debug_info .gnu.linkonce.wi.*) *(.debug_abbrev) *(.debug_line .debug_line.* .debug_line_end ) *(.debug_frame) *(.debug_str) *(.debug_loc) *(.debug_macinfo) *(.debug_weaknames) *(.debug_funcnames) *(.debug_typenames) *(.debug_varnames) *(.debug_pubtypes) *(.debug_ranges) *(.debug_macro) *(.mdebug.abi32) *(.mdebug.abiN32) *(.mdebug.abi64) *(.mdebug.abiO64) *(.mdebug.eabi32) *(.mdebug.eabi64) }

  /* Discard things that the standard link script drops, too. */
  /DISCARD/ : { *(.note.GNU-stack) *(.gnu_debuglink) *(.gnu.lto_*) }

  /* Don't include MIPS ABI information with the ROM. */
  /DISCARD/ : { *(.MIPS.abiflags) }
}

