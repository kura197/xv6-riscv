0850 #include "asm.h"
0851 #include "memlayout.h"
0852 #include "mmu.h"
0853 #include "param.h"
0854 
0855 # Multiboot header.  Data to direct multiboot loader.
0856 .p2align 2
0857 .text
0858 .globl multiboot_header
0859 multiboot_header:
0860   #define magic 0x1badb002
0861   #define flags 0
0862   .long magic
0863   .long flags
0864   .long (-magic-flags)
0865 
0866 # By convention, the _start symbol specifies the ELF entry point.
0867 # Since we haven't set up virtual memory yet, our entry point is
0868 # the physical address of 'entry'.
0869 .globl _start
0870 _start = V2P_WO(entry)
0871 
0872 # Entering xv6 on boot processor, with paging off.
0873 .globl entry
0874 entry:
0875   # Turn on page size extension for 4Mbyte pages
0876   //movl    %cr4, %eax
0877   //orl     $(CR4_PSE), %eax
0878   //movl    %eax, %cr4
0879   # Set page directory
0880   //movl    $(V2P_WO(entrypgdir)), a5
0881 
0882 
0883 	//set MXR 1
0884 	li t1,0x80000
0885 //	csrr t0,mstatus
0886 //	or	t0,t0,t1
0887 //	csrw mstatus,t0
0888 	csrs mstatus,t1
0889 /*
0890   la	  t0,V2P_WO(entrypgdir)
0891   li 	  t4,0x80000000
0892   sub 	  t0,t0,t4
0893 */
0894   la	  t0,entrypgdir
0895   srli    t0,t0,12
0896   csrw    satp, t0
0897   # Turn on paging.
0898   csrr    t0,satp
0899   li	 t1,0x80000000
0900   or     t0,t0,t1
0901   csrw    satp, t0
0902 
0903   # Set up the stack pointer.
0904   //movl $(stack + KSTACKSIZE), sp
0905   la	sp, (stack + KSTACKSIZE)
0906   li	t1,KERNBASE
0907   add  sp, sp,t1
0908 
0909   # Jump to main(), and switch to executing at
0910   # high addresses. The indirect call is needed because
0911   # the assembler produces a PC-relative instruction
0912   # for a direct jump.
0913   //mov $main, r4
0914   la t0, main
0915   li	 t1,KERNBASE
0916   add t0,t0,t1
0917   jr t0
0918 
0919 .comm stack, KSTACKSIZE
0920 
0921 
0922 
0923 
0924 
0925 
0926 
0927 
0928 
0929 
0930 
0931 
0932 
0933 
0934 
0935 
0936 
0937 
0938 
0939 
0940 
0941 
0942 
0943 
0944 
0945 
0946 
0947 
0948 
0949 
