5200 // File system implementation.  Five layers:
5201 //   + Blocks: allocator for raw disk blocks.
5202 //   + Log: crash recovery for multi-step updates.
5203 //   + Files: inode allocator, reading, writing, metadata.
5204 //   + Directories: inode with special contents (list of other inodes!)
5205 //   + Names: paths like /usr/rtm/xv6/fs.c for convenient naming.
5206 //
5207 // This file contains the low-level file system manipulation
5208 // routines.  The (higher-level) system call implementations
5209 // are in sysfile.c.
5210 
5211 #include "types.h"
5212 #include "defs.h"
5213 #include "param.h"
5214 #include "stat.h"
5215 #include "mmu.h"
5216 #include "proc.h"
5217 #include "spinlock.h"
5218 #include "sleeplock.h"
5219 #include "fs.h"
5220 #include "buf.h"
5221 #include "file.h"
5222 
5223 #define min(a, b) ((a) < (b) ? (a) : (b))
5224 static void itrunc(struct inode*);
5225 // there should be one superblock per disk device, but we run with
5226 // only one device
5227 struct superblock sb;
5228 
5229 // Read the super block.
5230 void
5231 readsb(int dev, struct superblock *sb)
5232 {
5233   struct buf *bp;
5234 
5235   bp = bread(dev, 1);
5236   memmove(sb, bp->data, sizeof(*sb));
5237   brelse(bp);
5238 }
5239 
5240 
5241 
5242 
5243 
5244 
5245 
5246 
5247 
5248 
5249 
5250 // Zero a block.
5251 static void
5252 bzero(int dev, int bno)
5253 {
5254   struct buf *bp;
5255 
5256   bp = bread(dev, bno);
5257   memset(bp->data, 0, BSIZE);
5258   log_write(bp);
5259   brelse(bp);
5260 }
5261 
5262 // Blocks.
5263 
5264 // Allocate a zeroed disk block.
5265 static uint
5266 balloc(uint dev)
5267 {
5268   int b, bi, m;
5269   struct buf *bp;
5270 
5271   bp = 0;
5272   for(b = 0; b < sb.size; b += BPB){
5273     bp = bread(dev, BBLOCK(b, sb));
5274     for(bi = 0; bi < BPB && b + bi < sb.size; bi++){
5275       m = 1 << (bi % 8);
5276       if((bp->data[bi/8] & m) == 0){  // Is block free?
5277         bp->data[bi/8] |= m;  // Mark block in use.
5278         log_write(bp);
5279         brelse(bp);
5280         bzero(dev, b + bi);
5281         return b + bi;
5282       }
5283     }
5284     brelse(bp);
5285   }
5286   panic("balloc: out of blocks");
5287 }
5288 
5289 
5290 
5291 
5292 
5293 
5294 
5295 
5296 
5297 
5298 
5299 
5300 // Free a disk block.
5301 static void
5302 bfree(int dev, uint b)
5303 {
5304   struct buf *bp;
5305   int bi, m;
5306 
5307   readsb(dev, &sb);
5308   bp = bread(dev, BBLOCK(b, sb));
5309   bi = b % BPB;
5310   m = 1 << (bi % 8);
5311   if((bp->data[bi/8] & m) == 0)
5312     panic("freeing free block");
5313   bp->data[bi/8] &= ~m;
5314   log_write(bp);
5315   brelse(bp);
5316 }
5317 
5318 // Inodes.
5319 //
5320 // An inode describes a single unnamed file.
5321 // The inode disk structure holds metadata: the file's type,
5322 // its size, the number of links referring to it, and the
5323 // list of blocks holding the file's content.
5324 //
5325 // The inodes are laid out sequentially on disk at
5326 // sb.startinode. Each inode has a number, indicating its
5327 // position on the disk.
5328 //
5329 // The kernel keeps a cache of in-use inodes in memory
5330 // to provide a place for synchronizing access
5331 // to inodes used by multiple processes. The cached
5332 // inodes include book-keeping information that is
5333 // not stored on disk: ip->ref and ip->valid.
5334 //
5335 // An inode and its in-memory representation go through a
5336 // sequence of states before they can be used by the
5337 // rest of the file system code.
5338 //
5339 // * Allocation: an inode is allocated if its type (on disk)
5340 //   is non-zero. ialloc() allocates, and iput() frees if
5341 //   the reference and link counts have fallen to zero.
5342 //
5343 // * Referencing in cache: an entry in the inode cache
5344 //   is free if ip->ref is zero. Otherwise ip->ref tracks
5345 //   the number of in-memory pointers to the entry (open
5346 //   files and current directories). iget() finds or
5347 //   creates a cache entry and increments its ref; iput()
5348 //   decrements ref.
5349 //
5350 // * Valid: the information (type, size, &c) in an inode
5351 //   cache entry is only correct when ip->valid is 1.
5352 //   ilock() reads the inode from
5353 //   the disk and sets ip->valid, while iput() clears
5354 //   ip->valid if ip->ref has fallen to zero.
5355 //
5356 // * Locked: file system code may only examine and modify
5357 //   the information in an inode and its content if it
5358 //   has first locked the inode.
5359 //
5360 // Thus a typical sequence is:
5361 //   ip = iget(dev, inum)
5362 //   ilock(ip)
5363 //   ... examine and modify ip->xxx ...
5364 //   iunlock(ip)
5365 //   iput(ip)
5366 //
5367 // ilock() is separate from iget() so that system calls can
5368 // get a long-term reference to an inode (as for an open file)
5369 // and only lock it for short periods (e.g., in read()).
5370 // The separation also helps avoid deadlock and races during
5371 // pathname lookup. iget() increments ip->ref so that the inode
5372 // stays cached and pointers to it remain valid.
5373 //
5374 // Many internal file system functions expect the caller to
5375 // have locked the inodes involved; this lets callers create
5376 // multi-step atomic operations.
5377 //
5378 // The icache.lock spin-lock protects the allocation of icache
5379 // entries. Since ip->ref indicates whether an entry is free,
5380 // and ip->dev and ip->inum indicate which i-node an entry
5381 // holds, one must hold icache.lock while using any of those fields.
5382 //
5383 // An ip->lock sleep-lock protects all ip-> fields other than ref,
5384 // dev, and inum.  One must hold ip->lock in order to
5385 // read or write that inode's ip->valid, ip->size, ip->type, &c.
5386 
5387 struct {
5388   struct spinlock lock;
5389   struct inode inode[NINODE];
5390 } icache;
5391 
5392 void
5393 iinit(int dev)
5394 {
5395   int i = 0;
5396 
5397   initlock(&icache.lock, "icache");
5398   for(i = 0; i < NINODE; i++) {
5399     initsleeplock(&icache.inode[i].lock, "inode");
5400   }
5401 
5402   readsb(dev, &sb);
5403   cprintf("sb: size %d nblocks %d ninodes %d nlog %d logstart %d\
5404  inodestart %d bmap start %d\n", sb.size, sb.nblocks,
5405           sb.ninodes, sb.nlog, sb.logstart, sb.inodestart,
5406           sb.bmapstart);
5407 }
5408 
5409 static struct inode* iget(uint dev, uint inum);
5410 
5411 
5412 
5413 
5414 
5415 
5416 
5417 
5418 
5419 
5420 
5421 
5422 
5423 
5424 
5425 
5426 
5427 
5428 
5429 
5430 
5431 
5432 
5433 
5434 
5435 
5436 
5437 
5438 
5439 
5440 
5441 
5442 
5443 
5444 
5445 
5446 
5447 
5448 
5449 
5450 // Allocate an inode on device dev.
5451 // Mark it as allocated by  giving it type type.
5452 // Returns an unlocked but allocated and referenced inode.
5453 struct inode*
5454 ialloc(uint dev, short type)
5455 {
5456   int inum;
5457   struct buf *bp;
5458   struct dinode *dip;
5459 
5460   for(inum = 1; inum < sb.ninodes; inum++){
5461     bp = bread(dev, IBLOCK(inum, sb));
5462     dip = (struct dinode*)bp->data + inum%IPB;
5463     if(dip->type == 0){  // a free inode
5464       memset(dip, 0, sizeof(*dip));
5465       dip->type = type;
5466       log_write(bp);   // mark it allocated on the disk
5467       brelse(bp);
5468       return iget(dev, inum);
5469     }
5470     brelse(bp);
5471   }
5472   panic("ialloc: no inodes");
5473 }
5474 
5475 // Copy a modified in-memory inode to disk.
5476 // Must be called after every change to an ip->xxx field
5477 // that lives on disk, since i-node cache is write-through.
5478 // Caller must hold ip->lock.
5479 void
5480 iupdate(struct inode *ip)
5481 {
5482   struct buf *bp;
5483   struct dinode *dip;
5484 
5485   bp = bread(ip->dev, IBLOCK(ip->inum, sb));
5486   dip = (struct dinode*)bp->data + ip->inum%IPB;
5487   dip->type = ip->type;
5488   dip->major = ip->major;
5489   dip->minor = ip->minor;
5490   dip->nlink = ip->nlink;
5491   dip->size = ip->size;
5492   memmove(dip->addrs, ip->addrs, sizeof(ip->addrs));
5493   log_write(bp);
5494   brelse(bp);
5495 }
5496 
5497 
5498 
5499 
5500 // Find the inode with number inum on device dev
5501 // and return the in-memory copy. Does not lock
5502 // the inode and does not read it from disk.
5503 static struct inode*
5504 iget(uint dev, uint inum)
5505 {
5506   struct inode *ip, *empty;
5507 
5508   acquire(&icache.lock);
5509 
5510   // Is the inode already cached?
5511   empty = 0;
5512   for(ip = &icache.inode[0]; ip < &icache.inode[NINODE]; ip++){
5513     if(ip->ref > 0 && ip->dev == dev && ip->inum == inum){
5514       ip->ref++;
5515       release(&icache.lock);
5516       return ip;
5517     }
5518     if(empty == 0 && ip->ref == 0)    // Remember empty slot.
5519       empty = ip;
5520   }
5521 
5522   // Recycle an inode cache entry.
5523   if(empty == 0)
5524     panic("iget: no inodes");
5525 
5526   ip = empty;
5527   ip->dev = dev;
5528   ip->inum = inum;
5529   ip->ref = 1;
5530   ip->valid = 0;
5531   release(&icache.lock);
5532 
5533   return ip;
5534 }
5535 
5536 // Increment reference count for ip.
5537 // Returns ip to enable ip = idup(ip1) idiom.
5538 struct inode*
5539 idup(struct inode *ip)
5540 {
5541   acquire(&icache.lock);
5542   ip->ref++;
5543   release(&icache.lock);
5544   return ip;
5545 }
5546 
5547 
5548 
5549 
5550 // Lock the given inode.
5551 // Reads the inode from disk if necessary.
5552 void
5553 ilock(struct inode *ip)
5554 {
5555   struct buf *bp;
5556   struct dinode *dip;
5557 
5558   if(ip == 0 || ip->ref < 1)
5559     panic("ilock");
5560 
5561   acquiresleep(&ip->lock);
5562 
5563   if(ip->valid == 0){
5564     bp = bread(ip->dev, IBLOCK(ip->inum, sb));
5565     dip = (struct dinode*)bp->data + ip->inum%IPB;
5566     ip->type = dip->type;
5567     ip->major = dip->major;
5568     ip->minor = dip->minor;
5569     ip->nlink = dip->nlink;
5570     ip->size = dip->size;
5571     memmove(ip->addrs, dip->addrs, sizeof(ip->addrs));
5572     brelse(bp);
5573     ip->valid = 1;
5574     if(ip->type == 0)
5575       panic("ilock: no type");
5576   }
5577 }
5578 
5579 // Unlock the given inode.
5580 void
5581 iunlock(struct inode *ip)
5582 {
5583   if(ip == 0 || !holdingsleep(&ip->lock) || ip->ref < 1)
5584     panic("iunlock");
5585 
5586   releasesleep(&ip->lock);
5587 }
5588 
5589 
5590 
5591 
5592 
5593 
5594 
5595 
5596 
5597 
5598 
5599 
5600 // Drop a reference to an in-memory inode.
5601 // If that was the last reference, the inode cache entry can
5602 // be recycled.
5603 // If that was the last reference and the inode has no links
5604 // to it, free the inode (and its content) on disk.
5605 // All calls to iput() must be inside a transaction in
5606 // case it has to free the inode.
5607 void
5608 iput(struct inode *ip)
5609 {
5610   acquiresleep(&ip->lock);
5611   if(ip->valid && ip->nlink == 0){
5612     acquire(&icache.lock);
5613     int r = ip->ref;
5614     release(&icache.lock);
5615     if(r == 1){
5616       // inode has no links and no other references: truncate and free.
5617       itrunc(ip);
5618       ip->type = 0;
5619       iupdate(ip);
5620       ip->valid = 0;
5621     }
5622   }
5623   releasesleep(&ip->lock);
5624 
5625   acquire(&icache.lock);
5626   ip->ref--;
5627   release(&icache.lock);
5628 }
5629 
5630 // Common idiom: unlock, then put.
5631 void
5632 iunlockput(struct inode *ip)
5633 {
5634   iunlock(ip);
5635   iput(ip);
5636 }
5637 
5638 
5639 
5640 
5641 
5642 
5643 
5644 
5645 
5646 
5647 
5648 
5649 
5650 // Inode content
5651 //
5652 // The content (data) associated with each inode is stored
5653 // in blocks on the disk. The first NDIRECT block numbers
5654 // are listed in ip->addrs[].  The next NINDIRECT blocks are
5655 // listed in block ip->addrs[NDIRECT].
5656 
5657 // Return the disk block address of the nth block in inode ip.
5658 // If there is no such block, bmap allocates one.
5659 static uint
5660 bmap(struct inode *ip, uint bn)
5661 {
5662   uint addr, *a;
5663   struct buf *bp;
5664 
5665   if(bn < NDIRECT){
5666     if((addr = ip->addrs[bn]) == 0)
5667       ip->addrs[bn] = addr = balloc(ip->dev);
5668     return addr;
5669   }
5670   bn -= NDIRECT;
5671 
5672   if(bn < NINDIRECT){
5673     // Load indirect block, allocating if necessary.
5674     if((addr = ip->addrs[NDIRECT]) == 0)
5675       ip->addrs[NDIRECT] = addr = balloc(ip->dev);
5676     bp = bread(ip->dev, addr);
5677     a = (uint*)bp->data;
5678     if((addr = a[bn]) == 0){
5679       a[bn] = addr = balloc(ip->dev);
5680       log_write(bp);
5681     }
5682     brelse(bp);
5683     return addr;
5684   }
5685 
5686   panic("bmap: out of range");
5687 }
5688 
5689 
5690 
5691 
5692 
5693 
5694 
5695 
5696 
5697 
5698 
5699 
5700 // Truncate inode (discard contents).
5701 // Only called when the inode has no links
5702 // to it (no directory entries referring to it)
5703 // and has no in-memory reference to it (is
5704 // not an open file or current directory).
5705 static void
5706 itrunc(struct inode *ip)
5707 {
5708   int i, j;
5709   struct buf *bp;
5710   uint *a;
5711 
5712   for(i = 0; i < NDIRECT; i++){
5713     if(ip->addrs[i]){
5714       bfree(ip->dev, ip->addrs[i]);
5715       ip->addrs[i] = 0;
5716     }
5717   }
5718 
5719   if(ip->addrs[NDIRECT]){
5720     bp = bread(ip->dev, ip->addrs[NDIRECT]);
5721     a = (uint*)bp->data;
5722     for(j = 0; j < NINDIRECT; j++){
5723       if(a[j])
5724         bfree(ip->dev, a[j]);
5725     }
5726     brelse(bp);
5727     bfree(ip->dev, ip->addrs[NDIRECT]);
5728     ip->addrs[NDIRECT] = 0;
5729   }
5730 
5731   ip->size = 0;
5732   iupdate(ip);
5733 }
5734 
5735 // Copy stat information from inode.
5736 // Caller must hold ip->lock.
5737 void
5738 stati(struct inode *ip, struct stat *st)
5739 {
5740   st->dev = ip->dev;
5741   st->ino = ip->inum;
5742   st->type = ip->type;
5743   st->nlink = ip->nlink;
5744   st->size = ip->size;
5745 }
5746 
5747 
5748 
5749 
5750 // Read data from inode.
5751 // Caller must hold ip->lock.
5752 int
5753 readi(struct inode *ip, char *dst, uint off, uint n)
5754 {
5755   uint tot, m;
5756   struct buf *bp;
5757 
5758   if(ip->type == T_DEV){
5759     if(ip->major < 0 || ip->major >= NDEV || !devsw[ip->major].read)
5760       return -1;
5761     return devsw[ip->major].read(ip, dst, n);
5762   }
5763 
5764   if(off > ip->size || off + n < off)
5765     return -1;
5766   if(off + n > ip->size)
5767     n = ip->size - off;
5768 
5769   for(tot=0; tot<n; tot+=m, off+=m, dst+=m){
5770     bp = bread(ip->dev, bmap(ip, off/BSIZE));
5771     m = min(n - tot, BSIZE - off%BSIZE);
5772     memmove(dst, bp->data + off%BSIZE, m);
5773     brelse(bp);
5774   }
5775   return n;
5776 }
5777 
5778 
5779 
5780 
5781 
5782 
5783 
5784 
5785 
5786 
5787 
5788 
5789 
5790 
5791 
5792 
5793 
5794 
5795 
5796 
5797 
5798 
5799 
5800 // Write data to inode.
5801 // Caller must hold ip->lock.
5802 int
5803 writei(struct inode *ip, char *src, uint off, uint n)
5804 {
5805   uint tot, m;
5806   struct buf *bp;
5807 
5808   if(ip->type == T_DEV){
5809     if(ip->major < 0 || ip->major >= NDEV || !devsw[ip->major].write)
5810       return -1;
5811     return devsw[ip->major].write(ip, src, n);
5812   }
5813 
5814   if(off > ip->size || off + n < off)
5815     return -1;
5816   if(off + n > MAXFILE*BSIZE)
5817     return -1;
5818 
5819   for(tot=0; tot<n; tot+=m, off+=m, src+=m){
5820     bp = bread(ip->dev, bmap(ip, off/BSIZE));
5821     m = min(n - tot, BSIZE - off%BSIZE);
5822     memmove(bp->data + off%BSIZE, src, m);
5823     log_write(bp);
5824     brelse(bp);
5825   }
5826 
5827   if(n > 0 && off > ip->size){
5828     ip->size = off;
5829     iupdate(ip);
5830   }
5831   return n;
5832 }
5833 
5834 
5835 
5836 
5837 
5838 
5839 
5840 
5841 
5842 
5843 
5844 
5845 
5846 
5847 
5848 
5849 
5850 // Directories
5851 
5852 int
5853 namecmp(const char *s, const char *t)
5854 {
5855   return strncmp(s, t, DIRSIZ);
5856 }
5857 
5858 // Look for a directory entry in a directory.
5859 // If found, set *poff to byte offset of entry.
5860 struct inode*
5861 dirlookup(struct inode *dp, char *name, uint *poff)
5862 {
5863   uint off, inum;
5864   struct dirent de;
5865 
5866   if(dp->type != T_DIR)
5867     panic("dirlookup not DIR");
5868 
5869   for(off = 0; off < dp->size; off += sizeof(de)){
5870     if(readi(dp, (char*)&de, off, sizeof(de)) != sizeof(de))
5871       panic("dirlookup read");
5872     if(de.inum == 0)
5873       continue;
5874     if(namecmp(name, de.name) == 0){
5875       // entry matches path element
5876       if(poff)
5877         *poff = off;
5878       inum = de.inum;
5879       return iget(dp->dev, inum);
5880     }
5881   }
5882 
5883   return 0;
5884 }
5885 
5886 
5887 
5888 
5889 
5890 
5891 
5892 
5893 
5894 
5895 
5896 
5897 
5898 
5899 
5900 // Write a new directory entry (name, inum) into the directory dp.
5901 int
5902 dirlink(struct inode *dp, char *name, uint inum)
5903 {
5904   int off;
5905   struct dirent de;
5906   struct inode *ip;
5907 
5908   // Check that name is not present.
5909   if((ip = dirlookup(dp, name, 0)) != 0){
5910     iput(ip);
5911     return -1;
5912   }
5913 
5914   // Look for an empty dirent.
5915   for(off = 0; off < dp->size; off += sizeof(de)){
5916     if(readi(dp, (char*)&de, off, sizeof(de)) != sizeof(de))
5917       panic("dirlink read");
5918     if(de.inum == 0)
5919       break;
5920   }
5921 
5922   strncpy(de.name, name, DIRSIZ);
5923   de.inum = inum;
5924   if(writei(dp, (char*)&de, off, sizeof(de)) != sizeof(de))
5925     panic("dirlink");
5926 
5927   return 0;
5928 }
5929 
5930 
5931 
5932 
5933 
5934 
5935 
5936 
5937 
5938 
5939 
5940 
5941 
5942 
5943 
5944 
5945 
5946 
5947 
5948 
5949 
5950 // Paths
5951 
5952 // Copy the next path element from path into name.
5953 // Return a pointer to the element following the copied one.
5954 // The returned path has no leading slashes,
5955 // so the caller can check *path=='\0' to see if the name is the last one.
5956 // If no name to remove, return 0.
5957 //
5958 // Examples:
5959 //   skipelem("a/bb/c", name) = "bb/c", setting name = "a"
5960 //   skipelem("///a//bb", name) = "bb", setting name = "a"
5961 //   skipelem("a", name) = "", setting name = "a"
5962 //   skipelem("", name) = skipelem("////", name) = 0
5963 //
5964 static char*
5965 skipelem(char *path, char *name)
5966 {
5967   char *s;
5968   int len;
5969 
5970   while(*path == '/')
5971     path++;
5972   if(*path == 0)
5973     return 0;
5974   s = path;
5975   while(*path != '/' && *path != 0)
5976     path++;
5977   len = path - s;
5978   if(len >= DIRSIZ)
5979     memmove(name, s, DIRSIZ);
5980   else {
5981     memmove(name, s, len);
5982     name[len] = 0;
5983   }
5984   while(*path == '/')
5985     path++;
5986   return path;
5987 }
5988 
5989 
5990 
5991 
5992 
5993 
5994 
5995 
5996 
5997 
5998 
5999 
6000 // Look up and return the inode for a path name.
6001 // If parent != 0, return the inode for the parent and copy the final
6002 // path element into name, which must have room for DIRSIZ bytes.
6003 // Must be called inside a transaction since it calls iput().
6004 static struct inode*
6005 namex(char *path, int nameiparent, char *name)
6006 {
6007   struct inode *ip, *next;
6008 
6009   if(*path == '/')
6010     ip = iget(ROOTDEV, ROOTINO);
6011   else
6012     ip = idup(myproc()->cwd);
6013 
6014   while((path = skipelem(path, name)) != 0){
6015     ilock(ip);
6016     if(ip->type != T_DIR){
6017       iunlockput(ip);
6018       return 0;
6019     }
6020     if(nameiparent && *path == '\0'){
6021       // Stop one level early.
6022       iunlock(ip);
6023       return ip;
6024     }
6025     if((next = dirlookup(ip, name, 0)) == 0){
6026       iunlockput(ip);
6027       return 0;
6028     }
6029     iunlockput(ip);
6030     ip = next;
6031   }
6032   if(nameiparent){
6033     iput(ip);
6034     return 0;
6035   }
6036   return ip;
6037 }
6038 
6039 struct inode*
6040 namei(char *path)
6041 {
6042   char name[DIRSIZ];
6043   return namex(path, 0, name);
6044 }
6045 
6046 
6047 
6048 
6049 
6050 struct inode*
6051 nameiparent(char *path, char *name)
6052 {
6053   return namex(path, 1, name);
6054 }
6055 
6056 
6057 
6058 
6059 
6060 
6061 
6062 
6063 
6064 
6065 
6066 
6067 
6068 
6069 
6070 
6071 
6072 
6073 
6074 
6075 
6076 
6077 
6078 
6079 
6080 
6081 
6082 
6083 
6084 
6085 
6086 
6087 
6088 
6089 
6090 
6091 
6092 
6093 
6094 
6095 
6096 
6097 
6098 
6099 
