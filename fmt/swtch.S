2950 # Context switch
2951 #
2952 #   void swtch(struct context **old, struct context *new);
2953 #
2954 # Save the current registers on the stack, creating
2955 # a struct context, and save its address in *old.
2956 # Switch stacks to new and pop previously-saved registers.
2957 
2958 .globl swtch
2959 swtch:
2960  # movl 4(sp), %eax
2961  # movl 8(sp), %edx
2962 
2963  # # Save old callee-save registers
2964  # pushl %ebp
2965  # pushl %ebx
2966  # pushl %esi
2967  # pushl %edi
2968 
2969  # # Switch stacks
2970  # movl %esp, (%eax)
2971  # movl %edx, %esp
2972 
2973  # # Load new callee-save registers
2974  # popl %edi
2975  # popl %esi
2976  # popl %ebx
2977  # popl %ebp
2978  # ret
2979 
2980   addi sp, sp, -56
2981 
2982   # Save old callee-save registers and the link register
2983  # sw a1, 56(sp)
2984   sw a0, 52(sp)
2985   sw ra, 48(sp)
2986   sw s11, 44(sp)
2987   sw s10, 40(sp)
2988   sw s9, 36(sp)
2989   sw s8, 32(sp)
2990   sw s7, 28(sp)
2991   sw s6, 24(sp)
2992   sw s5, 20(sp)
2993   sw s4, 16(sp)
2994   sw s3, 12(sp)
2995   sw s2, 8(sp)
2996   sw s1, 4(sp)
2997   sw s0, 0(sp)
2998 
2999 
3000   # Switch stacks
3001   sw sp, 0(a0)
3002   mv sp, a1
3003 
3004   # Load new callee-save registers
3005   lw ra, 52(sp)
3006   lw t0, 48(sp)
3007   lw s11, 44(sp)
3008   lw s10, 40(sp)
3009   lw s9, 36(sp)
3010   lw s8, 32(sp)
3011   lw s7, 28(sp)
3012   lw s6, 24(sp)
3013   lw s5, 20(sp)
3014   lw s4, 16(sp)
3015   lw s3, 12(sp)
3016   lw s2, 8(sp)
3017   lw s1, 4(sp)
3018   lw s0, 0(sp)
3019   addi sp, sp, 56
3020   jr t0
3021   nop
3022 /*
3023   lw ra, 52(sp)
3024   lw t0, 48(sp)
3025   lw s11, 44(sp)
3026   lw s10, 40(sp)
3027   lw s9, 36(sp)
3028   lw s8, 32(sp)
3029   lw s7, 28(sp)
3030   lw s6, 24(sp)
3031   lw s5, 20(sp)
3032   lw s4, 16(sp)
3033   lw s3, 12(sp)
3034   lw s2, 8(sp)
3035   lw s1, 4(sp)
3036   lw s0, 0(sp)
3037   addi sp, sp, 56
3038   jr t0
3039   nop
3040 */
3041 
3042 
3043 
3044 
3045 
3046 
3047 
3048 
3049 
