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

.section .text.libn64, "ax", @progbits

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

.type libn64_syscall_thread_create, @function
.align 5
libn64_syscall_thread_create:
  lui $k0, 0x8000
  lw $k0, 0x420($k0)
  mtc0 $k1, $14

# Grab the next available thread from the free thread list.
  lw $k1, THREAD_FREE_COUNT($k0)
  addiu $k1, $k1, -0x1
  sw $k1, THREAD_FREE_COUNT($k0)

  sll $k1, $k1, 0x2
  addu $k1, $k1, $k0
  lw $v0, THREAD_FREE_LIST($k1)

# Invalidate all of the new thread's L1 page table entries.
  addiu $at, $v0, 0x40

libn64_syscall_thread_create_invalidate_loop:
  cache 0xD, -0x10($at)
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
  cache 0xD, 0x198($v0)
  sw $a0, 0x010($v0)
  dsll32 $a0, $a2, 0x0
  sd $a0, 0x198($v0)
  subu $at, $at, $a2
  sd $zero, 0x190($v0)
  bltz $at, libn64_syscall_thread_create_start_new_thread
  sw $a1, 0x01C($v0)

# The running thread has a higher priority than the new thread.
# Queue the new thread so that it runs sometime in the future.
libn64_syscall_thread_queue_new_thread:
  sw $a2, 0x014($v0)
  sw $a3, 0x018($v0)

  la $at, libn64_syscall_thread_create_start_new
  sw $ra, 0x4($k0)
  jal libn64_exception_handler_queue_thread
  sw $at, 0x07C($v0)

# Restore destroyed variables and return.
  lw $a0, 0x010($v0)
  lw $a1, 0x01C($v0)
  lw $a2, 0x014($v0)
  lw $a3, 0x018($v0)
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
  addu $v0, $k1, $zero

# Set the new thread's status/coprocessor status, ASID, and stack/$gp.
  lw $a1, 0x010($v0)
  lw $a0, 0x01C($v0)

libn64_syscall_thread_create_start_new:
  mtc0 $a1, $14
  addiu $at, $zero, 0x401
  mtc0 $at, $12
  srl $at, $v0, 0x9
  andi $at, $at, 0xFF
  mtc0 $at, $10
  lui $sp, 0x8000
  la $gp, _gp

# By default, don't listen for any RCP interrupts.
  lui $at, 0xA430
  sw $zero, 0xC($at)

# If the thread returns, route it to libn64_thread_exit.
  la $ra, libn64_thread_exit
  eret

libn64_thread_exit:
  addiu $at, $zero, 0x1
  syscall

.size libn64_syscall_thread_create,.-libn64_syscall_thread_create

# -------------------------------------------------------------------
#  libn64::thread_exit
# -------------------------------------------------------------------
.type libn64_syscall_thread_exit, @function
.align 5

libn64_syscall_thread_exit:
  lui $k0, 0x8000
  lw $k0, 0x420($k0)
  sw $ra, 0x4($k0)
  jal libn64_exception_handler_dequeue_thread
  mtc0 $k1, $14
  j libn64_context_restore
  lw $k1, 0x8($k0)

.size libn64_syscall_thread_exit,.-libn64_syscall_thread_exit

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
#  libn64::send_message
#    $a0 = recipient
#    $a1 = message
#    $a2 = param
# -------------------------------------------------------------------
.type libn64_syscall_send_message, @function
.align 5
libn64_syscall_send_message:
  mtc0 $ra, $30
  jal libn64_send_message
  mtc0 $k1, $14

# Check to see if we unblocked a higher priority thread.
  lw $at, 0x420($at)
  mfc0 $ra, $30
  lw $at, 0x8($at)

  lw $k1, 0x198($a0)
  lw $k0, 0x198($at)
  subu $k1, $k1, $k0

  la $k0, libn64_unblock_hp_thread
  bgtz $k1, libn64_context_save
  addu $k1, $at, $zero
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
  lui $k1, 0x8000
  lw $k1, 0x420($k1)
  lw $k1, 0x8($k1)
  lw $at, 0x190($k1)

# If there are no messages available, block the thread.
libn64_recv_message_block:
  bnel $at, $zero, libn64_recv_message_deque
  lw $at, 0x0($at)

# No messages are available; set the thread's unblock return
# to this syscall, deque it, and schedule the next thread.
  addu $at, $k0, 0x4
  la $k0, libn64_block_thread
  j libn64_context_save
  mtc0 $at, $30

# Deque the message k0 the head/front of the message queue.
# If the message has a successor, make it the new queue head.
# If there is no successor, then there is no tail; update it.
libn64_recv_message_deque:
  sw $at, 0x190($k1)
  bnel $at, $zero, libn64_recv_message_after_next_update
  sw $zero, 0x4($at)
  sw $zero, 0x194($k1)

# Return the freed message to the message cache.
# Return the contents of the message to the caller.
libn64_recv_message_after_next_update:
  lui $k1, 0x8000
  lw $at, 0x424($k1)

  lw $v0, 0x8($k0)
  lw $v1, 0xC($k0)
  sw $at, 0x0($k0)
  sw $k0, 0x424($k1)
  eret

.size libn64_syscall_recv_message,.-libn64_syscall_recv_message

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
.long libn64_syscall_page_alloc
.long libn64_syscall_page_free
.long libn64_syscall_send_message
.long libn64_syscall_recv_message

.size libn64_syscall_table,.-libn64_syscall_table

.set at
.set reorder
.set gp=default

