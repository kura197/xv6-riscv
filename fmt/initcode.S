8650 # Initial process execs /init.
8651 # This code runs in user space.
8652 
8653 #include "syscall.h"
8654 #include "traps.h"
8655 
8656 
8657 # exec(init, argv)
8658 /*
8659 .globl start
8660 start:
8661   pushl $argv
8662   pushl $init
8663   pushl $0  // where caller pc would be
8664   movl $SYS_exec, %eax
8665   int $T_SYSCALL
8666 
8667 # for(;;) exit();
8668 exit:
8669   movl $SYS_exit, %eax
8670   int $T_SYSCALL
8671   jmp exit
8672 */
8673 .globl start
8674 start:
8675 	addi sp, sp,-12
8676 	la	 t0, argv
8677 	sw	 t0, 8(sp)
8678 	la	 t1, init
8679 	sw	 t1, 4(sp)
8680 	sw	 zero, 0(sp)
8681 	li	 a0, SYS_exec
8682 	ecall
8683 
8684 exit:
8685 	li	 a0, SYS_exec
8686 	ecall
8687 	j 	 exit
8688 
8689 # char init[] = "/init\0";
8690 init:
8691   .string "/init\0"
8692 
8693 # char *argv[] = { init, 0 };
8694 #.p2align 2
8695 .p2align 4
8696 argv:
8697   .long init
8698   .long 0
8699 
