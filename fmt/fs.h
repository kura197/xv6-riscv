4300 // On-disk file system format.
4301 // Both the kernel and user programs use this header file.
4302 
4303 
4304 #define ROOTINO 1  // root i-number
4305 #define BSIZE 512  // block size
4306 
4307 // Disk layout:
4308 // [ boot block | super block | log | inode blocks |
4309 //                                          free bit map | data blocks]
4310 //
4311 // mkfs computes the super block and builds an initial file system. The
4312 // super block describes the disk layout:
4313 struct superblock {
4314   uint size;         // Size of file system image (blocks)
4315   uint nblocks;      // Number of data blocks
4316   uint ninodes;      // Number of inodes.
4317   uint nlog;         // Number of log blocks
4318   uint logstart;     // Block number of first log block
4319   uint inodestart;   // Block number of first inode block
4320   uint bmapstart;    // Block number of first free map block
4321 };
4322 
4323 #define NDIRECT 12
4324 #define NINDIRECT (BSIZE / sizeof(uint))
4325 #define MAXFILE (NDIRECT + NINDIRECT)
4326 
4327 // On-disk inode structure
4328 struct dinode {
4329   short type;           // File type
4330   short major;          // Major device number (T_DEV only)
4331   short minor;          // Minor device number (T_DEV only)
4332   short nlink;          // Number of links to inode in file system
4333   uint size;            // Size of file (bytes)
4334   uint addrs[NDIRECT+1];   // Data block addresses
4335 };
4336 
4337 
4338 
4339 
4340 
4341 
4342 
4343 
4344 
4345 
4346 
4347 
4348 
4349 
4350 // Inodes per block.
4351 #define IPB           (BSIZE / sizeof(struct dinode))
4352 
4353 // Block containing inode i
4354 #define IBLOCK(i, sb)     ((i) / IPB + sb.inodestart)
4355 
4356 // Bitmap bits per block
4357 #define BPB           (BSIZE*8)
4358 
4359 // Block of free map containing bit for block b
4360 #define BBLOCK(b, sb) (b/BPB + sb.bmapstart)
4361 
4362 // Directory is a file containing a sequence of dirent structures.
4363 #define DIRSIZ 14
4364 
4365 struct dirent {
4366   ushort inum;
4367   char name[DIRSIZ];
4368 };
4369 
4370 
4371 
4372 
4373 
4374 
4375 
4376 
4377 
4378 
4379 
4380 
4381 
4382 
4383 
4384 
4385 
4386 
4387 
4388 
4389 
4390 
4391 
4392 
4393 
4394 
4395 
4396 
4397 
4398 
4399 
