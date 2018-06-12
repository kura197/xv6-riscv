#include "types.h"
#include "defs.h"
#include "param.h"
#include "memlayout.h"
#include "mmu.h"
#include "proc.h"
#include "riscv.h"

//static void startothers(void);
static void mpmain(void)  __attribute__((noreturn));
extern pde_t *kpgdir;
extern char end[]; // first address after kernel loaded from ELF file

// Bootstrap processor starts running C code here.
// Allocate a real stack and switch to it, first
// doing some setup required for memory allocator to work.
int
main(void)
{
//kalloc.c:4MB分を割り当て?
  kinit1(end, P2V(4*1024*1024)); // phys page allocator
//vm.c:kmapをcr3へ
  kvmalloc();      // kernel page table
//mp.c: ?? skip
  //mpinit();        // detect other processors
//lapic.c:割り込み　各種初期化
  lapicinit();     // interrupt controller
//vm.c:セグメント初期化
 // seginit();       // segment descriptors
//picirq.c:disable interrupt
  //picinit();       // disable pic
//ioapic.c:disable? interrupt
  //ioapicinit();    // another interrupt controller
//console.c: enable console functions & interrupt
  consoleinit();   // console hardware
//uart.c:uart setup, enable interrupt
  uartinit();      // serial port
//proc.c:pnit() -> spinlock:initlock():
//set up ptable
  pinit();         // process table
//make interrupt & trap vectors 
  tvinit();        // trap vectors
//ide cache 
  binit();         // buffer cache
//set up ftable
  fileinit();      // file table
//ide.c skip
  ideinit();       // disk 
//other cpu
  //startothers();   // start other processors
//kalloc.c: PHYSTOPまでのメモリ割り当て
  kinit2(P2V(4*1024*1024), P2V(PHYSTOP)); // must come after startothers()
//main.c: initcode.Sをva 0番地にセットしたプロセスを用意?
  userinit();      // first user process
//main.c: 
  mpmain();        // finish this processor's setup
}

// Other CPUs jump here from entryother.S.
// 必要なし??
/*
static void
mpenter(void)
{
  switchkvm();
  seginit();
  lapicinit();
  mpmain();
}
*/

// Common CPU setup code.
static void
mpmain(void)
{
  cprintf("cpu%d: starting %d\n", cpuid(), cpuid());
  //lidt
  idtinit();       // load idt register
  xchg(&(mycpu()->started), 1); // tell startothers() we're up
  //proc.c
  scheduler();     // start running processes
}

pde_t entrypgdir[];  // For entry.S

// Start the non-boot (AP) processors.
// 必要なし??
/*
static void
startothers(void)
{
  extern uchar _binary_entryother_start[], _binary_entryother_size[];
  uchar *code;
  struct cpu *c;
  char *stack;

  // Write entry code to unused memory at 0x7000.
  // The linker has placed the image of entryother.S in
  // _binary_entryother_start.
  code = P2V(0x7000);
  memmove(code, _binary_entryother_start, (uint)_binary_entryother_size);

  for(c = cpus; c < cpus+ncpu; c++){
    if(c == mycpu())  // We've started already.
      continue;

    // Tell entryother.S what stack to use, where to enter, and what
    // pgdir to use. We cannot use kpgdir yet, because the AP processor
    // is running in low  memory, so we use entrypgdir for the APs too.
    stack = kalloc();
    *(void**)(code-4) = stack + KSTACKSIZE;
    *(void**)(code-8) = mpenter;
    *(int**)(code-12) = (void *) V2P(entrypgdir);

    lapicstartap(c->apicid, V2P(code));

    // wait for cpu to finish mpmain()
    while(c->started == 0)
      ;
  }
}
*/

// The boot page table used in entry.S and entryother.S.
// Page directories (and page tables) must start on page boundaries,
// hence the __aligned__ attribute.
// PTE_PS in a page directory entry enables 4Mbyte pages.

__attribute__((__aligned__(PGSIZE)))
pde_t entrypgdir[NPDENTRIES] = {
  // Map VA's [0, 4MB) to PA's [0, 4MB)
 // [0] = (0) | PTE_V | PTE_W | PTE_PS,
  [0] = (0) | PTE_V | PTE_R | PTE_X | PTE_W,
  // Map VA's [KERNBASE, KERNBASE+4MB) to PA's [0, 4MB)
  //[KERNBASE>>PDXSHIFT] = (0) | PTE_V | PTE_W | PTE_PS,
  [KERNBASE>>PDXSHIFT] = (0) | PTE_V | PTE_R | PTE_X | PTE_W,
};

//PAGEBREAK!
// Blank page.
//PAGEBREAK!
// Blank page.
//PAGEBREAK!
// Blank page.

