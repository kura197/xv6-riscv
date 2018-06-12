4650 // Buffer cache.
4651 //
4652 // The buffer cache is a linked list of buf structures holding
4653 // cached copies of disk block contents.  Caching disk blocks
4654 // in memory reduces the number of disk reads and also provides
4655 // a synchronization point for disk blocks used by multiple processes.
4656 //
4657 // Interface:
4658 // * To get a buffer for a particular disk block, call bread.
4659 // * After changing buffer data, call bwrite to write it to disk.
4660 // * When done with the buffer, call brelse.
4661 // * Do not use the buffer after calling brelse.
4662 // * Only one process at a time can use a buffer,
4663 //     so do not keep them longer than necessary.
4664 //
4665 // The implementation uses two state flags internally:
4666 // * B_VALID: the buffer data has been read from the disk.
4667 // * B_DIRTY: the buffer data has been modified
4668 //     and needs to be written to disk.
4669 
4670 #include "types.h"
4671 #include "defs.h"
4672 #include "param.h"
4673 #include "spinlock.h"
4674 #include "sleeplock.h"
4675 #include "fs.h"
4676 #include "buf.h"
4677 
4678 struct {
4679   struct spinlock lock;
4680   struct buf buf[NBUF];
4681 
4682   // Linked list of all buffers, through prev/next.
4683   // head.next is most recently used.
4684   struct buf head;
4685 } bcache;
4686 
4687 void
4688 binit(void)
4689 {
4690   struct buf *b;
4691 
4692   initlock(&bcache.lock, "bcache");
4693 
4694 
4695 
4696 
4697 
4698 
4699 
4700   // Create linked list of buffers
4701   bcache.head.prev = &bcache.head;
4702   bcache.head.next = &bcache.head;
4703   for(b = bcache.buf; b < bcache.buf+NBUF; b++){
4704     b->next = bcache.head.next;
4705     b->prev = &bcache.head;
4706     initsleeplock(&b->lock, "buffer");
4707     bcache.head.next->prev = b;
4708     bcache.head.next = b;
4709   }
4710 }
4711 
4712 // Look through buffer cache for block on device dev.
4713 // If not found, allocate a buffer.
4714 // In either case, return locked buffer.
4715 static struct buf*
4716 bget(uint dev, uint blockno)
4717 {
4718   struct buf *b;
4719 
4720   acquire(&bcache.lock);
4721 
4722   // Is the block already cached?
4723   for(b = bcache.head.next; b != &bcache.head; b = b->next){
4724     if(b->dev == dev && b->blockno == blockno){
4725       b->refcnt++;
4726       release(&bcache.lock);
4727       acquiresleep(&b->lock);
4728       return b;
4729     }
4730   }
4731 
4732   // Not cached; recycle an unused buffer.
4733   // Even if refcnt==0, B_DIRTY indicates a buffer is in use
4734   // because log.c has modified it but not yet committed it.
4735   for(b = bcache.head.prev; b != &bcache.head; b = b->prev){
4736     if(b->refcnt == 0 && (b->flags & B_DIRTY) == 0) {
4737       b->dev = dev;
4738       b->blockno = blockno;
4739       b->flags = 0;
4740       b->refcnt = 1;
4741       release(&bcache.lock);
4742       acquiresleep(&b->lock);
4743       return b;
4744     }
4745   }
4746   panic("bget: no buffers");
4747 }
4748 
4749 
4750 // Return a locked buf with the contents of the indicated block.
4751 struct buf*
4752 bread(uint dev, uint blockno)
4753 {
4754   struct buf *b;
4755 
4756   b = bget(dev, blockno);
4757   if((b->flags & B_VALID) == 0) {
4758     iderw(b);
4759   }
4760   return b;
4761 }
4762 
4763 // Write b's contents to disk.  Must be locked.
4764 void
4765 bwrite(struct buf *b)
4766 {
4767   if(!holdingsleep(&b->lock))
4768     panic("bwrite");
4769   b->flags |= B_DIRTY;
4770   iderw(b);
4771 }
4772 
4773 // Release a locked buffer.
4774 // Move to the head of the MRU list.
4775 void
4776 brelse(struct buf *b)
4777 {
4778   if(!holdingsleep(&b->lock))
4779     panic("brelse");
4780 
4781   releasesleep(&b->lock);
4782 
4783   acquire(&bcache.lock);
4784   b->refcnt--;
4785   if (b->refcnt == 0) {
4786     // no one is waiting for it.
4787     b->next->prev = b->prev;
4788     b->prev->next = b->next;
4789     b->next = bcache.head.next;
4790     b->prev = &bcache.head;
4791     bcache.head.next->prev = b;
4792     bcache.head.next = b;
4793   }
4794 
4795   release(&bcache.lock);
4796 }
4797 
4798 
4799 
4800 // Blank page.
4801 
4802 
4803 
4804 
4805 
4806 
4807 
4808 
4809 
4810 
4811 
4812 
4813 
4814 
4815 
4816 
4817 
4818 
4819 
4820 
4821 
4822 
4823 
4824 
4825 
4826 
4827 
4828 
4829 
4830 
4831 
4832 
4833 
4834 
4835 
4836 
4837 
4838 
4839 
4840 
4841 
4842 
4843 
4844 
4845 
4846 
4847 
4848 
4849 
