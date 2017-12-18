#
# libn64/os/asm/context.s: libn64 context switching routines.
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
#  Blocks the active thread, places the next ready one on the CPU.
#  Intended to be invoked as the the return of a libn64_context_save.
# -------------------------------------------------------------------
.global libn64_block_thread
.type libn64_block_thread, @function
.align 5
libn64_block_thread:
  mfc0 $at, $12
  lw $k1, 0x8($k0)
  ori $at, $at, 0x8004

# Mark the thread blocked and dequeue it.
  jal libn64_exception_handler_dequeue_thread
  sw $at, 0x80($k1)

# Switch to the next active thread.
  j libn64_context_restore
  lw $k1, 0x8($k0)

.size libn64_block_thread,.-libn64_block_thread

# -------------------------------------------------------------------
#  Queues a previously blocked thread and switches to it if necessary.
# -------------------------------------------------------------------
.global libn64_maybe_unblock_thread
.type libn64_maybe_unblock_thread, @function
.align 5
libn64_maybe_unblock_thread:
  lw $k0, 0x80($a0)
  andi $at, $k0, 0x8000
  beq $at, $zero, libn64_unblock_eret
  xori $at, $k0, 0x8000

libn64_unblock_thread:
  lw $k0, 0x198($a0)
  sw $at, 0x80($a0)
  lw $at, 0x198($k1)
  subu $at, $at, $k0
  la $k0, libn64_unblock_hp_thread
  bltz $at, libn64_context_save
  lui $at, 0x8000

# Unblock a lower priority thread.
libn64_unblock_lp_thread:
  cache 0xD, 0x400($at)
  lw $k0, 0x420($at)
  sw $a0, 0x400($at)
  sw $a1, 0x404($at)
  addu $k1, $a0, $zero

  sw $ra, 0x4($k0)
  jal libn64_exception_handler_queue_thread
  sw $a2, 0x408($at)

  lui $at, 0x8000
  lw $a0, 0x400($at)
  lw $a1, 0x404($at)
  lw $a2, 0x408($at)
  cache 0x11, 0x400($at)

libn64_unblock_eret:
  eret

# Unblock a higher priority thread.
libn64_unblock_hp_thread:
  lui $k0, 0x8000
  lw $k0, 0x420($k0)

# Mark the thread unblocked and queue it.
  jal libn64_exception_handler_queue_thread
  addu $k1, $a0, $zero

# Switch to the next active thread.
  j libn64_context_restore
  lw $k1, 0x8($k0)

.size libn64_maybe_unblock_thread,.-libn64_maybe_unblock_thread

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
.global libn64_context_restore
.type libn64_context_restore, @function
.align 5
libn64_context_restore:
  lw $v0, 0x074($k1)
  lw $a0, 0x01C($k1)
  lw $a1, 0x010($k1)
  mtlo $v0
  lw $v0, 0x078($k1)
  lw $a2, 0x014($k1)
  lw $a3, 0x018($k1)
  mthi $v0
  lw $v0, 0x07C($k1)
  lw $t0, 0x00C($k1)
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
  lw $at, 0x19C($k1)
  lw $t4, 0x02C($k1)
  lw $t5, 0x030($k1)
  mtc0 $at, $30
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
  sll $v0, $v0, 0x2
  bltz $v0, libn64_context_restore_fpu
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
  sll $v0, $v0, 0x3
  lw $v1, 0x088($k1)
  bgez $v0, libn64_context_restore_done
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
.global libn64_context_save
.type libn64_context_save, @function
.align 5
libn64_context_save:
  cache 0xD, 0x000($k1)
  sw $at, 0x000($k1)
  addiu $at, $k1, 0x060

libn64_context_save_loop:
  cache 0xD, 0x010($at)
  bne $at, $k1, libn64_context_save_loop
  addiu $at, $at, -0x10

  sw $v0, 0x004($k1)
  sw $v1, 0x008($k1)
  sw $a0, 0x01C($k1)
  sw $a1, 0x010($k1)
  sw $a2, 0x014($k1)
  sw $a3, 0x018($k1)
  sw $t0, 0x00C($k1)
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
  mfc0 $v0, $30
  sw $s0, 0x03C($k1)
  sw $s1, 0x040($k1)
  sw $v0, 0x19C($k1)
  sw $s2, 0x044($k1)
  sw $s3, 0x048($k1)
  sw $s4, 0x04C($k1)
  sw $s5, 0x050($k1)
  sw $s6, 0x054($k1)
  sw $s7, 0x058($k1)
  sw $t8, 0x05C($k1)
  sw $t9, 0x060($k1)
  sw $gp, 0x064($k1)
  mfc0 $v0, $14
  sw $sp, 0x068($k1)
  sw $v0, 0x07C($k1)
  mfc0 $v0, $10
  sw $fp, 0x06C($k1)
  sw $v0, 0x084($k1)
  sll $v0, $at, 0x2
  bltz $v0, libn64_context_save_fpu
  sw $ra, 0x070($k1)

libn64_context_save_done:
  lui $at, 0x8000
  jr $k0
  lw $k0, 0x420($at)

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
  sw $v0, 0x088($k1)
  sdc1 $20, 0x0E0($k1)
  sdc1 $22, 0x0E8($k1)
  sdc1 $24, 0x0F0($k1)
  sdc1 $26, 0x0F8($k1)
  sll $v0, $at, 0x5
  sdc1 $28, 0x100($k1)
  bgez $v0, libn64_context_save_done
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

.set gp=default
.set fp=default
.set at
.set reorder

