#
# libn64/os/asm/thread.s: libn64 thread syscalls.
#
# n64chain: A (free) open-source N64 development toolchain.
# Copyright 2014-16 Tyler J. Stachecki <stachecki.tyler@gmail.com>
#
# This file is subject to the terms and conditions defined in
# 'LICENSE', which is part of this source code package.
#

#include <libn64.h>

.section .text.libn64, "ax", @progbits

.set noat
.set noreorder

# -------------------------------------------------------------------
#  Terminates the actively running thread.
# -------------------------------------------------------------------
.global libn64_thread_exit
.type libn64_thread_exit, @function
.align 1
libn64_thread_exit:
  addiu $at, $zero, 0x1
  syscall

.size libn64_thread_exit,.-libn64_thread_exit

.set at
.set reorder

