// Boot loader.
//
// Part of the boot block, along with bootasm.S, which calls bootmain().
// bootasm.S has put the processor into protected 32-bit mode.
// bootmain() loads an ELF kernel image from the disk starting at
// sector 1 and then jumps to the kernel entry routine.

#include "types.h"
#include "elf.h"
#include "riscv.h"
#include "memlayout.h"

#define SECTSIZE  512

void readseg(uchar*, uint, uint);

void
bootmain(void)
{
  struct elfhdr *elf;
  struct proghdr *ph, *eph;
  void (*entry)(void);
  uchar* pa;

  elf = (struct elfhdr*)0x10000;  // scratch space

  // Read 1st page off disk
  readseg((uchar*)elf, 4096, 0);

  // Is this an ELF executable?
  if(elf->magic != ELF_MAGIC)
    return;  // let bootasm.S handle error
  // Load each program segment (ignores ph flags).
  ph = (struct proghdr*)((uchar*)elf + elf->phoff);
  eph = ph + elf->phnum;
  for(; ph < eph; ph++){
    pa = (uchar*)ph->paddr;
    readseg(pa, ph->filesz, ph->off);
    if(ph->memsz > ph->filesz)
      stosb(pa + ph->filesz, 0, ph->memsz - ph->filesz);
  }
  // Call the entry point from the ELF header.
  // Does not return!

  entry = (void(*)(void))(elf->entry);
  //int ent = *((int*)elf->entry);
  //entry = (void(*)(void))(ent);
  entry();
}

void
waitdisk(void)
{
  // Wait for disk ready.
  while((inb(0x1F7) & 0xC0) != 0x40)
    ;
}

// Read a single sector at offset into dst.
void
readsect(void *dst, uint offset)
{
  // Issue command.
  //waitdisk();
  outb(0x1F2, 1);   // count = 1
  
  outb(0x1F3, offset);
  outb(0x1F4, offset >> 8);
  outb(0x1F5, offset >> 16);
  //outb(0x1F6, (offset >> 24) | 0xE0);
  outb(0x1F6, (offset >> 24));
  
  //*(int*)(RISCV_IO_BASE + 0x1F3) = offset;
  outb(0x1F7, 0x20);  // cmd 0x20 - read sectors

  // Read data.
  waitdisk();
  //insl(0x1F0, dst, SECTSIZE/4);
  insl(0x1F0, dst, SECTSIZE);
}

// Read 'count' bytes at 'offset' from kernel into physical address 'pa'.
// Might copy more than asked.
void
readseg(uchar* pa, uint count, uint offset)
{
//0x1E3:	read op
//0x1E4 - 0x1E7:	phisical address
//0x1E8 - 0x1EB:	read address(offset)
//0x1EC - 0x1EF:	read size(count)


  uchar* epa;

  epa = pa + count;

  // Round down to sector boundary.
  pa -= offset % SECTSIZE;

  // Translate from bytes to sectors; kernel starts at sector 1.
  offset = (offset / SECTSIZE) + 1;
/*
	int* addr = (int*)(0x1E4 + RISCV_IO_BASE);
	*addr = (int)pa;
	addr++;
	*addr = offset;
	addr++;
	*addr = count;

	char* read = (char*)(0x1E3 + RISCV_IO_BASE);
	*read = 1;
*/
  // If this is too slow, we could read lots of sectors at a time.
  // We'd write more to memory than asked, but it doesn't matter --
  // we load in increasing order.
  for(; pa < epa; pa += SECTSIZE, offset++)
    readsect(pa, offset);
}
