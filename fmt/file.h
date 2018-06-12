4400 struct file {
4401   enum { FD_NONE, FD_PIPE, FD_INODE } type;
4402   int ref; // reference count
4403   char readable;
4404   char writable;
4405   struct pipe *pipe;
4406   struct inode *ip;
4407   uint off;
4408 };
4409 
4410 
4411 // in-memory copy of an inode
4412 struct inode {
4413   uint dev;           // Device number
4414   uint inum;          // Inode number
4415   int ref;            // Reference count
4416   struct sleeplock lock; // protects everything below here
4417   int valid;          // inode has been read from disk?
4418 
4419   short type;         // copy of disk inode
4420   short major;
4421   short minor;
4422   short nlink;
4423   uint size;
4424   uint addrs[NDIRECT+1];
4425 };
4426 
4427 // table mapping major device number to
4428 // device functions
4429 struct devsw {
4430   int (*read)(struct inode*, char*, int);
4431   int (*write)(struct inode*, char*, int);
4432 };
4433 
4434 extern struct devsw devsw[];
4435 
4436 #define CONSOLE 1
4437 
4438 
4439 
4440 
4441 
4442 
4443 
4444 
4445 
4446 
4447 
4448 
4449 
