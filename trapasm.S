#include "mmu.h"

  # vectors.S sends all traps here.
#.globl alltraps
#alltraps:
/*
  # Build trap frame.
  pushl %ds
  pushl %es
  pushl %fs
  pushl %gs
  pushal
  
  # Set up data segments.
  movw $(SEG_KDATA<<3), %ax
  movw %ax, %ds
  movw %ax, %es

  # Call trap(tf), where tf=%esp
  pushl %esp
  call trap
  addl $4, %esp

  # Return falls through to trapret...
.globl trapret
trapret:
  popal
  popl %gs
  popl %fs
  popl %es
  popl %ds
  addl $0x8, %esp  # trapno and errcode
  iret
*/

.globl alltraps
alltraps:

	sw t6,-4(sp)
	sw t5,-8(sp)
	sw t4,-12(sp)
	mv t6,sp
	csrr t5,mcause
	#andi t5,t5,0x7fffffff
	li t4,0x80000003
	beq t5,t4,trapmain
	li t4,0x80000007
	beq t5,t4,trapmain
	li t4,0x8000000b
	beq t5,t4,trapmain

	csrr sp,mscratch

trapmain:
	addi sp,sp,-136
	
	sw ra,0(sp)
	sw gp,4(sp)
	sw tp,8(sp)
	sw t0,12(sp)
	sw t1,16(sp)
	sw t2,20(sp)
	sw s0,24(sp)
	sw s1,28(sp)
	sw a0,32(sp)
	sw a1,36(sp)
	sw a2,40(sp)
	sw a3,44(sp)
	sw a4,48(sp)
	sw a5,52(sp)
	sw a6,56(sp)
	sw a7,60(sp)
	sw s2,64(sp)
	sw s3,68(sp)
	sw s4,72(sp)
	sw s5,76(sp)
	sw s6,80(sp)
	sw s7,84(sp)
	sw s8,88(sp)
	sw s9,92(sp)
	sw s10,96(sp)
	sw s11,100(sp)
	sw t3,104(sp)

	//t3 <= sp
	mv t3,t6
	lw t4,-12(t3)
	lw t5,-8(t3)
	lw t6,-4(t3)
	sw t4,108(sp)
	sw t5,112(sp)
	sw t6,116(sp)

	csrr t0,mepc
	csrr t1,mcause
	csrr t2,mstatus
	sw t0,120(sp)
	sw t1,124(sp)
	sw t2,128(sp)

	sw t3,132(sp)
	mv a0,sp
	jal trap

	#addi sp,sp,4

# Return falls through to trapret...
.globl trapret
trapret:
	//addi sp,sp,-136
	
	lw ra,0(sp)
	lw gp,4(sp)
	lw tp,8(sp)
	lw t0,12(sp)
	lw t1,16(sp)
	lw t2,20(sp)
	lw s0,24(sp)
	lw s1,28(sp)
	lw a0,32(sp)
	lw a1,36(sp)
	lw a2,40(sp)
	lw a3,44(sp)
	lw a4,48(sp)
	lw a5,52(sp)
	lw a6,56(sp)
	lw a7,60(sp)
	lw s2,64(sp)
	lw s3,68(sp)
	lw s4,72(sp)
	lw s5,76(sp)
	lw s6,80(sp)
	lw s7,84(sp)
	lw s8,88(sp)
	lw s9,92(sp)
	lw s10,96(sp)
	lw s11,100(sp)

	lw t3,120(sp)
	lw t4,124(sp)
	lw t5,128(sp)
	csrw mepc,t3
	csrw mcause,t4
	csrw mstatus,t5

	lw t3,104(sp)
	lw t4,108(sp)
	lw t5,112(sp)
	lw t6,116(sp)

	addi sp,sp,132
	#addi sp,sp,4  # trapno #and errcode
	lw sp,0(sp)
	mret
