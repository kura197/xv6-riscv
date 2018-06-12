3350 #include "mmu.h"
3351 
3352   # vectors.S sends all traps here.
3353 #.globl alltraps
3354 #alltraps:
3355 /*
3356   # Build trap frame.
3357   pushl %ds
3358   pushl %es
3359   pushl %fs
3360   pushl %gs
3361   pushal
3362 
3363   # Set up data segments.
3364   movw $(SEG_KDATA<<3), %ax
3365   movw %ax, %ds
3366   movw %ax, %es
3367 
3368   # Call trap(tf), where tf=%esp
3369   pushl %esp
3370   call trap
3371   addl $4, %esp
3372 
3373   # Return falls through to trapret...
3374 .globl trapret
3375 trapret:
3376   popal
3377   popl %gs
3378   popl %fs
3379   popl %es
3380   popl %ds
3381   addl $0x8, %esp  # trapno and errcode
3382   iret
3383 */
3384 
3385 .globl alltraps
3386 alltraps:
3387 
3388 	sw t6,-4(sp)
3389 	sw t5,-8(sp)
3390 	sw t4,-12(sp)
3391 	mv t6,sp
3392 	csrr t5,mcause
3393 	#andi t5,t5,0x7fffffff
3394 	li t4,0x80000003
3395 	beq t5,t4,trapmain
3396 	li t4,0x80000007
3397 	beq t5,t4,trapmain
3398 	li t4,0x8000000b
3399 	beq t5,t4,trapmain
3400 	csrr sp,mscratch
3401 
3402 trapmain:
3403 	addi sp,sp,-136
3404 
3405 	sw ra,0(sp)
3406 	sw gp,4(sp)
3407 	sw tp,8(sp)
3408 	sw t0,12(sp)
3409 	sw t1,16(sp)
3410 	sw t2,20(sp)
3411 	sw s0,24(sp)
3412 	sw s1,28(sp)
3413 	sw a0,32(sp)
3414 	sw a1,36(sp)
3415 	sw a2,40(sp)
3416 	sw a3,44(sp)
3417 	sw a4,48(sp)
3418 	sw a5,52(sp)
3419 	sw a6,56(sp)
3420 	sw a7,60(sp)
3421 	sw s2,64(sp)
3422 	sw s3,68(sp)
3423 	sw s4,72(sp)
3424 	sw s5,76(sp)
3425 	sw s6,80(sp)
3426 	sw s7,84(sp)
3427 	sw s8,88(sp)
3428 	sw s9,92(sp)
3429 	sw s10,96(sp)
3430 	sw s11,100(sp)
3431 	sw t3,104(sp)
3432 
3433 	//t3 <= sp
3434 	mv t3,t6
3435 	lw t4,-12(t3)
3436 	lw t5,-8(t3)
3437 	lw t6,-4(t3)
3438 	sw t4,108(sp)
3439 	sw t5,112(sp)
3440 	sw t6,116(sp)
3441 
3442 	csrr t0,mepc
3443 	csrr t1,mcause
3444 	csrr t2,mstatus
3445 	sw t0,120(sp)
3446 	sw t1,124(sp)
3447 	sw t2,128(sp)
3448 
3449 
3450 	sw t3,132(sp)
3451 	mv a0,sp
3452 	jal trap
3453 
3454 	#addi sp,sp,4
3455 
3456 # Return falls through to trapret...
3457 .globl trapret
3458 trapret:
3459 	//addi sp,sp,-136
3460 
3461 	lw ra,0(sp)
3462 	lw gp,4(sp)
3463 	lw tp,8(sp)
3464 	lw t0,12(sp)
3465 	lw t1,16(sp)
3466 	lw t2,20(sp)
3467 	lw s0,24(sp)
3468 	lw s1,28(sp)
3469 	lw a0,32(sp)
3470 	lw a1,36(sp)
3471 	lw a2,40(sp)
3472 	lw a3,44(sp)
3473 	lw a4,48(sp)
3474 	lw a5,52(sp)
3475 	lw a6,56(sp)
3476 	lw a7,60(sp)
3477 	lw s2,64(sp)
3478 	lw s3,68(sp)
3479 	lw s4,72(sp)
3480 	lw s5,76(sp)
3481 	lw s6,80(sp)
3482 	lw s7,84(sp)
3483 	lw s8,88(sp)
3484 	lw s9,92(sp)
3485 	lw s10,96(sp)
3486 	lw s11,100(sp)
3487 
3488 	lw t3,120(sp)
3489 	lw t4,124(sp)
3490 	lw t5,128(sp)
3491 	csrw mepc,t3
3492 	csrw mcause,t4
3493 	csrw mstatus,t5
3494 
3495 	lw t3,104(sp)
3496 	lw t4,108(sp)
3497 	lw t5,112(sp)
3498 	lw t6,116(sp)
3499 
3500 	addi sp,sp,132
3501 	#addi sp,sp,4  # trapno #and errcode
3502 	lw sp,0(sp)
3503 	mret
3504 
3505 
3506 
3507 
3508 
3509 
3510 
3511 
3512 
3513 
3514 
3515 
3516 
3517 
3518 
3519 
3520 
3521 
3522 
3523 
3524 
3525 
3526 
3527 
3528 
3529 
3530 
3531 
3532 
3533 
3534 
3535 
3536 
3537 
3538 
3539 
3540 
3541 
3542 
3543 
3544 
3545 
3546 
3547 
3548 
3549 
