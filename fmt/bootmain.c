9500 // Boot loader.
9501 //
9502 // Part of the boot block, along with bootasm.S, which calls bootmain().
9503 // bootasm.S has put the processor into protected 32-bit mode.
9504 // bootmain() loads an ELF kernel image from the disk starting at
9505 // sector 1 and then jumps to the kernel entry routine.
9506 
9507 #include "types.h"
9508 #include "elf.h"
9509 #include "riscv.h"
9510 #include "memlayout.h"
9511 
9512 #define SECTSIZE  512
9513 
9514 void readseg(uchar*, uint, uint);
9515 
9516 void
9517 bootmain(void)
9518 {
9519   struct elfhdr *elf;
9520   struct proghdr *ph, *eph;
9521   void (*entry)(void);
9522   uchar* pa;
9523 
9524   elf = (struct elfhdr*)0x10000;  // scratch space
9525 
9526   // Read 1st page off disk
9527   readseg((uchar*)elf, 4096, 0);
9528 
9529   // Is this an ELF executable?
9530   if(elf->magic != ELF_MAGIC)
9531     return;  // let bootasm.S handle error
9532   // Load each program segment (ignores ph flags).
9533   ph = (struct proghdr*)((uchar*)elf + elf->phoff);
9534   eph = ph + elf->phnum;
9535   for(; ph < eph; ph++){
9536     pa = (uchar*)ph->paddr;
9537     readseg(pa, ph->filesz, ph->off);
9538     if(ph->memsz > ph->filesz)
9539       stosb(pa + ph->filesz, 0, ph->memsz - ph->filesz);
9540   }
9541   // Call the entry point from the ELF header.
9542   // Does not return!
9543 
9544   entry = (void(*)(void))(elf->entry);
9545   //int ent = *((int*)elf->entry);
9546   //entry = (void(*)(void))(ent);
9547   entry();
9548 }
9549 
9550 void
9551 waitdisk(void)
9552 {
9553   // Wait for disk ready.
9554   while((inb(0x1F7) & 0xC0) != 0x40)
9555     ;
9556 }
9557 
9558 // Read a single sector at offset into dst.
9559 void
9560 readsect(void *dst, uint offset)
9561 {
9562   // Issue command.
9563   //waitdisk();
9564   outb(0x1F2, 1);   // count = 1
9565 
9566   outb(0x1F3, offset);
9567   outb(0x1F4, offset >> 8);
9568   outb(0x1F5, offset >> 16);
9569   //outb(0x1F6, (offset >> 24) | 0xE0);
9570   outb(0x1F6, (offset >> 24));
9571 
9572   //*(int*)(RISCV_IO_BASE + 0x1F3) = offset;
9573   outb(0x1F7, 0x20);  // cmd 0x20 - read sectors
9574 
9575   // Read data.
9576   waitdisk();
9577   //insl(0x1F0, dst, SECTSIZE/4);
9578   insl(0x1F0, dst, SECTSIZE);
9579 }
9580 
9581 // Read 'count' bytes at 'offset' from kernel into physical address 'pa'.
9582 // Might copy more than asked.
9583 void
9584 readseg(uchar* pa, uint count, uint offset)
9585 {
9586 //0x1E3:	read op
9587 //0x1E4 - 0x1E7:	phisical address
9588 //0x1E8 - 0x1EB:	read address(offset)
9589 //0x1EC - 0x1EF:	read size(count)
9590 
9591 
9592   uchar* epa;
9593 
9594   epa = pa + count;
9595 
9596   // Round down to sector boundary.
9597   pa -= offset % SECTSIZE;
9598 
9599 
9600   // Translate from bytes to sectors; kernel starts at sector 1.
9601   offset = (offset / SECTSIZE) + 1;
9602 /*
9603 	int* addr = (int*)(0x1E4 + RISCV_IO_BASE);
9604 	*addr = (int)pa;
9605 	addr++;
9606 	*addr = offset;
9607 	addr++;
9608 	*addr = count;
9609 
9610 	char* read = (char*)(0x1E3 + RISCV_IO_BASE);
9611 	*read = 1;
9612 */
9613   // If this is too slow, we could read lots of sectors at a time.
9614   // We'd write more to memory than asked, but it doesn't matter --
9615   // we load in increasing order.
9616   for(; pa < epa; pa += SECTSIZE, offset++)
9617     readsect(pa, offset);
9618 }
9619 
9620 
9621 
9622 
9623 
9624 
9625 
9626 
9627 
9628 
9629 
9630 
9631 
9632 
9633 
9634 
9635 
9636 
9637 
9638 
9639 
9640 
9641 
9642 
9643 
9644 
9645 
9646 
9647 
9648 
9649 
