7450 // Multiprocessor support
7451 // Search memory for MP description structures.
7452 // http://developer.intel.com/design/pentium/datashts/24201606.pdf
7453 #include "types.h"
7454 #include "defs.h"
7455 #include "param.h"
7456 #include "memlayout.h"
7457 #include "mp.h"
7458 #include "mmu.h"
7459 #include "proc.h"
7460 #include "riscv.h"
7461 
7462 struct cpu cpus[NCPU];
7463 int ncpu;
7464 uchar ioapicid;
7465 
7466 static uchar
7467 sum(uchar *addr, int len)
7468 {
7469   int i, sum;
7470 
7471   sum = 0;
7472   for(i=0; i<len; i++)
7473     sum += addr[i];
7474   return sum;
7475 }
7476 
7477 // Look for an MP structure in the len bytes at addr.
7478 static struct mp*
7479 mpsearch1(uint a, int len)
7480 {
7481   uchar *e, *p, *addr;
7482 
7483   addr = P2V(a);
7484   e = addr+len;
7485   for(p = addr; p < e; p += sizeof(struct mp))
7486     if(memcmp(p, "_MP_", 4) == 0 && sum(p, sizeof(struct mp)) == 0)
7487       return (struct mp*)p;
7488   return 0;
7489 }
7490 
7491 
7492 
7493 
7494 
7495 
7496 
7497 
7498 
7499 
7500 // Search for the MP Floating Pointer Structure, which according to the
7501 // spec is in one of the following three locations:
7502 // 1) in the first KB of the EBDA;
7503 // 2) in the last KB of system base memory;
7504 // 3) in the BIOS ROM between 0xE0000 and 0xFFFFF.
7505 static struct mp*
7506 mpsearch(void)
7507 {
7508   uchar *bda;
7509   uint p;
7510   struct mp *mp;
7511 
7512   bda = (uchar *) P2V(0x400);
7513   if((p = ((bda[0x0F]<<8)| bda[0x0E]) << 4)){
7514     if((mp = mpsearch1(p, 1024)))
7515       return mp;
7516   } else {
7517     p = ((bda[0x14]<<8)|bda[0x13])*1024;
7518     if((mp = mpsearch1(p-1024, 1024)))
7519       return mp;
7520   }
7521   return mpsearch1(0xF0000, 0x10000);
7522 }
7523 
7524 // Search for an MP configuration table.  For now,
7525 // don't accept the default configurations (physaddr == 0).
7526 // Check for correct signature, calculate the checksum and,
7527 // if correct, check the version.
7528 // To do: check extended table checksum.
7529 static struct mpconf*
7530 mpconfig(struct mp **pmp)
7531 {
7532   struct mpconf *conf;
7533   struct mp *mp;
7534 
7535   if((mp = mpsearch()) == 0 || mp->physaddr == 0)
7536     return 0;
7537   conf = (struct mpconf*) P2V((uint) mp->physaddr);
7538   if(memcmp(conf, "PCMP", 4) != 0)
7539     return 0;
7540   if(conf->version != 1 && conf->version != 4)
7541     return 0;
7542   if(sum((uchar*)conf, conf->length) != 0)
7543     return 0;
7544   *pmp = mp;
7545   return conf;
7546 }
7547 
7548 
7549 
7550 void
7551 mpinit(void)
7552 {
7553   uchar *p, *e;
7554   int ismp;
7555   struct mp *mp;
7556   struct mpconf *conf;
7557   struct mpproc *proc;
7558   struct mpioapic *ioapic;
7559 
7560   if((conf = mpconfig(&mp)) == 0)
7561     panic("Expect to run on an SMP");
7562   ismp = 1;
7563   lapic = (uint*)conf->lapicaddr;
7564   for(p=(uchar*)(conf+1), e=(uchar*)conf+conf->length; p<e; ){
7565     switch(*p){
7566     case MPPROC:
7567       proc = (struct mpproc*)p;
7568       if(ncpu < NCPU) {
7569         cpus[ncpu].apicid = proc->apicid;  // apicid may differ from ncpu
7570         ncpu++;
7571       }
7572       p += sizeof(struct mpproc);
7573       continue;
7574     case MPIOAPIC:
7575       ioapic = (struct mpioapic*)p;
7576       ioapicid = ioapic->apicno;
7577       p += sizeof(struct mpioapic);
7578       continue;
7579     case MPBUS:
7580     case MPIOINTR:
7581     case MPLINTR:
7582       p += 8;
7583       continue;
7584     default:
7585       ismp = 0;
7586       break;
7587     }
7588   }
7589   if(!ismp)
7590     panic("Didn't find a suitable machine");
7591 
7592   if(mp->imcrp){
7593     // Bochs doesn't support IMCR, so this doesn't run on Bochs.
7594     // But it would on real hardware.
7595     outb(0x22, 0x70);   // Select IMCR
7596     outb(0x23, inb(0x23) | 1);  // Mask external interrupts.
7597   }
7598 
7599 }
