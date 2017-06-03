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
.global libn64_syscall_thread_create
.type libn64_syscall_thread_create, @function
.align 5
libn64_syscall_thread_create:
  la $k0, libn64_syscall_thread_create_aftersave
  mtc0 $k1, $14
  lui $k1, 0x8000
  lw $k1, 0x420($k1)
  j libn64_context_save
  lw $k1, 0x8($k1)

# Grab the next available thread, set it's priority.
libn64_syscall_thread_create_aftersave:
  lw $k1, ((LIBN64_THREADS_MAX + 1) * 0x8 + 0x4)($k0)
  addiu $k1, $k1, -0x1
  sw $k1, ((LIBN64_THREADS_MAX + 1) * 0x8 + 0x4)($k0)

  sll $k1, $k1, 0x2
  addu $k1, $k1, $k0
  lw $k1, ((LIBN64_THREADS_MAX + 1) * 0x8 + 0x8)($k1)

# Clear the L1 page table entries.
  addiu $at, $k1, 0x40

libn64_syscall_thread_create_clearloop:
  cache 0xD, -0x10($at)
  addiu $at, $at, -0x10
  sd $zero, 0x1C0($at)
  bne $at, $k1, libn64_syscall_thread_create_clearloop
  sd $zero, 0x1C8($at)

# Set the thread's priority, stack, $gp, and coprocessor status.
  sw $a2, 0x190($k1)
  #cache 0xD, 0x060($k1)
  lui $at, 0x8000
  sw $at, 0x068($k1)
  la $at, _gp
  sw $at, 0x064($k1)
  addiu $at, $zero, 0x400
  cache 0xD, 0x080($k1)
  sw $at, 0x080($k1)
  srl $at, $k1, 0x9
  andi $at, $at, 0xFF
  sw $at, 0x084($k1)
  sw $zero, 0x08C($k1)

# Store the thread's argument to $a0 so that it can be accessed upon start.
# Store the thread's entrypoint to the exception return address register.
# Set the return address from thread entrypoint to libn64_thread_exit.
  cache 0xD, 0x000($k1)
  sw $a1, 0x00C($k1)
  cache 0xD, 0x070($k1)
  sw $a0, 0x07C($k1)
  la $at, libn64_thread_exit

# Insert the thread into the ready queue and load up the next thread.
  sw $ra, 0x4($k0)
  jal libn64_exception_handler_queue_thread
  sw $at, 0x070($k1)

  j libn64_context_restore
  lw $k1, 0x8($k0)

.size libn64_syscall_thread_create,.-libn64_syscall_thread_create

# -------------------------------------------------------------------
#  libn64::thread_exit
# -------------------------------------------------------------------
.global libn64_syscall_thread_exit
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
.global libn64_syscall_page_alloc
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

.set at
.set reorder
.set gp=default

.section  .rodata

# -------------------------------------------------------------------
#  System call table.
# -------------------------------------------------------------------
.global libn64_syscall_table
.type libn64_syscall_table, @object
.align 4
libn64_syscall_table:
.long libn64_syscall_thread_create
.long libn64_syscall_thread_exit
.long libn64_syscall_page_alloc

.size libn64_syscall_table,.-libn64_syscall_table

