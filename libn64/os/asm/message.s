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
  lwu $at, 0x190($a0)
  addu $k1, $at, $zero

# Allocate a message from the message cache and populate it.
# If there a message already at the tail, link this to it.
# If there isn't a message, then we're the head, so update it.
  bnel $at, $zero, libn64_send_message_after_prev_update
  sw $k0, 0x0($k1)
  sw $k0, 0x194($a0)

# Stuff the message, update the tail, remove from the cache.
libn64_send_message_after_prev_update:
  lw $k1, 0x0($k0)
  sd $at, 0x0($k0)
  sw $a1, 0x8($k0)
  sw $a2, 0xC($k0)

  lui $at, 0x8000
  sw $k0, 0x190($a0)
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
  lui $k1, 0xA430
  lw $a2, 0x8($k1)
  lui $at, 0x8000

# Check for RCP interrupts, send messages as needed.
libn64_send_rcp_messages_check_ai:
  andi $k1, $a2, 0x4
  lui $k0, 0xA450
  bnel $k1, $zero, libn64_send_ai_messages
  sw $zero, 0xC($k0)

libn64_send_rcp_messages_check_dp:
  andi $k1, $a2, 0x20
  lui $k0, 0xA430
  addiu $a1, $zero, 0x800
  bnel $k1, $zero, libn64_send_dp_messages
  sw $a1, 0x0($k0)

libn64_send_rcp_messages_check_sp:
  andi $k1, $a2, 0x1
  lui $k0, 0xA404
  addiu $a1, $zero, 0x8
  bnel $k1, $zero, libn64_send_sp_messages
  sw $a1, 0x10($k0)

libn64_send_rcp_messages_check_pi:
  andi $k1, $a2, 0x10
  lui $k0, 0xA460
  addiu $a1, $zero, 0x2
  bnel $k1, $zero, libn64_send_pi_messages
  sw $a1, 0x10($k0)

libn64_send_rcp_messages_check_si:
  andi $k1, $a2, 0x2
  lui $k0, 0xA480
  bnel $k1, $zero, libn64_send_si_messages
  sw $zero, 0x18($k0)

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
  lw $at, 0x40C($k0)
  cache 0x11, 0x400($k0)

# Reschedule the active thread as needed.
  lw $k0, 0x420($k0)
  mfc0 $k1, $10
  lw $k0, 0x8($k0)

  andi $k1, $k1, 0xFF
  srl $k0, $k0, 0x9
  andi $ra, $k0, 0xFF

  beq $ra, $k1, libn64_send_rcp_messages_eret
  xor $k0, $k0, $ra
  or $k1, $k0, $k1
  sll $k1, $k1, 0x9

  la $k0, libn64_send_rcp_messages_restore
  j libn64_context_save
libn64_send_rcp_messages_eret:
  mfc0 $ra, $30
  eret

libn64_send_rcp_messages_restore:
  j libn64_context_restore
  lw $k1, 0x8($k0)

# Send a message to everyone watching AI interrupts.
.align 5
libn64_send_ai_messages:
  lw $a0, 0x430($at)

libn64_send_ai_messages_next:
  beq $a0, $zero, libn64_send_rcp_messages_end

# Send a message out on behalf of the AI interrupt.
  addiu $a1, $zero, -0x1
  jal libn64_send_message
  lw $a2, 0x80($a0)

# Test if the thread is blocked and needs to be queued.
  addu $k1, $a0, $zero
  andi $a1, $a2, 0x8000
  beq $a1, $zero, libn64_send_ai_messages_next
  lw $a0, 0x1A0($k1)

# Thread is blocked; unblock it and continue.
  xori $a1, $a2, 0x8000
  lw $k0, 0x420($at)
  sw $a1, 0x80($k1)
  jal libn64_exception_handler_queue_thread
  sw $a0, 0x4($k0)

  j libn64_send_ai_messages_next
  addu $a0, $ra, $zero

# Send a message to everyone watching DP interrupts.
.align 5
libn64_send_dp_messages:
  lw $a0, 0x434($at)

libn64_send_dp_messages_next:
  beq $a0, $zero, libn64_send_rcp_messages_end

# Send a message out on behalf of the DP interrupt.
  addiu $a1, $zero, -0x2
  jal libn64_send_message
  lw $a2, 0x80($a0)

# Test if the thread is blocked and needs to be queued.
  addu $k1, $a0, $zero
  andi $a1, $a2, 0x8000
  beq $a1, $zero, libn64_send_dp_messages_next
  lw $a0, 0x1A4($k1)

# Thread is blocked; unblock it and continue.
  xori $a1, $a2, 0x8000
  lw $k0, 0x420($at)
  sw $a1, 0x80($k1)
  jal libn64_exception_handler_queue_thread
  sw $a0, 0x4($k0)

  j libn64_send_dp_messages_next
  addu $a0, $ra, $zero

# Send a message to everyone watching PI interrupts.
.align 5
libn64_send_pi_messages:
  lw $a0, 0x438($at)

libn64_send_pi_messages_next:
  beq $a0, $zero, libn64_send_rcp_messages_end

# Send a message out on behalf of the PI interrupt.
  addiu $a1, $zero, -0x3
  jal libn64_send_message
  lw $a2, 0x80($a0)

# Test if the thread is blocked and needs to be queued.
  addu $k1, $a0, $zero
  andi $a1, $a2, 0x8000
  beq $a1, $zero, libn64_send_pi_messages_next
  lw $a0, 0x1A8($k1)

# Thread is blocked; unblock it and continue.
  xori $a1, $a2, 0x8000
  lw $k0, 0x420($at)
  sw $a1, 0x80($k1)
  jal libn64_exception_handler_queue_thread
  sw $a0, 0x4($k0)

  j libn64_send_pi_messages_next
  addu $a0, $ra, $zero

# Send a message to everyone watching SP interrupts.
.align 5
libn64_send_sp_messages:
  lw $a0, 0x43C($at)

libn64_send_sp_messages_next:
  beq $a0, $zero, libn64_send_rcp_messages_end

# Send a message out on behalf of the SP interrupt.
  addiu $a1, $zero, -0x4
  jal libn64_send_message
  lw $a2, 0x80($a0)

# Test if the thread is blocked and needs to be queued.
  addu $k1, $a0, $zero
  andi $a1, $a2, 0x8000
  beq $a1, $zero, libn64_send_sp_messages_next
  lw $a0, 0x1AC($k1)

# Thread is blocked; unblock it and continue.
  xori $a1, $a2, 0x8000
  lw $k0, 0x420($at)
  sw $a1, 0x80($k1)
  jal libn64_exception_handler_queue_thread
  sw $a0, 0x4($k0)

  j libn64_send_sp_messages_next
  addu $a0, $ra, $zero

# Send a message to everyone watching SI interrupts.
.align 5
libn64_send_si_messages:
  lw $a0, 0x440($at)

libn64_send_si_messages_next:
  beq $a0, $zero, libn64_send_rcp_messages_end

# Send a message out on behalf of the SI interrupt.
  addiu $a1, $zero, -0x5
  jal libn64_send_message
  lw $a2, 0x80($a0)

# Test if the thread is blocked and needs to be queued.
  addu $k1, $a0, $zero
  andi $a1, $a2, 0x8000
  beq $a1, $zero, libn64_send_si_messages_next
  lw $a0, 0x1B0($k1)

# Thread is blocked; unblock it and continue.
  xori $a1, $a2, 0x8000
  lw $k0, 0x420($at)
  sw $a1, 0x80($k1)
  jal libn64_exception_handler_queue_thread
  sw $a0, 0x4($k0)

  j libn64_send_si_messages_next
  addu $a0, $ra, $zero

# Send a message to everyone watching VI interrupts.
.align 5
libn64_send_vi_messages:
  lw $a0, 0x444($at)

libn64_send_vi_messages_next:
  beq $a0, $zero, libn64_send_rcp_messages_end

# Send a message out on behalf of the VI interrupt.
  addiu $a1, $zero, -0x6
  jal libn64_send_message
  lw $a2, 0x80($a0)

# Test if the thread is blocked and needs to be queued.
  addu $k1, $a0, $zero
  andi $a1, $a2, 0x8000
  beq $a1, $zero, libn64_send_vi_messages_next
  lw $a0, 0x1B4($k1)

# Thread is blocked; unblock it and continue.
  xori $a1, $a2, 0x8000
  lw $k0, 0x420($at)
  sw $a1, 0x80($k1)
  jal libn64_exception_handler_queue_thread
  sw $a0, 0x4($k0)

  j libn64_send_vi_messages_next
  addu $a0, $ra, $zero

.size libn64_send_rcp_messages,.-libn64_send_rcp_messages

.set gp=default
.set fp=default
.set at
.set reorder

