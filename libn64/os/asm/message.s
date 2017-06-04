#
# libn64/os/asm/message.s: libn64 message passing routines.
#
# n64chain: A (free) open-source N64 development toolchain.
# Copyright 2014-16 Tyler J. Stachecki <stachecki.tyler@gmail.com>
#
# This file is subject to the terms and conditions defined in
# 'LICENSE', which is part of this source code package.
#

.section .text.libn64, "ax", @progbits

.set gp=64
.set fp=64
.set noat
.set noreorder

# -------------------------------------------------------------------
#  Receives a message ($a0/message buffer); blocks if nothing available.
#  A slight optimization: we assume that $k1 is non-zero at entry.
# -------------------------------------------------------------------
.global libn64_recv_message
.type libn64_recv_message, @function
.align 5
libn64_recv_message:
  lui $k0, 0x8000
  lw $k0, 0x420($k0)
  lw $k0, 0x8($k0)
  lw $k1, 0x194($k0)

# TODO: Block until a message comes in.
libn64_recv_message_block:
  beq $k1, $zero, libn64_recv_message_block
  nop

# Acknowledge the message, move it from the thread to the cache.
  lw $at, 0x0($k1)
  sw $at, 0x194($k0)

  lui $k0, 0x8000
  lw $at, 0x424($k0)
  sw $at, 0x0($k1)
  sw $k1, 0x424($k0)

# Read the message and its contents, return them to the caller.
  lw $at, 0x4($k1)
  ld $k1, 0x8($k1)
  jr $ra
  sd $k1, 0x0($a0)

.size libn64_recv_message,.-libn64_recv_message

# -------------------------------------------------------------------
#  Sends a message ($a1/message, $a2/data[0], $a3/data[1]) to a thread ($a0).
# -------------------------------------------------------------------
.global libn64_send_message
.type libn64_send_message, @function
.align 5
libn64_send_message:
  lui $k1, 0x8000
  lw $k0, 0x424($k1)
  beq $k0, $zero, libn64_send_message_expand_cache
  lw $at, 0x194($a0)

# Allocate a message from the cache and populate it.
  lw $k1, 0x0($k0)
  cache 0xD, 0x0($k0)
  sw $at, 0x0($k0)
  sw $a1, 0x4($k0)
  sw $a2, 0x8($k0)
  sw $a3, 0xC($k0)

  lui $at, 0x8000
  sw $k1, 0x424($at)
  jr $ra
  sw $k0, 0x194($a0)

# The message cache is dried up; expand the cache.
.align 5
libn64_send_message_expand_cache:
  addu $k1, $ra, $zero
  jal libn64_exception_handler_allocpage
  sll $k0, $k0, 0xC
  addu $ra, $k1, $zero

  lui $at, 0x8000
  or $k0, $k0, $at
  sw $k0, 0x424($at)
  addu $k1, $k0, $zero

libn64_send_message_alloc_loop:
  addiu $k0, $k0, 0x10
  cache 0xD, -0x10($k0)
  xor $at, $k0, $k1
  andi $at, $at, 0x1000
  beql $at, $zero, libn64_send_message_alloc_loop
  sw $k0, -0x10($k0)

# Now that the cache is populated, replay the send.
  j libn64_send_message
  sw $zero, -0x10($k0)

.size libn64_send_message,.-libn64_send_message

.set gp=default
.set fp=default
.set at
.set reorder

