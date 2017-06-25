#
# libn64/os/asm/message.s: libn64 message passing routines.
#
# n64chain: A (free) open-source N64 development toolchain.
# Copyright 2014-16 Tyler J. Stachecki <stachecki.tyler@gmail.com>
#
# This file is subject to the terms and conditions defined in
# 'LICENSE', which is part of this source code package.
#

.section .text.libn64.asm, "ax", @progbits

.set gp=64
.set fp=64
.set noat
.set noreorder

# -------------------------------------------------------------------
#  Sends a message ($a1/message, $a2/data) to a thread ($a0).
# -------------------------------------------------------------------
.global libn64_send_message
.type libn64_send_message, @function
.align 5
libn64_send_message:
  lui $k1, 0x8000
  lw $k0, 0x424($k1)
  beq $k0, $zero, libn64_send_message_expand_cache
  lwu $at, 0x194($a0)
  addu $k1, $at, $zero

# Allocate a message from the message cache and populate it.
# If there a message already at the tail, link this to it.
# If there isn't a message, then we're the head, so update it.
  bnel $at, $zero, libn64_send_message_after_prev_update
  sw $k0, 0x0($k1)
  sw $k0, 0x190($a0)

# Stuff the message, update the tail, remove from the cache.
libn64_send_message_after_prev_update:
  lw $k1, 0x0($k0)
  sd $at, 0x0($k0)
  sw $a1, 0x8($k0)
  sw $a2, 0xC($k0)

  lui $at, 0x8000
  sw $k0, 0x194($a0)
  jr $ra
  sw $k1, 0x424($at)

# The message cache is dried up; expand the cache.
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

