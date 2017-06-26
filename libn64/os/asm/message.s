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

# -------------------------------------------------------------------
#  Blasts messages out on behalf of the RCP (interrupts).
# -------------------------------------------------------------------
.global libn64_send_rcp_messages
.type libn64_send_rcp_messages, @function
.align 5
libn64_send_rcp_messages:
  sw $a3, 0x448($k0) # TODO save a3
  lw $k1, 0x420($k0)
  lw $k1, 0x8($k1)
  sw $k1, 0x44C($k0) # TODO save current thread

libn64_send_rcp_messages_again:
  lui $k1, 0xA430
  lw $a2, 0x8($k1)
  lui $at, 0x8000

# Check for RCP interrupts, send messages as needed.
libn64_send_rcp_messages_check_vi:
  andi $k1, $a2, 0x8
  lui $k0, 0xA440
  bnel $k1, $zero, libn64_send_vi_messages
  sw $zero, 0x10($k0)

libn64_send_rcp_messages_end:
  lui $k0, 0x8000
  lw $a0, 0x400($k0)
  lw $a1, 0x404($k0)
  lw $a2, 0x408($k0)
  lw $a3, 0x448($k0) # TODO load a3
  lw $at, 0x40C($k0)
  mfc0 $ra, $30

# Reschedule the active thread as needed.
  lw $k1, 0x44C($k0)
  la $k0, libn64_send_rcp_messages_eret
  j libn64_context_save
  nop

libn64_send_rcp_messages_eret:
  j libn64_context_restore
  lw $k1, 0x8($k0)

# Messages have been sent out: unblock threads as needed.
.align 5
libn64_unblock_threads:
  lw $k0, 0x420($at)

libn64_unblock_threads_next:
  beq $ra, $zero, libn64_send_rcp_messages_again
  sw $ra, 0x4($k0)

  lw $at, 0x80($ra)
  andi $at, $at, 0x4
  beql $at, $zero, libn64_unblock_threads_next
  lw $ra, 0x1B4($ra)

  jal libn64_exception_handler_queue_thread
  lw $k1, 0x4($k0)
  j libn64_unblock_threads_next
  lw $ra, 0x1B4($ra)

# Send a message to everyone watching VI interrupts.
.align 5
libn64_send_vi_messages:
  lw $a0, 0x444($at)
  xor $a2, $a2, $a2

libn64_send_vi_messages_next:
  beql $a0, $zero, libn64_unblock_threads
  lw $ra, 0x444($at)

  jal libn64_send_message
  ori $a1, $at, 0x6
  j libn64_send_vi_messages_next
  lw $a0, 0x1B4($a0)

.size libn64_send_rcp_messages,.-libn64_send_rcp_messages

.set gp=default
.set fp=default
.set at
.set reorder

