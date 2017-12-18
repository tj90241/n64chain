#
# libn64/os/asm/exception.s: libn64 exception handler.
#
# n64chain: A (free) open-source N64 development toolchain.
# Copyright 2014-16 Tyler J. Stachecki <stachecki.tyler@gmail.com>
#
# This file is subject to the terms and conditions defined in
# 'LICENSE', which is part of this source code package.
#

#include <syscall.h>

.section .text.libn64, "ax", @progbits

.set gp=64
.set fp=64
.set noat
.set noreorder

# -------------------------------------------------------------------
#  This part of the exception handler gets loaded directly at 0x80000000
#  by the loader. Certain large portions of the context handler (i.e., the
#  context switching) live with the rest of the libn64 (@ 0x80000480+) due
#  to the fact that we only have about 0x480 bytes for the entirety of this
#  routine without doing a lot more relocation work.
# -------------------------------------------------------------------
.section .exception.tlbmiss, "ax", @progbits

.global libn64_tlb_exception_handler
.type libn64_tlb_exception_handler, @function
.align 5
libn64_tlb_exception_handler:

# Ensure that the address is within 8MB of the top of the TLB-mapped
# address range. If not, it can't be a stack address, so panic.
  mfc0 $k0, $8
  lui $k1, 0x7F80
  subu $k0, $k0, $k1

# Get the current/active thread, save $at so we have some breathing room.
  lui $k1, 0x8000
  lw $k1, 0x420($k1)
  bltz $k0, libn64_tlb_exception_handler_badvaddr
  sw $at, 0x4($k1)

# Load the L1 page table entry ($at) and L2 page table index ($k0).
# Save $ra so we can use jal without ripping our hair out.
  mtc0 $ra, $30
  lw $k1, 0x8($k1)
  srl $at, $k0, 0x11
  andi $at, $at, 0x3E
  addu $k1, $k1, $at
  lhu $at, 0x1C0($k1)
  srl $ra, $k0, 0xB
  bne $at, $zero, libn64_tlb_exception_handler_get_page_entry
  andi $k0, $ra, 0x7E

# Don't have L2 alloc'd: check L2 page table chain for an existing entry.
  lui $ra, 0x8000
  lw $ra, 0x42C($ra)
  bne $ra, $zero, libn64_tlb_exception_handler_init_l2_stack_entry
  sh $k0, 0x1C0($k1)

# Need to allocate more L2 entries: add them to the L2 page table chain.
  jal libn64_exception_handler_allocpage
  sll $k0, $k0, 0xC
  lui $at, 0x8000
  or $ra, $k0, $at
  sw $ra, 0x42C($at)

libn64_tlb_exception_handler_alloc_l2_stack_entries_loop:
  addiu $ra, $ra, 0x80
  cache 0xD, -0x80($ra)
  xor $at, $k0, $ra
  andi $at, $at, 0x1000
  beql $at, $zero, libn64_tlb_exception_handler_alloc_l2_stack_entries_loop
  sw $ra, -0x80($ra)
  sw $zero, -0x80($ra)
  addiu $ra, $ra, -0x1000

# Take the first entry from the L2 page table chain and wipe it clean.
libn64_tlb_exception_handler_init_l2_stack_entry:
  lw $k0, 0x0($ra)
  lui $at, 0x8000
  sw $k0, 0x42C($at)
  addiu $at, $ra, 0x80

libn64_tlb_exception_handler_clear_l2_stack_entry_loop:
  cache 0xD, -0x10($at)
  addiu $at, $at, -0x10
  sd $zero, 0x0($at)
  bne $at, $ra, libn64_tlb_exception_handler_clear_l2_stack_entry_loop
  sd $zero, 0x8($at)

  srl $at, $ra, 0x7
  lhu $k0, 0x1C0($k1)
  sh $at, 0x1C0($k1)

# Get physical page frame allocated to the thread/address.
libn64_tlb_exception_handler_get_page_entry:
  sll $k1, $at, 0x7
  addu $k1, $k1, $k0
  lui $k0, 0x8000
  or $k1, $k1, $k0
  lhu $k0, 0x0($k1)

  bne $k0, $zero, libn64_tlb_exception_handler_update_tlb
  mtc0 $zero, $5 # Always using 4kB pages for the stack

  jal libn64_exception_handler_allocpage
  sh $k0, 0x0($k1)

# For the first page, mark it valid/dirty and assign the physical page.
# For the adjacent page entry, only mark it valid: we'll fill on TLBM.
libn64_tlb_exception_handler_update_tlb:
  lui $at, 0x8000
  lw $at, 0x420($at)
  mfc0 $ra, $8
  lw $k0, 0x8($at)
  srl $ra, $ra, 0xD
  sll $ra, $ra, 0xD
  srl $k0, $k0, 0x9
  andi $k0, $k0, 0xFF
  or $ra, $ra, $k0
  mtc0 $ra, $10
  tlbp
  lw $at, 0x4($at)

  # Probe for an existing entry; if we missed, high bit will be set.
  # If we missed, we want to keep the existing index so we update it.
  mfc0 $ra, $0
  lhu $k0, 0x0($k1)
  andi $k1, $k1, 0x2
  sll $k0, $k0, 0x6
  ori $k0, $k0, 0x6

  # On a hit, read probed TLB entries. On a miss, prepare new entries.
  srl $ra, $ra, 31
  beql $ra, $zero, libn64_tlb_exception_handler_update_tlb_continue
  tlbr
  mtc0 $zero, $3
  mtc0 $zero, $2

# Update EntryLo0/1 (if its an even page, Lo0, else odd and Lo1).
libn64_tlb_exception_handler_update_tlb_continue:
  bnel $k1, $zero, libn64_tlb_exception_handler_update_tlb_continue_odd
  mtc0 $k0, $3
  mtc0 $k0, $2

# Use random replacement when writing a new TLB entry; index otherwise.
libn64_tlb_exception_handler_update_tlb_continue_odd:
  beql $ra, $zero, libn64_tlb_exception_handler_finish
  tlbwi
  tlbwr

# Restore the thread's $at and return from the exception; we're done.
libn64_tlb_exception_handler_finish:
  mfc0 $ra, $30
  eret

# Address is not within 8MB "stack boundary" of the top of *useg. Since
# we only support virtual stack addresses, this is fatal, so panic.
libn64_tlb_exception_handler_badvaddr:
  la $k0, libn64_tlb_exception_handler_badvaddr_return
  j libn64_context_save
  lui $k1, 0x8000

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

.fill 0x180-(.-libn64_tlb_exception_handler), 1, 0x00
.size libn64_tlb_exception_handler,.-libn64_tlb_exception_handler

.section .exception.general, "ax", @progbits

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
  bltz $k1, libn64_tlb_exception_handler
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

libn64_tlb_exception_handler_badvaddr_return:
  j libn64_tlb_exception_handler_panic
  addiu $k0, $zero, 0x80

libn64_miss_exception_handler_allocfail_return:
  j libn64_tlb_exception_handler_panic
  addiu $k0, $zero, 0x84

libn64_panic:
  la $k0, libn64_exception_handler_panic
  j libn64_context_save
  lui $k1, 0x8000

libn64_exception_handler_panic:
  mfc0 $k0, $13

libn64_tlb_exception_handler_panic:
  lui $sp, 0x8000
  addiu $sp, $sp, 0x400
  la $gp, _gp

  mfc0 $at, $14
  j libn64_panic_from_isr
  sw $at, 0x074($k1)

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
  bne $k1, $zero, libn64_exception_handler_rcp_interrupt
  andi $k1, $k0, 0x8000
  bne $k1, $zero, libn64_exception_handler_timer_interrupt
  nop

# We got an unexpected interrupt. Since we don't know how to handle it, panic.
# Currently, this will happen for software interrupts and the Indy debugger-
# reserved external interrupts.
libn64_exception_handler_64dd_interrupt:
  j libn64_panic
  addu $k0, $zero, $zero

# A timer interrupt occurred. Bounce the compare register to silence it
# and increment the "rollover" counter that we use to track time.
libn64_exception_handler_timer_interrupt:
  mfc0 $k1, $11
  mtc0 $k1, $11
  lui $k0, 0x8000
  lw $k1, 0x448($k0)
  addiu $k1, $k1, 0x1
  sw $k1, 0x448($k0)
  eret

# Handle a RCP interrupt: pump messages out to the listeners.
.align 5
libn64_exception_handler_rcp_interrupt:
  lui $k0, 0x8000
  cache 0xD, 0x400($k0)
  sw $a0, 0x400($k0)
  mtc0 $ra, $30
  sw $a1, 0x404($k0)
  sw $a2, 0x408($k0)
  j libn64_send_rcp_messages
  sw $at, 0x40C($k0)

.size libn64_exception_handler,.-libn64_exception_handler

.section .exception.routines, "ax", @progbits

# -------------------------------------------------------------------
#  Allocate the next free page from the page cache.
#
#  Routine fits in 3 cache lines. There are a few load-after-use
#  stalls, but they seem to largely be unavoidable given how few
#  registers we have to work with. PFN of the allocated page is
#  returned in $k0.
#
#  Clobbers: $at, $k0
# -------------------------------------------------------------------
.global libn64_exception_handler_allocpage
.type libn64_exception_handler_allocpage, @function
.align 5
libn64_exception_handler_allocpage:
  lui $at, 0x8000
  lhu $k0, 0x410($at)
  addiu $at, $at, 0x410

libn64_exception_handler_allocpage_scanbanks:
  bnel $k0, $zero, libn64_exception_handler_allocpage_found
  addiu $k0, $k0, -0x1
  addiu $at, $at, 0x2
  andi $k0, $at, 0xF
  bnel $k0, $zero, libn64_exception_handler_allocpage_scanbanks
  lhu $k0, 0x0($at)

# Ran out of pages in the first 8MB to allocate: panic.
libn64_miss_exception_handler_allocfail:
  la $k0, libn64_miss_exception_handler_allocfail_return
  j libn64_context_save
  lui $k1, 0x8000

libn64_exception_handler_allocpage_found:
  sh $k0, 0x0($at)
  addu $k0, $k0, $k0
  andi $at, $at, 0xF
  sll $at, $at, 0x8
  addu $k0, $k0, $at
  lui $at, 0x8000
  lw $at, 0x428($at)
  addu $at, $at, $k0
  addiu $ra, $ra, -0x4
  jr $ra
  lhu $k0, 0x0($at)

.size libn64_exception_handler_allocpage,. \
    -libn64_exception_handler_allocpage

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
#  Clobbers: $k1, $a0-$a3
# -------------------------------------------------------------------
.global libn64_exception_handler_dequeue_thread
.type libn64_exception_handler_dequeue_thread, @function
.align 5
libn64_exception_handler_dequeue_thread:

# Decrement the thread count, write it back.
# Take the value of count /before/ decrementing it.
  lw $a0, ($k0)
  lw $k1, 0x8($k0)
  addiu $a1, $a0, -0x8
  sw $a1, ($k0)

# Promote the last thing in the heap to the front.
  addu $a1, $a0, $k0
  ld $a2, ($a1)
  addiu $a1, $zero, 0x8
  sd $a2, 0x8($k0)

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

# -------------------------------------------------------------------
#  Inserts a thread ($k1) into a thread queue ($k0). Before calling,
#  save $ra to 0x4($k0) if desired; it will be reloaded after the
#  jr $ra.
#
#  Routine fits in 3 cache lines. The only stalling that should occur
#  is for data accesses that miss the cache, which is unavoidable.
#
#  Clobbers: $k1, $a0-$a2
# -------------------------------------------------------------------
.global libn64_exception_handler_queue_thread
.type libn64_exception_handler_queue_thread, @function
.align 5
libn64_exception_handler_queue_thread:

# Increment the thread queue count, write it back.
  lw $a0, ($k0)
  lw $at, 0x198($k1)
  addiu $a0, $a0, 0x8
  sw $a0, ($k0)

# Construct a new entry within the thread queue (index = $a0).
# Branch to the while loop condition check (count > 1 check).
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
#   $k1 = priority check, parent contents
libn64_exception_handler_queue_thread_loop:
  addu $a2, $a1, $k0
  lw $k1, 0x4($a2)
  addu $a0, $a0, $k0
  subu $k1, $k1, $at
  bgez $k1, libn64_exception_handler_queue_thread_finish
  ld $a2, ($a2)
  ld $k1, ($a0)
  sd $a2, ($a0)
  addu $a0, $a1, $zero
  addu $a2, $a1, $k0
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

.set gp=default
.set fp=default
.set at
.set reorder

