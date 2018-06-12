8700 #include "syscall.h"
8701 #include "traps.h"
8702 
8703 /*
8704 #define SYSCALL(name) \
8705   .globl name; \
8706   name: \
8707     movl $SYS_ ## name, %eax; \
8708     int $T_SYSCALL; \
8709     ret
8710 */
8711 
8712 #define SYSCALL_arg0(name) \
8713   .globl name; \
8714   name: \
8715 	addi sp,sp,-4; \
8716 	sw zero,0(sp); \
8717     li a0,SYS_ ## name; \
8718     ecall;	\
8719 	addi sp,sp,4; \
8720     ret;
8721 
8722 #define SYSCALL_arg1(name) \
8723   .globl name; \
8724   name: \
8725 	addi sp,sp,-8; \
8726 	sw a0,4(sp); \
8727 	sw zero,0(sp); \
8728     li a0,SYS_ ## name; \
8729     ecall;	\
8730 	addi sp,sp,8; \
8731     ret
8732 
8733 #define SYSCALL_arg2(name) \
8734   .globl name; \
8735   name: \
8736 	addi sp,sp,-12; \
8737 	sw a1,8(sp); \
8738 	sw a0,4(sp); \
8739 	sw zero,0(sp); \
8740     li a0,SYS_ ## name; \
8741     ecall;	\
8742 	addi sp,sp,12; \
8743     ret
8744 
8745 #define SYSCALL_arg3(name) \
8746   .globl name; \
8747   name: \
8748 	addi sp,sp,-16; \
8749 	sw a2,12(sp); \
8750 	sw a1,8(sp); \
8751 	sw a0,4(sp); \
8752 	sw zero,0(sp); \
8753     li a0,SYS_ ## name; \
8754     ecall;	\
8755 	addi sp,sp,16; \
8756     ret
8757 
8758 SYSCALL_arg0(fork)
8759 SYSCALL_arg0(exit)
8760 SYSCALL_arg0(wait)
8761 SYSCALL_arg1(pipe)
8762 SYSCALL_arg3(read)
8763 SYSCALL_arg3(write)
8764 SYSCALL_arg1(close)
8765 SYSCALL_arg1(kill)
8766 SYSCALL_arg2(exec)
8767 SYSCALL_arg2(open)
8768 SYSCALL_arg3(mknod)
8769 SYSCALL_arg1(unlink)
8770 SYSCALL_arg2(fstat)
8771 SYSCALL_arg2(link)
8772 SYSCALL_arg1(mkdir)
8773 SYSCALL_arg1(chdir)
8774 SYSCALL_arg1(dup)
8775 SYSCALL_arg0(getpid)
8776 SYSCALL_arg1(sbrk)
8777 SYSCALL_arg1(sleep)
8778 SYSCALL_arg0(uptime)
8779 /*
8780 #define SYSCALL(name) \
8781   .globl name; \
8782   name: \
8783     li a0,SYS_ ## name; \
8784     ecall;
8785     mret
8786 SYSCALL(fork)
8787 SYSCALL(exit)
8788 SYSCALL(wait)
8789 SYSCALL(pipe)
8790 SYSCALL(read)
8791 SYSCALL(write)
8792 SYSCALL(close)
8793 SYSCALL(kill)
8794 SYSCALL(exec)
8795 SYSCALL(open)
8796 SYSCALL(mknod)
8797 SYSCALL(unlink)
8798 SYSCALL(fstat)
8799 SYSCALL(link)
8800 SYSCALL(mkdir)
8801 SYSCALL(chdir)
8802 SYSCALL(dup)
8803 SYSCALL(getpid)
8804 SYSCALL(sbrk)
8805 SYSCALL(sleep)
8806 SYSCALL(uptime)
8807 */
8808 
8809 
8810 
8811 
8812 
8813 
8814 
8815 
8816 
8817 
8818 
8819 
8820 
8821 
8822 
8823 
8824 
8825 
8826 
8827 
8828 
8829 
8830 
8831 
8832 
8833 
8834 
8835 
8836 
8837 
8838 
8839 
8840 
8841 
8842 
8843 
8844 
8845 
8846 
8847 
8848 
8849 
