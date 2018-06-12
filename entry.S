
#include "asm.h"
#include "memlayout.h"
#include "mmu.h"
#include "param.h"

# Multiboot header.  Data to direct multiboot loader.
.p2align 2
.text
.globl multiboot_header
multiboot_header:
  #define magic 0x1badb002
  #define flags 0
  .long magic
  .long flags
  .long (-magic-flags)

# By convention, the _start symbol specifies the ELF entry point.
# Since we haven't set up virtual memory yet, our entry point is
# the physical address of 'entry'.
.globl _start
_start = V2P_WO(entry)

# Entering xv6 on boot processor, with paging off.
.globl entry
entry:
  # Turn on page size extension for 4Mbyte pages
  //movl    %cr4, %eax
  //orl     $(CR4_PSE), %eax
  //movl    %eax, %cr4
  # Set page directory
  //movl    $(V2P_WO(entrypgdir)), a5


	//set MXR 1
	li t1,0x80000
//	csrr t0,mstatus
//	or	t0,t0,t1
//	csrw mstatus,t0
	csrs mstatus,t1
/*
  la	  t0,V2P_WO(entrypgdir)
  li 	  t4,0x80000000
  sub 	  t0,t0,t4
*/
  la	  t0,entrypgdir
  srli    t0,t0,12
  csrw    satp, t0
  # Turn on paging.
  csrr    t0,satp
  li	 t1,0x80000000
  or     t0,t0,t1
  csrw    satp, t0

  # Set up the stack pointer.
  //movl $(stack + KSTACKSIZE), sp
  la	sp, (stack + KSTACKSIZE)
  li	t1,KERNBASE
  add  sp, sp,t1

  # Jump to main(), and switch to executing at
  # high addresses. The indirect call is needed because
  # the assembler produces a PC-relative instruction
  # for a direct jump.
  //mov $main, r4
  la t0, main
  li	 t1,KERNBASE
  add t0,t0,t1
  jr t0

.comm stack, KSTACKSIZE
