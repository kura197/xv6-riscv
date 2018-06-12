4450 // Simple PIO-based (non-DMA) IDE driver code.
4451 
4452 #include "types.h"
4453 #include "defs.h"
4454 #include "param.h"
4455 #include "memlayout.h"
4456 #include "mmu.h"
4457 #include "proc.h"
4458 #include "riscv.h"
4459 #include "traps.h"
4460 #include "spinlock.h"
4461 #include "sleeplock.h"
4462 #include "fs.h"
4463 #include "buf.h"
4464 
4465 #define SECTOR_SIZE   512
4466 #define IDE_BSY       0x80
4467 #define IDE_DRDY      0x40
4468 #define IDE_DF        0x20
4469 #define IDE_ERR       0x01
4470 
4471 #define IDE_CMD_READ  0x20
4472 #define IDE_CMD_WRITE 0x30
4473 #define IDE_CMD_RDMUL 0xc4
4474 #define IDE_CMD_WRMUL 0xc5
4475 
4476 // idequeue points to the buf now being read/written to the disk.
4477 // idequeue->qnext points to the next buf to be processed.
4478 // You must hold idelock while manipulating queue.
4479 
4480 static struct spinlock idelock;
4481 static struct buf *idequeue;
4482 
4483 static int havedisk1;
4484 static void idestart(struct buf*);
4485 
4486 // Wait for IDE disk to become ready.
4487 static int
4488 idewait(int checkerr)
4489 {
4490   int r;
4491 
4492   while(((r = inb(0x1f7)) & (IDE_BSY|IDE_DRDY)) != IDE_DRDY)
4493     ;
4494   if(checkerr && (r & (IDE_DF|IDE_ERR)) != 0)
4495     return -1;
4496   return 0;
4497 }
4498 
4499 
4500 void
4501 ideinit(void)
4502 {
4503   int i;
4504 
4505   initlock(&idelock, "ide");
4506   ioapicenable(IRQ_IDE, ncpu - 1);
4507   idewait(0);
4508 
4509   // Check if disk 1 is present
4510   outb(0x1f6, 0xe0 | (1<<4));
4511   for(i=0; i<1000; i++){
4512     if(inb(0x1f7) != 0){
4513       havedisk1 = 1;
4514       break;
4515     }
4516   }
4517 
4518   // Switch back to disk 0.
4519   outb(0x1f6, 0xe0 | (0<<4));
4520 }
4521 
4522 // Start the request for b.  Caller must hold idelock.
4523 static void
4524 idestart(struct buf *b)
4525 {
4526   if(b == 0)
4527     panic("idestart");
4528   if(b->blockno >= FSSIZE)
4529     panic("incorrect blockno");
4530   int sector_per_block =  BSIZE/SECTOR_SIZE;
4531   int sector = b->blockno * sector_per_block;
4532   int read_cmd = (sector_per_block == 1) ? IDE_CMD_READ :  IDE_CMD_RDMUL;
4533   int write_cmd = (sector_per_block == 1) ? IDE_CMD_WRITE : IDE_CMD_WRMUL;
4534 
4535   if (sector_per_block > 7) panic("idestart");
4536 
4537   idewait(0);
4538   outb(0x3f6, 0);  // generate interrupt
4539   outb(0x1f2, sector_per_block);  // number of sectors
4540   outb(0x1f3, sector & 0xff);
4541   outb(0x1f4, (sector >> 8) & 0xff);
4542   outb(0x1f5, (sector >> 16) & 0xff);
4543   outb(0x1f6, 0xe0 | ((b->dev&1)<<4) | ((sector>>24)&0x0f));
4544   if(b->flags & B_DIRTY){
4545     outb(0x1f7, write_cmd);
4546     outsl(0x1f0, b->data, BSIZE/4);
4547   } else {
4548     outb(0x1f7, read_cmd);
4549   }
4550 }
4551 
4552 // Interrupt handler.
4553 void
4554 ideintr(void)
4555 {
4556   struct buf *b;
4557 
4558   // First queued buffer is the active request.
4559   acquire(&idelock);
4560 
4561   if((b = idequeue) == 0){
4562     release(&idelock);
4563     return;
4564   }
4565   idequeue = b->qnext;
4566 
4567   // Read data if needed.
4568   if(!(b->flags & B_DIRTY) && idewait(1) >= 0)
4569     insl(0x1f0, b->data, BSIZE/4);
4570 
4571   // Wake process waiting for this buf.
4572   b->flags |= B_VALID;
4573   b->flags &= ~B_DIRTY;
4574   wakeup(b);
4575 
4576   // Start disk on next buf in queue.
4577   if(idequeue != 0)
4578     idestart(idequeue);
4579 
4580   release(&idelock);
4581 }
4582 
4583 
4584 
4585 
4586 
4587 
4588 
4589 
4590 
4591 
4592 
4593 
4594 
4595 
4596 
4597 
4598 
4599 
4600 // Sync buf with disk.
4601 // If B_DIRTY is set, write buf to disk, clear B_DIRTY, set B_VALID.
4602 // Else if B_VALID is not set, read buf from disk, set B_VALID.
4603 void
4604 iderw(struct buf *b)
4605 {
4606   struct buf **pp;
4607 
4608   if(!holdingsleep(&b->lock))
4609     panic("iderw: buf not locked");
4610   if((b->flags & (B_VALID|B_DIRTY)) == B_VALID)
4611     panic("iderw: nothing to do");
4612   if(b->dev != 0 && !havedisk1)
4613     panic("iderw: ide disk 1 not present");
4614 
4615   acquire(&idelock);  //DOC:acquire-lock
4616 
4617   // Append b to idequeue.
4618   b->qnext = 0;
4619   for(pp=&idequeue; *pp; pp=&(*pp)->qnext)  //DOC:insert-queue
4620     ;
4621   *pp = b;
4622 
4623   // Start disk if necessary.
4624   if(idequeue == b)
4625     idestart(b);
4626 
4627   // Wait for request to finish.
4628   while((b->flags & (B_VALID|B_DIRTY)) != B_VALID){
4629     sleep(b, &idelock);
4630   }
4631 
4632 
4633   release(&idelock);
4634 }
4635 
4636 
4637 
4638 
4639 
4640 
4641 
4642 
4643 
4644 
4645 
4646 
4647 
4648 
4649 
