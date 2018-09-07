#
# libn64/os/asm/syscall.s: libn64 syscalls.
#
# n64chain: A (free) open-source N64 development toolchain.
# Copyright 2014-16 Tyler J. Stachecki <stachecki.tyler@gmail.com>
#
# This file is subject to the terms and conditions defined in
# 'LICENSE', which is part of this source code package.
#

#include <libn64.h>
#include <syscall.h>

.section .text.libn64.asm, "ax", @progbits

.set noat
.set noreorder
.set gp=64

# -------------------------------------------------------------------
#  libn64::thread_create
#    $a0 = entrypoint
#    $a1 = argument
#    $a2 = priority
# -------------------------------------------------------------------
.set THREAD_FREE_COUNT, ((LIBN64_THREADS_MAX + 1) * 0x8)
.set THREAD_FREE_LIST, (THREAD_FREE_COUNT + 0x4)
.set DEFAULT_MI_INTR_MASK, 0xAAA

.type libn64_syscall_thread_create, @function
.align 5
libn64_syscall_thread_create:
  lui $k0, 0x8000
  lw $k0, 0x420($k0)
  mtc0 $k1, $14

# Grab the next available thread from the free thread list.
  lw $k1, THREAD_FREE_COUNT($k0)
  addiu $k1, $k1, -0x4
  sw $k1, THREAD_FREE_COUNT($k0)

  addu $k1, $k1, $k0
  lw $v0, THREAD_FREE_LIST($k1)

# Invalidate all of the new thread's L1 page table entries.
  addiu $at, $v0, 0x40

libn64_syscall_thread_create_invalidate_loop:
  cache 0xD, 0x1B0($at)
  addiu $at, $at, -0x10
  sd $zero, 0x1C0($at)
  bne $at, $v0, libn64_syscall_thread_create_invalidate_loop
  sd $zero, 0x1C8($at)

# Compare the running thread's priority against the new thread.
# If the running thread has a higher priority, keep it going.
# Set the thread's initial message queue pointers and priority.
# Flush out $a0/$a1 to the thread's $a1/$a0 registers as well.
  lw $at, 0x8($k0)
  cache 0xD, 0x010($v0)
  lw $at, 0x198($at)
  cache 0xD, 0x190($v0)
  sw $a0, 0x010($v0)
  dsll32 $a0, $a2, 0x0
  sd $a0, 0x198($v0)
  subu $at, $at, $a2
  sd $zero, 0x190($v0)
  addiu $k1, $zero, DEFAULT_MI_INTR_MASK
  sw $k1, 0x08C($v0)
  bltz $at, libn64_syscall_thread_create_start_new_thread
  sw $a1, 0x01C($v0)

# The running thread has a higher priority than the new thread.
# Queue the new thread so that it runs sometime in the future.
libn64_syscall_thread_queue_new_thread:
  sw $a2, 0x014($v0)
  addu $k1, $v0, $zero
  sw $a3, 0x018($v0)
  addiu $a3, $zero, 0x6
  sw $a3, 0x80($v0)
  la $at, libn64_syscall_thread_create_start_new
  sw $ra, 0x4($k0)
  jal libn64_exception_handler_queue_thread
  sw $at, 0x19C($v0)

# Put thread ref in $v0, restore destroyed variables and return.
  lw $k1, 0x8($k0)
  lw $a0, 0x010($v0)
  lw $a1, 0x01C($v0)
  lw $a2, 0x014($v0)
  lw $a3, 0x018($v0)
  sw $k1, 0x4($v0)
  eret

# The running thread has a lower/equal priority than the new thread.
# Save the current thread context, insert the new thread in its place.
libn64_syscall_thread_create_start_new_thread:
  addu $at, $k0, $zero
  la $k0, libn64_syscall_thread_create_start_new_thread_continue
  sw $v0, 0x4($at)
  j libn64_context_save
  lw $k1, 0x8($at)

libn64_syscall_thread_create_start_new_thread_continue:
  lw $k1, 0x4($k0)
  jal libn64_exception_handler_queue_thread
  lw $v0, 0x8($k0)

# Apply the RCP interrupt mask - we haven't done a context_load yet.
  lui $at, 0xA430
  addiu $k1, $zero, DEFAULT_MI_INTR_MASK
  sw $k1, 0xC($at)

# Set the new thread's status/coprocessor status, ASID, and stack/$gp.
libn64_syscall_thread_create_start_new:
  lw $k1, 0x8($k0)
  lw $a1, 0x010($k1)
  lw $a0, 0x01C($k1)
  mtc0 $a1, $14

  xori $at, $zero, 0x8403
  mtc0 $at, $12
  srl $at, $k1, 0x9
  andi $at, $at, 0xFF
  mtc0 $at, $10
  lui $sp, 0x8000
  la $gp, _gp

# If the thread returns, route it to libn64_thread_exit.
  la $ra, libn64_thread_exit
  eret

.size libn64_syscall_thread_create,.-libn64_syscall_thread_create

# -------------------------------------------------------------------
#  libn64::thread_exit
# -------------------------------------------------------------------
.type libn64_syscall_thread_exit, @function
.align 5

libn64_syscall_thread_exit:
  lui $v0, 0x8000
  lw $k0, 0x420($v0)
  mtc0 $zero, $2

# Return the thread to the thread stack, bump the free count.
  lw $at, THREAD_FREE_COUNT($k0)
  lw $k1, 0x8($k0)
  addiu $at, $at, 0x4
  sw $at, THREAD_FREE_COUNT($k0)
  addu $at, $at, $k0
  sw $k1, (THREAD_FREE_LIST-0x4)($at)

# Free any messages that were not yet received by the thread.
  mtc0 $zero, $3
  ld $at, 0x190($k1)
  mtc0 $zero, $10
  beq $at, $zero, libn64_syscall_thread_exit_free_stack_l2_entries
  lw $ra, 0x424($v0)

  sw $at, 0x424($v0)
  dsra32 $at, $at, 0x0
  sw $ra, 0x0($at)

# Free any tracking information and pages alloc'd for the stack.
libn64_syscall_thread_exit_free_stack_l2_entries:
  addu $a1, $k1, $zero
  addiu $a2, $k1, 0x40
  mtc0 $zero, $12

libn64_syscall_thread_exit_free_stack_l2_entries_loop:
  beq $a2, $k1, libn64_syscall_thread_exit_dtlb_flush
  lhu $v1, (0x1C0 - 0x2)($a2)

  addiu $a2, $a2, -0x2
  beq $v1, $zero, libn64_syscall_thread_exit_free_stack_l2_entries_loop
  sll $v1, $v1, 0x7
  or $v1, $v1, $v0
  addiu $ra, $v1, 0x80

libn64_syscall_thread_exit_free_stack_pages_loop:
  beq $ra, $v1, libn64_syscall_thread_exit_free_stack_l2_entry_finish
  lhu $a0, -0x2($ra)

  addiu $ra, $ra, -0x2
  beq $a0, $zero, libn64_syscall_thread_exit_free_stack_pages_loop
  addiu $at, $zero, LIBN64_SYSCALL_PAGE_FREE
  sll $a0, $a0, 0xC
  syscall

  bne $ra, $v1, libn64_syscall_thread_exit_free_stack_pages_loop
  addu $k1, $a1, $zero

libn64_syscall_thread_exit_free_stack_l2_entry_finish:
  lw $k0, 0x42C($v0)
  sw $v1, 0x42C($v0)
  addu $k1, $a1, $zero
  cache 0xD, 0x0($v1)
  bne $a2, $k1, libn64_syscall_thread_exit_free_stack_l2_entries_loop
  sw $k0, 0x0($v1)

# Blow out the DTLB (to effectively invalidate the thread's entries).
# TODO: Only blow out entries corresponding to *this* thread...
libn64_syscall_thread_exit_dtlb_flush:
  mtc0 $zero, $10
  addiu $k1, $zero, 0x1F

libn64_syscall_thread_exit_dtlb_flush_loop:
  mtc0 $k1, $0
  addiu $k1, $k1, -0x1
  bgez $k1, libn64_syscall_thread_exit_dtlb_flush_loop
  tlbwi

# Unregister the thread from any interrupts chains that it's on.
  addiu $a0, $zero, 0x430

libn64_syscall_thread_exit_unreg_intr:
  addiu $at, $zero, LIBN64_SYSCALL_THREAD_UNREG_INTR
  syscall

  addiu $at, $a0, -0x444
  bne $at, $zero, libn64_syscall_thread_exit_unreg_intr
  addiu $a0, $a0, 0x4

# Mark the thread unblocked (so we don't requeue) and unqueue it.
  lw $k0, 0x420($v0)
  addu $k1, $a1, $zero
  jal libn64_exception_handler_dequeue_thread
  sw $zero, 0x80($k1)
  j libn64_context_restore
  lw $k1, 0x8($k0)

.size libn64_syscall_thread_exit,.-libn64_syscall_thread_exit

# -------------------------------------------------------------------
#  libn64::thread_reg_intr
#    $a0 = interrupt
#    $a1 = thread
# -------------------------------------------------------------------
.type libn64_syscall_thread_reg_intr, @function
.align 5
libn64_syscall_thread_reg_intr:
  lui $at, 0x8000
  mtc0 $k1, $14
  addu $k0, $at, $a0
  lw $k1, 0x0($k0)
  sw $a1, 0x0($k0)
  addu $k0, $a1, $a0

libn64_syscall_thread_unreg_intr_found:
  sw $k1, (0x1A0-0x430)($k0)
  eret

.size libn64_syscall_thread_reg_intr, .-libn64_syscall_thread_reg_intr

# -------------------------------------------------------------------
#  libn64::thread_self
# -------------------------------------------------------------------
.type libn64_syscall_thread_self, @function
.align 5

libn64_syscall_thread_self:
  lui $k0, 0x8000
  mtc0 $k1, $14
  lw $k0, 0x420($k0)
  lw $v0, 0x8($k0)
  eret

libn64_thread_exit:
  addiu $at, $zero, 0x1
  syscall

.size libn64_syscall_thread_self, .-libn64_syscall_thread_self

# -------------------------------------------------------------------
#  libn64::thread_unreg_intr
#    $a0 = interrupt
#    $a1 = thread
# -------------------------------------------------------------------
.type libn64_syscall_thread_unreg_intr, @function
libn64_syscall_thread_unreg_intr:
  lui $k0, 0x8000
  addu $at, $k0, $a0
  lw $k0, 0x0($at)
  mtc0 $k1, $14
  beq $k0, $zero, libn64_syscall_thread_unreg_intr_none
  addu $k1, $k0, $a0
  beq $k0, $a1, libn64_syscall_thread_unreg_intr_head
  lw $k1, (0x1A0-0x430)($k1)

libn64_syscall_thread_unreg_intr_loop:
  beq $k1, $zero, libn64_syscall_thread_unreg_intr_none
  addu $k0, $k0, $a0
  addu $at, $k1, $a0
  beq $k1, $a1, libn64_syscall_thread_unreg_intr_found
  lw $k1, (0x1A0-0x430)($at)
  j libn64_syscall_thread_unreg_intr_loop
  lw $k0, (0x1A0-0x430)($k0)

libn64_syscall_thread_unreg_intr_head:
  sw $k1, 0x0($at)

libn64_syscall_thread_unreg_intr_none:
  eret

.size libn64_syscall_thread_unreg_intr, .-libn64_syscall_thread_unreg_intr

# -------------------------------------------------------------------
#  libn64::page_alloc
# -------------------------------------------------------------------
.type libn64_syscall_page_alloc, @function
.align 5
libn64_syscall_page_alloc:
  mtc0 $k1, $14
  addu $k1, $ra, $zero
  jal libn64_exception_handler_allocpage
  sll $at, $k0, 0xC
  addu $ra, $k1, $zero
  lui $k0, 0x8000
  or $at, $k0, $at
  eret

.size libn64_syscall_page_alloc,.-libn64_syscall_page_alloc

# -------------------------------------------------------------------
#  libn64::page_free
#    $a0 = page address
# -------------------------------------------------------------------
.type libn64_syscall_page_free, @function
.align 5
libn64_syscall_page_free:
  lui $at, 0x8000
  mtc0 $k1, $14

  # Increment the page count for the bank.
  srl $k1, $a0, 0x13
  andi $k1, $k1, 0xE
  addu $k1, $at, $k1
  lhu $k0, 0x410($k1)
  addiu $k0, $k0, 0x1
  sh $k0, 0x410($k1)

  # Return the page to the bank's list.
  sll $k0, $k0, 0x1
  sll $k1, $k1, 0x8
  addu $k1, $k1, $k0

  lw $k0, 0x428($at)
  srl $at, $a0, 0xC
  addu $k0, $k0, $k1
  sh $at, -0x2($k0)
  eret

.size libn64_syscall_page_free,.-libn64_syscall_page_free

# -------------------------------------------------------------------
#  libn64::time
#
#  TODO: Preserve HI and LO registers.
# -------------------------------------------------------------------
.type libn64_syscall_time, @function
.align 5
libn64_syscall_time:
  lui $k0, 0x8000
  ld $k0, 0x448($k0)
  mtc0 $k1, $14
  dmfc0 $k1, $9
  or $k0, $k0, $k1

  # 62,500,000 * 1.5 / 2 = 46,875,000 ticks per second.
  lui $k1, 0x02CB
  ori $k1, 0x4178
  ddivu $zero, $k0, $k1

  # tv_sec is the result of the div. Compute tv_usec next.
  mflo $v0 # div result -> tv_sec
  mfhi $v1 # mod result * 1e6
  lui $k0, 0x000F
  ori $k0, 0x4240
  multu $v1, $k0 

  mfhi $v1
  mflo $k0
  dsll32 $k0, $k0, 0
  dsll32 $v1, $v1, 0
  dsrl32 $k0, $k0, 0
  or $v1, $v1, $k0
  ddivu $zero, $v1, $k1
  mflo $v1
  nop
  eret

.size libn64_syscall_time,.-libn64_syscall_time

# -------------------------------------------------------------------
#  libn64::sendt_message
#    $a0 = recipient
#    $a1 = message
#    $a2 = param
# -------------------------------------------------------------------
.type libn64_syscall_sendt_message, @function
.align 5
libn64_syscall_sendt_message:
  mtc0 $ra, $30
  jal libn64_send_message
  mtc0 $k1, $14

# Check to see if we unblocked a higher priority thread.
libn64_sendt_message_unblock:
  lui $at, 0x8000
  lw $at, 0x420($at)
  mfc0 $ra, $30
  j libn64_maybe_unblock_thread
  lw $k1, 0x8($at)

.size libn64_syscall_sendt_message,.-libn64_syscall_sendt_message

# -------------------------------------------------------------------
#  libn64::recvt_message
# -------------------------------------------------------------------
.global libn64_syscall_recvt_message
.type libn64_syscall_recvt_message, @function
.align 5
libn64_syscall_recvt_message:
  mtc0 $k1, $14
  addiu $v0, $k0, 0x8

libn64_recvt_replay:
  lui $k1, 0x8000
  lw $k1, 0x420($k1)
  lw $k1, 0x8($k1)
  lw $at, 0x194($k1)

# If there is > 1 message available, extract it from the queue.
libn64_recvt_message_block:
  bnel $at, $zero, libn64_recvt_message_deque
  lw $k0, 0x0($at)

# No messages are available; set the thread's unblock return
# to this syscall, deque it, and schedule the next thread.
  la $k0, libn64_block_thread
  j libn64_context_save
  mtc0 $v0, $30

# Deque the message at the head/front of the message queue.
# If the message has a successor, make it the new queue head.
# If there is no successor, then there is no tail; update it.
libn64_recvt_message_deque:
  sw $k0, 0x194($k1)
  bnel $k0, $zero, libn64_recvt_message_after_next_update
  sw $zero, 0x4($k0)
  sw $zero, 0x190($k1)

# Return the freed message to the message cache.
# Return the contents of the message to the caller.
libn64_recvt_message_after_next_update:
  lui $k1, 0x8000
  lw $k0, 0x424($k1)

  lw $v0, 0x8($at)
  sw $k0, 0x0($at)
  sw $at, 0x424($k1)
  lw $at, 0xC($at)

  eret

.size libn64_syscall_recvt_message,.-libn64_syscall_recvt_message

# -------------------------------------------------------------------
#  libn64::send_message
# -------------------------------------------------------------------
.global libn64_syscall_send_message
.type libn64_syscall_send_message, @function
.align 5
libn64_syscall_send_message:
  mtc0 $ra, $30
  addiu $a0, $a0, -0x190
  jal libn64_send_message
  mtc0 $k1, $14

# Check to see if we unblocked a higher priority thread.
  lw $a0, 0x198($a0)
  bne $a0, $zero, libn64_sendt_message_unblock
  nop
  eret

.size libn64_syscall_send_message,.-libn64_syscall_send_message

# -------------------------------------------------------------------
#  libn64::recv_message
# -------------------------------------------------------------------
.global libn64_syscall_recv_message
.type libn64_syscall_recv_message, @function
.align 5
libn64_syscall_recv_message:
  mtc0 $k1, $14
  addiu $k0, $k0, 0x8

libn64_recv_replay:
  lw $at, 0x4($a0)

# If there is > 1 message available, extract it from the queue.
libn64_recv_message_block:
  beq $at, $zero, libn64_recv_maybe_block_thread
  addiu $k1, $a0, -0x190
  j libn64_recvt_message_deque
  lw $k0, 0x0($at)

# No messages are available; set the thread's unblock return
# to this syscall, deque it, and schedule the next thread.
libn64_recv_maybe_block_thread:
  bgezl $a1, libn64_recv_block_thread
  lw $k1, 0x8($a0)
  lui $v0, 0x8000
  eret

libn64_recv_block_thread:
  mtc0 $k0, $30
  la $k0, libn64_block_thread
  j libn64_context_save
  nop


.size libn64_syscall_recv_message,.-libn64_syscall_recv_message

# -------------------------------------------------------------------
#  libn64::mq_alloc
# -------------------------------------------------------------------
.global libn64_syscall_mq_alloc
.type libn64_syscall_mq_alloc, @function
.align 5
libn64_syscall_mq_alloc:
  mtc0 $k1, $14

libn64_mq_alloc_replay:
  lui $at, 0x8000
  lw $v0, 0x424($at)
  beql $v0, $zero, libn64_sendt_message_expand_cache
  mtc0 $ra, $30

  lw $k1, 0x0($v0)
  sw $k1, 0x424($at)
  eret

# The message cache is dried up; expand the cache.
libn64_sendt_message_expand_cache:
  jal libn64_exception_handler_allocpage
  lui $at, 0x8000
  sll $k0, $k0, 0xC
  or $k0, $k0, $at
  sw $k0, 0x424($at)
  addu $k1, $k0, $zero

libn64_sendt_message_alloc_loop:
  addiu $k0, $k0, 0x10
  cache 0xD, -0x10($k0)
  xor $at, $k0, $k1
  andi $at, $at, 0x1000
  beql $at, $zero, libn64_sendt_message_alloc_loop
  sw $k0, -0x10($k0)

# Now that the cache is populated, replay the alloc.
  sw $zero, -0x10($k0)
  j libn64_mq_alloc_replay
  mfc0 $ra, $30

.size libn64_syscall_mq_alloc,.-libn64_syscall_mq_alloc

# -------------------------------------------------------------------
#  libn64::mq_free
# -------------------------------------------------------------------
.global libn64_syscall_mq_free
.type libn64_syscall_mq_free, @function
.align 5
libn64_syscall_mq_free:
  lui $at, 0x8000
  lw $k0, 0x424($at)
  cache 0xD, 0x0($a0)
  sw $k0, 0x0($a0)
  mtc0 $k1, $14
  sw $a0, 0x424($at)
  eret

.size libn64_syscall_mq_free,.-libn64_syscall_mq_free

# -------------------------------------------------------------------
#  System call table.
# -------------------------------------------------------------------
.section  .rodata

.global libn64_syscall_table
.type libn64_syscall_table, @object
.align 4
libn64_syscall_table:
.long libn64_syscall_thread_create
.long libn64_syscall_thread_exit
.long libn64_syscall_thread_reg_intr
.long libn64_syscall_thread_self
.long libn64_syscall_thread_unreg_intr
.long libn64_syscall_page_alloc
.long libn64_syscall_page_free
.long libn64_syscall_time
.long libn64_syscall_sendt_message
.long libn64_syscall_recvt_message
.long libn64_syscall_send_message
.long libn64_syscall_recv_message
.long libn64_syscall_mq_alloc
.long libn64_syscall_mq_free

.size libn64_syscall_table,.-libn64_syscall_table

.set at
.set reorder
.set gp=default

