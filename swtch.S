# Context switch
#
#   void swtch(struct context **old, struct context *new);
# 
# Save the current registers on the stack, creating
# a struct context, and save its address in *old.
# Switch stacks to new and pop previously-saved registers.

.globl swtch
swtch:
 # movl 4(sp), %eax
 # movl 8(sp), %edx

 # # Save old callee-save registers
 # pushl %ebp
 # pushl %ebx
 # pushl %esi
 # pushl %edi

 # # Switch stacks
 # movl %esp, (%eax)
 # movl %edx, %esp

 # # Load new callee-save registers
 # popl %edi
 # popl %esi
 # popl %ebx
 # popl %ebp
 # ret

  addi sp, sp, -56

  # Save old callee-save registers and the link register
 # sw a1, 56(sp)
  sw a0, 52(sp)
  sw ra, 48(sp)
  sw s11, 44(sp)
  sw s10, 40(sp)
  sw s9, 36(sp)
  sw s8, 32(sp)
  sw s7, 28(sp)
  sw s6, 24(sp)
  sw s5, 20(sp)
  sw s4, 16(sp)
  sw s3, 12(sp)
  sw s2, 8(sp)
  sw s1, 4(sp)
  sw s0, 0(sp)

  # Switch stacks
  sw sp, 0(a0)
  mv sp, a1

  # Load new callee-save registers 
  lw ra, 52(sp)
  lw t0, 48(sp)
  lw s11, 44(sp)
  lw s10, 40(sp)
  lw s9, 36(sp)
  lw s8, 32(sp)
  lw s7, 28(sp)
  lw s6, 24(sp)
  lw s5, 20(sp)
  lw s4, 16(sp)
  lw s3, 12(sp)
  lw s2, 8(sp)
  lw s1, 4(sp)
  lw s0, 0(sp)
  addi sp, sp, 56
  jr t0
  nop
/*
  lw ra, 52(sp)
  lw t0, 48(sp)
  lw s11, 44(sp)
  lw s10, 40(sp)
  lw s9, 36(sp)
  lw s8, 32(sp)
  lw s7, 28(sp)
  lw s6, 24(sp)
  lw s5, 20(sp)
  lw s4, 16(sp)
  lw s3, 12(sp)
  lw s2, 8(sp)
  lw s1, 4(sp)
  lw s0, 0(sp)
  addi sp, sp, 56
  jr t0
  nop
*/
