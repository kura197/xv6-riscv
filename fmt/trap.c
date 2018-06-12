3550 #include "types.h"
3551 #include "defs.h"
3552 #include "param.h"
3553 #include "memlayout.h"
3554 #include "mmu.h"
3555 #include "proc.h"
3556 #include "riscv.h"
3557 #include "traps.h"
3558 #include "spinlock.h"
3559 
3560 // Interrupt descriptor table (shared by all CPUs).
3561 int idt[256];
3562 extern uint vectors[];  // in vectors.S: array of 256 entry pointers
3563 extern void* alltraps();
3564 struct spinlock tickslock;
3565 uint ticks;
3566 
3567 void
3568 tvinit(void)
3569 {
3570 /*
3571   int i;
3572 
3573   for(i = 0; i < 256; i++)
3574 //mmu.h
3575 	//interrupt
3576     SETGATE(idt[i], 0, SEG_KCODE<<3, vectors[i], 0);
3577   //trap
3578   SETGATE(idt[T_SYSCALL], 1, SEG_KCODE<<3, vectors[T_SYSCALL], DPL_USER);
3579 */
3580 
3581   initlock(&tickslock, "time");
3582   //int i;
3583   //for(i = 0; i < 256; i++)
3584   //	idt[i] = (int)vectors[i];
3585   idt[0] = (int)alltraps;
3586   initlock(&tickslock, "time");
3587 }
3588 
3589 void
3590 idtinit(void)
3591 {
3592   //lidt(idt, sizeof(idt));
3593   ltvec(idt);
3594 }
3595 
3596 
3597 
3598 
3599 
3600 void
3601 trap(struct trapframe *tf)
3602 {
3603 	uint trapno = read_cause() & 0x7fffffff;
3604 	uint interrupt = read_cause() & ~0x7fffffff;
3605 
3606 	if(!interrupt){
3607 		if(trapno == 8 || trapno == 11){
3608 			if(myproc()->killed)
3609 				exit();
3610 			myproc()->tf = tf;
3611 			syscall();
3612 			if(myproc()->killed)
3613 				exit();
3614 			return;
3615 		}
3616 
3617 	}else{
3618 		switch(trapno){
3619 			//TIMER
3620 			case 4:
3621 			case 7:
3622 				if(cpuid() == 0){
3623 					acquire(&tickslock);
3624 					ticks++;
3625 					wakeup(&ticks);
3626 					release(&tickslock);
3627 				}
3628 				//lapiceoi();
3629 				eoi(TIMER);
3630 				break;
3631 				/*
3632 				   case T_IRQ0 + IRQ_IDE:
3633 				   ideintr();
3634 				   lapiceoi();
3635 				   break;
3636 				   case T_IRQ0 + IRQ_IDE+1:
3637 				// Bochs generates spurious IDE1 interrupts.
3638 				break;
3639 				case T_IRQ0 + IRQ_KBD:
3640 				kbdintr();
3641 				lapiceoi();
3642 				break;
3643 				*/
3644 				//COM
3645 			case 8:
3646 			case 11:
3647 				uartintr();
3648 				//lapiceoi();
3649 				eoi(EXTERNAL);
3650 				break;
3651 				/*
3652 				   case T_IRQ0 + 7:
3653 				   case T_IRQ0 + IRQ_SPURIOUS:
3654 				//cprintf("cpu%d: spurious interrupt at %x:%x\n",
3655 				//        cpuid(), tf->cs, tf->eip);
3656 				cprintf("cpu%d: spurious interrupt at %x\n",
3657 				cpuid(), tf->ra);
3658 				lapiceoi();
3659 				break;
3660 				*/
3661 
3662 			default:
3663 				/*
3664 				   if(myproc() == 0 || (tf->cs&3) == 0){
3665 				// In kernel, it must be our mistake.
3666 				cprintf("unexpected trap %d from cpu %d eip %x (cr2=0x%x)\n",
3667 				tf->trapno, cpuid(), tf->eip, rcr2());
3668 				cprintf("unexpected trap %d from cpu %d eip %x\n",
3669 				tf->trapno, cpuid(), tf->ra);
3670 				panic("trap");
3671 				}
3672 				// In user space, assume process misbehaved.
3673 				cprintf("pid %d %s: trap %d err %d on cpu %d "
3674 				"eip 0x%x addr 0x%x--kill proc\n",
3675 				myproc()->pid, myproc()->name, tf->trapno,
3676 				tf->err, cpuid(), tf->eip, rcr2());
3677 				*/
3678 				cprintf("pid %d %s: trap %d on cpu %d "
3679 						"eip 0x%x addr --kill proc\n",
3680 						myproc()->pid, myproc()->name, trapno,
3681 						cpuid(), tf->ra);
3682 				myproc()->killed = 1;
3683 		}
3684 
3685 	}
3686 
3687 	// Force process exit if it has been killed and is in user space.
3688 	// (If it is still executing in the kernel, let it keep running
3689 	// until it gets to the regular system call return.)
3690 	//if(myproc() && myproc()->killed && (tf->cs&3) == DPL_USER)
3691 	if(myproc() && myproc()->killed)
3692 		exit();
3693 
3694 	// Force process to give up CPU on clock tick.
3695 	// If interrupts were on while locks held, would need to check nlock.
3696 	if(myproc() && myproc()->state == RUNNING &&
3697 			(trapno == 4 || trapno == 7))
3698 		yield();
3699 
3700 	// Check if the process has been killed since we yielded
3701 	//if(myproc() && myproc()->killed && (tf->cs&3) == DPL_USER)
3702 	if(myproc() && myproc()->killed )
3703 		exit();
3704 }
3705 
3706 
3707 
3708 
3709 
3710 
3711 
3712 
3713 
3714 
3715 
3716 
3717 
3718 
3719 
3720 
3721 
3722 
3723 
3724 
3725 
3726 
3727 
3728 
3729 
3730 
3731 
3732 
3733 
3734 
3735 
3736 
3737 
3738 
3739 
3740 
3741 
3742 
3743 
3744 
3745 
3746 
3747 
3748 
3749 
