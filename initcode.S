# Initial process execs /init.
# This code runs in user space.

#include "syscall.h"
#include "traps.h"


# exec(init, argv)
/*
.globl start
start:
  pushl $argv
  pushl $init
  pushl $0  // where caller pc would be
  movl $SYS_exec, %eax
  int $T_SYSCALL

# for(;;) exit();
exit:
  movl $SYS_exit, %eax
  int $T_SYSCALL
  jmp exit
*/
.globl start
start:
	addi sp, sp,-12
	la	 t0, argv
	sw	 t0, 8(sp)
	la	 t1, init
	sw	 t1, 4(sp)
	sw	 zero, 0(sp)
	li	 a0, SYS_exec
	ecall

exit:
	li	 a0, SYS_exec
	ecall
	j 	 exit

# char init[] = "/init\0";
init:
  .string "/init\0"

# char *argv[] = { init, 0 };
#.p2align 2
.p2align 4
argv:
  .long init
  .long 0

