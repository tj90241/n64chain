#
# libn64/os/asm/idle_thread.s: libn64 idle thread.
#
# n64chain: A (free) open-source N64 development toolchain.
# Copyright 2014-16 Tyler J. Stachecki <stachecki.tyler@gmail.com>
#
# This file is subject to the terms and conditions defined in
# 'LICENSE', which is part of this source code package.
#

#include <libn64.h>

.section .text.libn64.ipl, "ax", @progbits

.set noat
.set gp=64
.set noreorder

# -------------------------------------------------------------------
#  This thread invokes main and then spins continuously.
# -------------------------------------------------------------------
.global libn64_idle_thread
.type libn64_idle_thread, @function
.align 5
libn64_idle_thread:
  la $a0, main
  xor $a1, $a1, $a2
  addiu $a2, $zero, 0x1
  xor $at, $at, $at
  syscall

libn64_idle_thread_spin:
  j libn64_idle_thread_spin
  nop

.size libn64_idle_thread,.-libn64_idle_thread

.set at
.set gp=default
.set reorder

