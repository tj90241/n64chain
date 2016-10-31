#
# libn64/os/asm/boot.s: libn64 IPL handoff.
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
.set noreorder

# -------------------------------------------------------------------
#  The IPL loads this to the entrypoint (0x80000400).
# -------------------------------------------------------------------
.global libn64_ipl
.type libn64_ipl, @function
.align 5
libn64_ipl:

# Tell the PIF to not throw a NMI.
  addiu $v0, $zero, 0x8
  lui $at, 0xBFC0
  sw $v0, 0x7FC($at)

# Setup a stack at the top of cached RAM.
  lui $sp, 0x8000
  lw $at, 0x318($sp)
  addu $sp, $sp, $at

# Reserve the necessary amount of space immediately above the stack
# for thread contexts and queues (0x200 bytes per thread, rounded up
# to the nearest 4kB page).
  li $v0, LIBN64_THREADS_MAX + 0x7
  srl $v0, $v0, 0x3
  sll $v0, $v0, 0xC
  subu $sp, $sp, $v0
  addu $a0, $sp, $zero

# Set the global pointer reference value.
  la $gp, _gp

# Set initial status register value.
  addiu $v0, $zero, 0x400
  mtc0 $v0, $12

# DMA interrupt handler on top of the vector.
# First, write the DRAM (destination) register.
  lui $at, 0xA460
  la $v0, (libn64_tlb_miss_exception_handler - 0x80000000)
  sw $v0, ($at)

# Next, convert the exception handler address into a PI
# cart address and write that to CART (source) register.
  la $v0, (__bss_end + 0xC00)
  lui $v1, 0x9000
  xor $v1, $v0, $v1
  sw $v1, 0x4($at)

# Finally, write out the length to start the DMA.
  addiu $v1, $zero, (0x480 - 0x1)
  sw $v1, 0xC($at)

# These next few instructions are fortunately in the same
# cache line, so they won't be clobbered by the DMA. We can
# DMA upto 0x480 bytes safely (anything else will begin
# overwriting libn64_init, which is not yet in the cache).
#
# Wait for the PI to finish DMA'ing the interrupt handler.
libn64_ipl_pi_wait:
  xori $v0, $v0, 0x8
  bnezl $v0, libn64_ipl_pi_wait
  lw $v0, 0x10($at)

# Clear the cause register and load $ra with the address
# where we can stop invalidating the cache (0x80000480).
  lui $v0, 0x8000
  jal libn64_init
  mtc0 $zero, $13

# -------------------------------------------------------------------
#  Initialize any libn64 components that cannot be done from C.
#  After that's done, branch to libn64's C entrypoint.
# -------------------------------------------------------------------
.align 5
libn64_init:
  addiu $v0, $v0, 0x20

# Invalidate the instruction cache where the exception
# vectors lie, since they likely contain cached code.
libn64_init_inval_icache:
  cache 0x0, -0x20($v0)
  bne $v0, $ra, libn64_init_inval_icache
  addiu $v0, $v0, 0x20

# Write out the address of the thread table (it's above the stack).
# Done from the ASM side of things; off to C to continue init'ing.
  la $at, libn64_thread_table
  j libn64_main
  sw $sp, ($at)

.size libn64_ipl,.-libn64_ipl

.set at
.set reorder

