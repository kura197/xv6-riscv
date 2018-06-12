#include "syscall.h"
#include "traps.h"

/*
#define SYSCALL(name) \
  .globl name; \
  name: \
    movl $SYS_ ## name, %eax; \
    int $T_SYSCALL; \
    ret
*/

#define SYSCALL_arg0(name) \
  .globl name; \
  name: \
	addi sp,sp,-4; \
	sw zero,0(sp); \
    li a0,SYS_ ## name; \
    ecall;	\
	addi sp,sp,4; \
    ret;

#define SYSCALL_arg1(name) \
  .globl name; \
  name: \
	addi sp,sp,-8; \
	sw a0,4(sp); \
	sw zero,0(sp); \
    li a0,SYS_ ## name; \
    ecall;	\
	addi sp,sp,8; \
    ret

#define SYSCALL_arg2(name) \
  .globl name; \
  name: \
	addi sp,sp,-12; \
	sw a1,8(sp); \
	sw a0,4(sp); \
	sw zero,0(sp); \
    li a0,SYS_ ## name; \
    ecall;	\
	addi sp,sp,12; \
    ret

#define SYSCALL_arg3(name) \
  .globl name; \
  name: \
	addi sp,sp,-16; \
	sw a2,12(sp); \
	sw a1,8(sp); \
	sw a0,4(sp); \
	sw zero,0(sp); \
    li a0,SYS_ ## name; \
    ecall;	\
	addi sp,sp,16; \
    ret

SYSCALL_arg0(fork)
SYSCALL_arg0(exit)
SYSCALL_arg0(wait)
SYSCALL_arg1(pipe)
SYSCALL_arg3(read)
SYSCALL_arg3(write)
SYSCALL_arg1(close)
SYSCALL_arg1(kill)
SYSCALL_arg2(exec)
SYSCALL_arg2(open)
SYSCALL_arg3(mknod)
SYSCALL_arg1(unlink)
SYSCALL_arg2(fstat)
SYSCALL_arg2(link)
SYSCALL_arg1(mkdir)
SYSCALL_arg1(chdir)
SYSCALL_arg1(dup)
SYSCALL_arg0(getpid)
SYSCALL_arg1(sbrk)
SYSCALL_arg1(sleep)
SYSCALL_arg0(uptime)
/*
#define SYSCALL(name) \
  .globl name; \
  name: \
    li a0,SYS_ ## name; \
    ecall;	
    mret
SYSCALL(fork)
SYSCALL(exit)
SYSCALL(wait)
SYSCALL(pipe)
SYSCALL(read)
SYSCALL(write)
SYSCALL(close)
SYSCALL(kill)
SYSCALL(exec)
SYSCALL(open)
SYSCALL(mknod)
SYSCALL(unlink)
SYSCALL(fstat)
SYSCALL(link)
SYSCALL(mkdir)
SYSCALL(chdir)
SYSCALL(dup)
SYSCALL(getpid)
SYSCALL(sbrk)
SYSCALL(sleep)
SYSCALL(uptime)
*/
