#include "types.h"
#include "defs.h"
#include "param.h"
#include "memlayout.h"
#include "mmu.h"
#include "proc.h"
#include "riscv.h"
#include "traps.h"
#include "spinlock.h"

// Interrupt descriptor table (shared by all CPUs).
int idt[256];
extern uint vectors[];  // in vectors.S: array of 256 entry pointers
extern void* alltraps();
struct spinlock tickslock;
uint ticks;

void
tvinit(void)
{
/*
  int i;

  for(i = 0; i < 256; i++)
//mmu.h
	//interrupt
    SETGATE(idt[i], 0, SEG_KCODE<<3, vectors[i], 0);
  //trap
  SETGATE(idt[T_SYSCALL], 1, SEG_KCODE<<3, vectors[T_SYSCALL], DPL_USER);
*/

  initlock(&tickslock, "time");
  //int i;
  //for(i = 0; i < 256; i++)
  //	idt[i] = (int)vectors[i];
  idt[0] = (int)alltraps;
  initlock(&tickslock, "time");
}

void
idtinit(void)
{
  //lidt(idt, sizeof(idt));
  ltvec(idt);
}

//PAGEBREAK: 41
void
trap(struct trapframe *tf)
{
	uint trapno = read_cause() & 0x7fffffff;
	uint interrupt = read_cause() & ~0x7fffffff;

	if(!interrupt){
		if(trapno == 8 || trapno == 11){
			if(myproc()->killed)
				exit();
			myproc()->tf = tf;
			syscall();
			if(myproc()->killed)
				exit();
			return;
		}

	}else{
		switch(trapno){
			//TIMER
			case 4:
			case 7:
				if(cpuid() == 0){
					acquire(&tickslock);
					ticks++;
					wakeup(&ticks);
					release(&tickslock);
				}
				//lapiceoi();
				eoi(TIMER);
				break;
				/*
				   case T_IRQ0 + IRQ_IDE:
				   ideintr();
				   lapiceoi();
				   break;
				   case T_IRQ0 + IRQ_IDE+1:
				// Bochs generates spurious IDE1 interrupts.
				break;
				case T_IRQ0 + IRQ_KBD:
				kbdintr();
				lapiceoi();
				break;
				*/
				//COM
			case 8:
			case 11:
				uartintr();
				//lapiceoi();
				eoi(EXTERNAL);
				break;
				/*
				   case T_IRQ0 + 7:
				   case T_IRQ0 + IRQ_SPURIOUS:
				//cprintf("cpu%d: spurious interrupt at %x:%x\n",
				//        cpuid(), tf->cs, tf->eip);
				cprintf("cpu%d: spurious interrupt at %x\n",
				cpuid(), tf->ra);
				lapiceoi();
				break;
				*/
				//PAGEBREAK: 13
			default:
				/*
				   if(myproc() == 0 || (tf->cs&3) == 0){
				// In kernel, it must be our mistake.
				cprintf("unexpected trap %d from cpu %d eip %x (cr2=0x%x)\n",
				tf->trapno, cpuid(), tf->eip, rcr2());
				cprintf("unexpected trap %d from cpu %d eip %x\n",
				tf->trapno, cpuid(), tf->ra);
				panic("trap");
				}
				// In user space, assume process misbehaved.
				cprintf("pid %d %s: trap %d err %d on cpu %d "
				"eip 0x%x addr 0x%x--kill proc\n",
				myproc()->pid, myproc()->name, tf->trapno,
				tf->err, cpuid(), tf->eip, rcr2());
				*/
				cprintf("pid %d %s: trap %d on cpu %d "
						"eip 0x%x addr --kill proc\n",
						myproc()->pid, myproc()->name, trapno,
						cpuid(), tf->ra);
				myproc()->killed = 1;
		}

	}

	// Force process exit if it has been killed and is in user space.
	// (If it is still executing in the kernel, let it keep running
	// until it gets to the regular system call return.)
	//if(myproc() && myproc()->killed && (tf->cs&3) == DPL_USER)
	if(myproc() && myproc()->killed)
		exit();

	// Force process to give up CPU on clock tick.
	// If interrupts were on while locks held, would need to check nlock.
	if(myproc() && myproc()->state == RUNNING &&
			(trapno == 4 || trapno == 7))
		yield();

	// Check if the process has been killed since we yielded
	//if(myproc() && myproc()->killed && (tf->cs&3) == DPL_USER)
	if(myproc() && myproc()->killed )
		exit();
}
