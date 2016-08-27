#
# libn64/os/asm/exception.s: libn64 exception handler.
#
# n64chain: A (free) open-source N64 development toolchain.
# Copyright 2014-16 Tyler J. Stachecki <stachecki.tyler@gmail.com>
#
# This file is subject to the terms and conditions defined in
# 'LICENSE', which is part of this source code package.
#

#include <os/syscall.h>

.section .text.libn64, "ax", @progbits

.set gp=64
.set fp=64
.set noat
.set noreorder

.global libn64_context_restore
.type libn64_context_restore, @function

# -------------------------------------------------------------------
#  Loads the context from the thread pointed to by $k1. After the context is
#  reloaded, we return from the exception handler (i.e., the thread resumes
#  execution).
#
#  The address must be 8-byte aligned and in a cacheable region.
#
#  The assembly looks like a nightmare for a reason: instructions have been
#  ordered carefully so as to reduce the number of stalls (e.g., load-to-
#  use instances have been cracked where possible to prevent a pipeline stall).
# -------------------------------------------------------------------
.align 5
libn64_context_restore:
  lw $v0, 0x074($k1)
  lw $a0, 0x00C($k1)
  lw $a1, 0x010($k1)
  mtlo $v0
  lw $v0, 0x078($k1)
  lw $a2, 0x014($k1)
  lw $a3, 0x018($k1)
  mthi $v0
  lw $v0, 0x07C($k1)
  lw $t0, 0x01C($k1)
  lw $t1, 0x020($k1)
  mtc0 $v0, $14
  lw $v0, 0x084($k1)
  lw $t2, 0x024($k1)
  lw $t3, 0x028($k1)
  mtc0 $v0, $10
  lui $v1, 0xA430
  lw $at, 0x08C($k1)
  lw $v0, 0x080($k1)
  sw $at, 0xC($v1)
  mtc0 $v0, $12
  lw $t4, 0x02C($k1)
  lw $t5, 0x030($k1)
  lw $t6, 0x034($k1)
  lw $t7, 0x038($k1)
  lw $s0, 0x03C($k1)
  lw $s1, 0x040($k1)
  lw $s2, 0x044($k1)
  lw $s3, 0x048($k1)
  lw $s4, 0x04C($k1)
  lw $s5, 0x050($k1)
  lw $s6, 0x054($k1)
  lw $s7, 0x058($k1)
  lw $t8, 0x05C($k1)
  lw $t9, 0x060($k1)
  lw $gp, 0x064($k1)
  lw $sp, 0x068($k1)
  lw $fp, 0x06C($k1)
  srl $v0, $v0, 26
  andi $v1, $v0, 0x8
  bne $v1, $zero, libn64_context_restore_fpu
  lw $ra, 0x070($k1)

libn64_context_restore_done:
  lw $at, 0x000($k1)
  lw $v0, 0x004($k1)
  lw $v1, 0x008($k1)
  eret

libn64_context_restore_fpu:
  ldc1 $0, 0x090($k1)
  ldc1 $2, 0x098($k1)
  ldc1 $4, 0x0A0($k1)
  ldc1 $6, 0x0A8($k1)
  ldc1 $8, 0x0B0($k1)
  ldc1 $10, 0x0B8($k1)
  ldc1 $12, 0x0C0($k1)
  ldc1 $14, 0x0C8($k1)
  ldc1 $16, 0x0D0($k1)
  ldc1 $18, 0x0D8($k1)
  ldc1 $20, 0x0E0($k1)
  ldc1 $22, 0x0E8($k1)
  ldc1 $24, 0x0F0($k1)
  ldc1 $26, 0x0F8($k1)
  ldc1 $28, 0x100($k1)
  ldc1 $30, 0x108($k1)
  and $v0, $v0, 0x1
  lw $v1, 0x088($k1)
  beq $v0, $zero, libn64_context_restore_done
  ctc1 $v1, $31

  ldc1 $1, 0x110($k1)
  ldc1 $3, 0x118($k1)
  ldc1 $5, 0x120($k1)
  ldc1 $7, 0x128($k1)
  ldc1 $9, 0x130($k1)
  ldc1 $11, 0x138($k1)
  ldc1 $13, 0x140($k1)
  ldc1 $15, 0x148($k1)
  ldc1 $17, 0x150($k1)
  ldc1 $19, 0x158($k1)
  ldc1 $21, 0x160($k1)
  ldc1 $23, 0x168($k1)
  ldc1 $25, 0x170($k1)
  ldc1 $27, 0x178($k1)
  ldc1 $29, 0x180($k1)
  j libn64_context_restore_done
  ldc1 $31, 0x188($k1)

.size libn64_context_restore,.-libn64_context_restore

.global libn64_context_save
.type libn64_context_save, @function

# -------------------------------------------------------------------
#  Saves the context to the address pointed to by $basereg and then branches
#  back to the $k0 ($ra is not usable because it's part of the thread context).
#  Before returning, the address of the thread table is loaded to $k0.
#
#  The address must be 8-byte aligned and in a cacheable region.
#
#  The assembly looks like a nightmare for a reason: instructions have been
#  ordered carefully so as to reduce the number of stalls (e.g., back-to-
#  back cache stores are avoided where possible to prevent a pipeline stall).
#
#  The CACHE operations within are there to create dirty exclusive lines:
#  If the current line referenced by the address is dirty (and thus has valid
#  data), it is written back to memory as would on any normal conflict hit.
#  However, the line is not initially filled from memory with the conflicting
#  address- it is simply initialized to the dirty state (since we're going to
#  fill it anyways). This maximizes the use of the write buffer and prevents a
#  lot of wasteful cache block reads during context switches.
# -------------------------------------------------------------------
.align 5
libn64_context_save:
  cache 0xD, 0x000($k1)
  sw $at, 0x000($k1)
  addiu $at, $k1, 0x070

libn64_context_save_loop:
  cache 0xD, 0x010($at)
  bne $at, $k1, libn64_context_save_loop
  addiu $at, $at, -0x10

  sw $v0, 0x004($k1)
  sw $v1, 0x008($k1)
  sw $a0, 0x00C($k1)
  sw $a1, 0x010($k1)
  sw $a2, 0x014($k1)
  sw $a3, 0x018($k1)
  sw $t0, 0x01C($k1)
  sw $t1, 0x020($k1)
  mflo $v0
  sw $t2, 0x024($k1)
  sw $t3, 0x028($k1)
  sw $v0, 0x074($k1)
  mfhi $v0
  sw $t4, 0x02C($k1)
  sw $t5, 0x030($k1)
  sw $v0, 0x078($k1)
  mfc0 $at, $12
  sw $t6, 0x034($k1)
  sw $t7, 0x038($k1)
  sw $at, 0x080($k1)
  srl $at, $at, 26
  sw $s0, 0x03C($k1)
  lui $v1, 0xA430
  sw $s1, 0x040($k1)
  lw $v1, 0x00C($v1)
  sw $s2, 0x044($k1)
  sw $s3, 0x048($k1)
  sw $s4, 0x04C($k1)
  sw $s5, 0x050($k1)
  sw $s6, 0x054($k1)
  sw $s7, 0x058($k1)
  sw $t8, 0x05C($k1)
  sw $t9, 0x060($k1)
  sw $gp, 0x064($k1)
  sw $sp, 0x068($k1)
  mfc0 $v0, $14
  sw $fp, 0x06C($k1)
  sw $v0, 0x07C($k1)
  mfc0 $v0, $10
  sw $ra, 0x070($k1)
  sw $v0, 0x084($k1)
  and $v0, $at, 0x8
  bne $v0, $zero, libn64_context_save_fpu
  sw $v1, 0x08C($k1)

libn64_context_save_done:
  lw $at, (libn64_thread_table)
  jr $k0
  addu $k0, $at, $zero

libn64_context_save_fpu:
  addiu $v1, $k1, 0x70

libn64_context_save_fpu_loop:
  cache 0xD, 0x090($v1)
  bne $v1, $k1, libn64_context_save_fpu_loop
  addiu $v1, $v1, -0x10

  sdc1 $0, 0x090($k1)
  sdc1 $2, 0x098($k1)
  sdc1 $4, 0x0A0($k1)
  sdc1 $6, 0x0A8($k1)
  sdc1 $8, 0x0B0($k1)
  sdc1 $10, 0x0B8($k1)
  sdc1 $12, 0x0C0($k1)
  sdc1 $14, 0x0C8($k1)
  cfc1 $v0, $31
  sdc1 $16, 0x0D0($k1)
  sdc1 $18, 0x0D8($k1)
  sw $v0, 0x088($v1)
  sdc1 $20, 0x0E0($k1)
  sdc1 $22, 0x0E8($k1)
  sdc1 $24, 0x0F0($k1)
  and $v0, $at, 0x1
  sdc1 $26, 0x0F8($k1)
  sdc1 $28, 0x100($k1)
  beq $v0, $zero, libn64_context_save_done
  sdc1 $30, 0x108($k1)
  addiu $v1, $k1, 0x70

libn64_context_save_fpufr_loop:
  cache 0xD, 0x110($v1)
  bne $v1, $k1, libn64_context_save_fpufr_loop
  addiu $v1, $v1, -0x10

  sdc1 $1, 0x110($k1)
  sdc1 $3, 0x118($k1)
  sdc1 $5, 0x120($k1)
  sdc1 $7, 0x128($k1)
  sdc1 $9, 0x130($k1)
  sdc1 $11, 0x138($k1)
  sdc1 $13, 0x140($k1)
  sdc1 $15, 0x148($k1)
  sdc1 $17, 0x150($k1)
  sdc1 $19, 0x158($k1)
  sdc1 $21, 0x160($k1)
  sdc1 $23, 0x168($k1)
  sdc1 $25, 0x170($k1)
  sdc1 $27, 0x178($k1)
  sdc1 $29, 0x180($k1)
  j libn64_context_save_done
  sdc1 $31, 0x188($k1)

.size libn64_context_save,.-libn64_context_save

# -------------------------------------------------------------------
#  This part of the exception handler gets loaded directly at 0x80000180
#  by the loader. Certain large portions of the context handler (i.e., the
#  context switching) live with the rest of the libn64 (@ 0x80000400+) due
#  to the fact that we only have about 0x280 bytes for the entirety of this
#  routine without doing a lot more relocation work.
# -------------------------------------------------------------------
.section .exception, "ax", @progbits

.global libn64_exception_handler
.type libn64_exception_handler, @function

# -------------------------------------------------------------------
#  We have eight instructions per cache line. Before we fall out of this line,
#  branch to one of the four specific exception handlers (either interrupt, TLB,
#  syscall, or a catch-all for all other cases).
# -------------------------------------------------------------------
.align 5
libn64_exception_handler:
  mfc0 $k0, $13
  andi $k1, $k0, 0x7C
  beq $k1, $zero, libn64_exception_handler_interrupt
  addiu $k1, $k1, -0x10
  bltz $k1, libn64_exception_handler_tlb
  addiu $k1, $k1, -0x10
  beql $k1, $zero, libn64_exception_handler_syscall
  sll $k0, $at, 0x2

# -------------------------------------------------------------------
#  Breakpoint/CpU exception handler. Also catches other (unexpected) exceptions.
# -------------------------------------------------------------------
.align 5
libn64_exception_handler_infrequent:
  addiu $k1, $k1, -0x4
  beq $k1, $zero, libn64_exception_handler_break
  addiu $k1, $k1, -0x8
  bne $k1, $zero, libn64_panic

# We have a CpU exception. If the coprocessor that caused this exception was
# the FPU, then branch to the dedicated handler. Otherwise, the exception we
# are catching is unhandled and we should panic.
  srl $k1, $k0, 0x1C
  andi $k1, $k1, 0x3
  addiu $k1, $k1, -0x1
  beql $k1, $zero, libn64_exception_handler_cpu_fpu
  mfc0 $k0, $12

# Grant the thread FPU access and return from the exception. We grant access
# by updating the status register (to enable CP1). The status register is part
# of the thread context and will be restored/saved at each context switch.
libn64_exception_handler_cpu_fpu:
  lui $k1, 0x2000
  or $k0, $k0, $k1
  mtc0 $k0, $12

# Set the FPU control register to a default value (trap on divide by zero,
# clear out all the cause and flag bits, set default rounding mode, etc.)
  addiu $k0, $zero, 0x400
  ctc0 $k0, $31
  eret

# -------------------------------------------------------------------
#  Dumps the thread context to RDRAM and invokes libn64_panic_from_isr.
# -------------------------------------------------------------------
.align 5
libn64_panic:
  la $k0, libn64_panic_from_isr_return
  j libn64_context_save
  lui $k1, 0x8000

libn64_panic_from_isr_return:
  mfc0 $k0, $14
  sw $k0, 0x074($k1)
  j libn64_panic_from_isr
  mfc0 $k0, $13

# -------------------------------------------------------------------
#  Breakpoint exception handler.
# -------------------------------------------------------------------
.align 5
libn64_exception_handler_break:
  bltz $k0, libn64_panic
  mfc1 $k1, $14
  addiu $k1, $k1, 0x4
  mtc1 $k1, $14
  eret

# -------------------------------------------------------------------
#  Interrupt exception handler (RCP, 64DD, timer, etc.)
# -------------------------------------------------------------------
.align 5
libn64_exception_handler_interrupt:
  andi $k1, $k0, 0x0400
  bne $k1, $zero, libn64_exception_handler_interrupt_infrequent
  lui $k1, 0xA430

# Handle a RCP interrupt:
libn64__exception_handler_rcp_interrupt:
  lw $k0, 0x8($k1)
  eret

# Handle infrequent interrupts (64DD, timer, and other unhandled interrupts)
.align 5
libn64_exception_handler_interrupt_infrequent:
  andi $k1, $k0, 0x8000
  beq $k1, $zero, libn64_exception_handler_timer_interrupt
  andi $k1, $k0, 0x0800
  beql $k1, $zero, libn64_exception_handler_64dd_interrupt
  lui $k0, 0xA500

# We got an unexpected interrupt. Since we don't know how to handle it, panic.
# Currently, this will happen for software interrupts and the Indy debugger-
# reserved external interrupts.
#
# In the future, when 64DD is supported, we should move this label elsewhere.
libn64_exception_handler_64dd_interrupt:
  j libn64_panic
  addu $k0, $zero, $zero

# A timer interrupt occurred. Bounce the compare register to silence it.
# In the future, we probably want to pump a message out on a message queue
# or something here to acknowledge that a timer interrupt occurred.
.align 5
libn64_exception_handler_timer_interrupt:
  mfc1 $k1, $11
  mtc1 $k1, $11
  eret

# -------------------------------------------------------------------
#  TLB exception handler (Mod, TLBL, TLBS)
# -------------------------------------------------------------------
.align 5
libn64_exception_handler_tlb:
  j libn64_panic
  nop

# -------------------------------------------------------------------
#  Syscall exception handler.
#
#  Advance the exception program counter by a word so we return to
#  the instruction following the system call (don't bother trying
#  to support running the instruction from a branch delay slot).
#
#  Unfortunately, saving EPC back to CP0 would mean we would need to
#  fetch an extra cache line's worth of instructions, so we defer the
#  restoration of EPC to the syscall itself.
# -------------------------------------------------------------------
.align 5
libn64_exception_handler_syscall:
  addiu $k1, $k0, -(LIBN64_SYSCALL_INVALID << 2)
  bgez $k1, libn64_panic
.set at
.set reorder
  lw $k0, libn64_syscall_table($k0)
.set noat
.set noreorder
  mfc0 $k1, $14
  jr $k0
  addiu $k1, $k1, 0x4

.size libn64_exception_handler,.-libn64_exception_handler

.section .exception.routines, "ax", @progbits

# -------------------------------------------------------------------
#  Inserts a thread ($k1) into a thread queue ($k0). Before calling,
#  save $ra to 0x4($k0) if desired; it will be reloaded after the
#  jr $ra.
#
#  Routine fits in 3 cache lines. The only stalling that should occur
#  is for data accesses that miss the cache, which is unavoidable.
#
#  Clobbers: $k1, $a0-$a3
# -------------------------------------------------------------------
.global libn64_exception_handler_queue_thread
.type libn64_exception_handler_queue_thread, @function
.align 5
libn64_exception_handler_queue_thread:

# Increment the thread queue count, write it back.
  lw $a0, ($k0)
  lw $at, 0x190($k1)
  addiu $a0, $a0, 0x1
  sw $a0, ($k0)

# Construct a new entry within the thread queue (index = $a0).
# Branch to the while loop condition check (count > 1 check).
  sll $a0, $a0, 0x3
  addu $a1, $a0, $k0
  sw $k1, 0x0($a1)
  j libn64_exception_handler_queue_thread_loop_check
  sw $at, 0x4($a1)

# Continually swap the contents of the queue entry at index '$a0'
# with the one at its parent as long as the parent has a priority
# that is greater than what is rooted at index '$a0'.
#
# Register allocation within this loop works like so:
#   $a0 = pos << 3, swap address for pos
#   $a1 = parent << 3
#   $a2 = swap address for parent
#   $a3 = priority check, parent contents
#   $k1 = scratch
libn64_exception_handler_queue_thread_loop:
  addu $a2, $a1, $k0
  lw $a3, 0x4($a2)
  addu $a0, $a0, $k0
  subu $a3, $a3, $at
  bgez $a3, libn64_exception_handler_queue_thread_finish
  ld $a3, ($a2)
  ld $k1, ($a0)
  sd $a3, ($a0)
  addu $a0, $a1, $zero
  sd $k1, ($a2)

libn64_exception_handler_queue_thread_loop_check:
  srl $a1, $a0, 0x4
  bgtz $a1, libn64_exception_handler_queue_thread_loop
  sll $a1, $a1, 0x3

libn64_exception_handler_queue_thread_finish:
  jr $ra
  lw $ra, 0x4($k0)

.size libn64_exception_handler_queue_thread,. \
    -libn64_exception_handler_queue_thread

# -------------------------------------------------------------------
#  Pop the next highest-priority thread off $k0's thread queue. At
#  least one thread must be in the queue (or bad things happen). On
#  return, the removed thread is loaded to $k1. Before calling, save
#  $ra to 0x4($k0) if desired; it will be reloaded after the jr $ra.
#
#  Routine fits in 4 cache lines. The one load-after-use stall seems
#  to be unavoidable and causes an unnecessary one cycle delay once
#  per loop. Other than that, the only stalls are due to data
#  accesses that miss the cache.
#
#  Clobbers: $a0-$a3
# -------------------------------------------------------------------
.global libn64_exception_handler_dequeue_thread
.type libn64_exception_handler_dequeue_thread, @function
.align 5
libn64_exception_handler_dequeue_thread:

# Decrement the thread count, write it back.
# Take the value of count /before/ decrementing it.
  lw $a0, ($k0)
  lw $k1, 0x8($k0)
  addiu $a1, $a0, 0x1
  sw $a1, ($k0)

# Promote the last thing in the heap to the front.
  sll $a0, $a0, 0x3
  addu $a1, $a0, $k0
  ld $a2, ($a1)
  addiu $a1, $zero, 0x8
  sd $s2, 0x8($k0)

# Drop the root node down into the heap.
#
# Register allocation within this loop works like so:
#   $a0 = count << 3
#   $a1 = pos << 3, swap address for pos
#   $a2 = left/right << 3), swap address for left/right
#   $a3 = index check << 3, priority check, scratch
#   $at = scratch
libn64_exception_handler_dequeue_thread_loop:
  addu $a2, $a1, $a1
  subu $a3, $a2, $a0
  bgtz $a3, libn64_exception_handler_dequeue_thread_finish
  addu $a1, $a1, $k0
  bgez $a3, libn64_exception_handler_dequeue_thread_leftonly
  addu $a2, $a2, $k0

# Check which child (left or right) has a higher priority.
# Pick that child as the potential swap candidate.
  lw $a3, 0x4($a2)
  lw $at, 0xC($a2) # stall; use after load
  subu $at, $at, $a3
  bgtzl $at, libn64_exception_handler_dequeue_thread_leftonly
  addiu $a2, $a2, 0x8

libn64_exception_handler_dequeue_thread_leftonly:
  lw $a3, 0x4($a2)
  lw $at, 0x4($a1)
  subu $at, $at, $a3
  bgtz $at, libn64_exception_handler_dequeue_thread_finish
  ld $at, ($a2)
  ld $a3, ($a1)
  sd $at, ($a1)
  subu $a1, $a2, $k0
  j libn64_exception_handler_dequeue_thread_loop
  sd $a3, ($a2)

libn64_exception_handler_dequeue_thread_finish:
  jr $ra
  lw $ra, 0x4($k0)

.size libn64_exception_handler_dequeue_thread,. \
    -libn64_exception_handler_dequeue_thread

.set gp=default
.set fp=default
.set at
.set reorder

