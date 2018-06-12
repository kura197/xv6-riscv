6300 //
6301 // File-system system calls.
6302 // Mostly argument checking, since we don't trust
6303 // user code, and calls into file.c and fs.c.
6304 //
6305 
6306 #include "types.h"
6307 #include "defs.h"
6308 #include "param.h"
6309 #include "stat.h"
6310 #include "mmu.h"
6311 #include "proc.h"
6312 #include "fs.h"
6313 #include "spinlock.h"
6314 #include "sleeplock.h"
6315 #include "file.h"
6316 #include "fcntl.h"
6317 
6318 // Fetch the nth word-sized system call argument as a file descriptor
6319 // and return both the descriptor and the corresponding struct file.
6320 static int
6321 argfd(int n, int *pfd, struct file **pf)
6322 {
6323   int fd;
6324   struct file *f;
6325 
6326   if(argint(n, &fd) < 0)
6327     return -1;
6328   if(fd < 0 || fd >= NOFILE || (f=myproc()->ofile[fd]) == 0)
6329     return -1;
6330   if(pfd)
6331     *pfd = fd;
6332   if(pf)
6333     *pf = f;
6334   return 0;
6335 }
6336 
6337 
6338 
6339 
6340 
6341 
6342 
6343 
6344 
6345 
6346 
6347 
6348 
6349 
6350 // Allocate a file descriptor for the given file.
6351 // Takes over file reference from caller on success.
6352 static int
6353 fdalloc(struct file *f)
6354 {
6355   int fd;
6356   struct proc *curproc = myproc();
6357 
6358   for(fd = 0; fd < NOFILE; fd++){
6359     if(curproc->ofile[fd] == 0){
6360       curproc->ofile[fd] = f;
6361       return fd;
6362     }
6363   }
6364   return -1;
6365 }
6366 
6367 int
6368 sys_dup(void)
6369 {
6370   struct file *f;
6371   int fd;
6372 
6373   if(argfd(0, 0, &f) < 0)
6374     return -1;
6375   if((fd=fdalloc(f)) < 0)
6376     return -1;
6377   filedup(f);
6378   return fd;
6379 }
6380 
6381 int
6382 sys_read(void)
6383 {
6384   struct file *f;
6385   int n;
6386   char *p;
6387 
6388   if(argfd(0, 0, &f) < 0 || argint(2, &n) < 0 || argptr(1, &p, n) < 0)
6389     return -1;
6390   return fileread(f, p, n);
6391 }
6392 
6393 
6394 
6395 
6396 
6397 
6398 
6399 
6400 int
6401 sys_write(void)
6402 {
6403   struct file *f;
6404   int n;
6405   char *p;
6406 
6407   if(argfd(0, 0, &f) < 0 || argint(2, &n) < 0 || argptr(1, &p, n) < 0)
6408     return -1;
6409   return filewrite(f, p, n);
6410 }
6411 
6412 int
6413 sys_close(void)
6414 {
6415   int fd;
6416   struct file *f;
6417 
6418   if(argfd(0, &fd, &f) < 0)
6419     return -1;
6420   myproc()->ofile[fd] = 0;
6421   fileclose(f);
6422   return 0;
6423 }
6424 
6425 int
6426 sys_fstat(void)
6427 {
6428   struct file *f;
6429   struct stat *st;
6430 
6431   if(argfd(0, 0, &f) < 0 || argptr(1, (void*)&st, sizeof(*st)) < 0)
6432     return -1;
6433   return filestat(f, st);
6434 }
6435 
6436 
6437 
6438 
6439 
6440 
6441 
6442 
6443 
6444 
6445 
6446 
6447 
6448 
6449 
6450 // Create the path new as a link to the same inode as old.
6451 int
6452 sys_link(void)
6453 {
6454   char name[DIRSIZ], *new, *old;
6455   struct inode *dp, *ip;
6456 
6457   if(argstr(0, &old) < 0 || argstr(1, &new) < 0)
6458     return -1;
6459 
6460   begin_op();
6461   if((ip = namei(old)) == 0){
6462     end_op();
6463     return -1;
6464   }
6465 
6466   ilock(ip);
6467   if(ip->type == T_DIR){
6468     iunlockput(ip);
6469     end_op();
6470     return -1;
6471   }
6472 
6473   ip->nlink++;
6474   iupdate(ip);
6475   iunlock(ip);
6476 
6477   if((dp = nameiparent(new, name)) == 0)
6478     goto bad;
6479   ilock(dp);
6480   if(dp->dev != ip->dev || dirlink(dp, name, ip->inum) < 0){
6481     iunlockput(dp);
6482     goto bad;
6483   }
6484   iunlockput(dp);
6485   iput(ip);
6486 
6487   end_op();
6488 
6489   return 0;
6490 
6491 bad:
6492   ilock(ip);
6493   ip->nlink--;
6494   iupdate(ip);
6495   iunlockput(ip);
6496   end_op();
6497   return -1;
6498 }
6499 
6500 // Is the directory dp empty except for "." and ".." ?
6501 static int
6502 isdirempty(struct inode *dp)
6503 {
6504   int off;
6505   struct dirent de;
6506 
6507   for(off=2*sizeof(de); off<dp->size; off+=sizeof(de)){
6508     if(readi(dp, (char*)&de, off, sizeof(de)) != sizeof(de))
6509       panic("isdirempty: readi");
6510     if(de.inum != 0)
6511       return 0;
6512   }
6513   return 1;
6514 }
6515 
6516 
6517 
6518 
6519 
6520 
6521 
6522 
6523 
6524 
6525 
6526 
6527 
6528 
6529 
6530 
6531 
6532 
6533 
6534 
6535 
6536 
6537 
6538 
6539 
6540 
6541 
6542 
6543 
6544 
6545 
6546 
6547 
6548 
6549 
6550 int
6551 sys_unlink(void)
6552 {
6553   struct inode *ip, *dp;
6554   struct dirent de;
6555   char name[DIRSIZ], *path;
6556   uint off;
6557 
6558   if(argstr(0, &path) < 0)
6559     return -1;
6560 
6561   begin_op();
6562   if((dp = nameiparent(path, name)) == 0){
6563     end_op();
6564     return -1;
6565   }
6566 
6567   ilock(dp);
6568 
6569   // Cannot unlink "." or "..".
6570   if(namecmp(name, ".") == 0 || namecmp(name, "..") == 0)
6571     goto bad;
6572 
6573   if((ip = dirlookup(dp, name, &off)) == 0)
6574     goto bad;
6575   ilock(ip);
6576 
6577   if(ip->nlink < 1)
6578     panic("unlink: nlink < 1");
6579   if(ip->type == T_DIR && !isdirempty(ip)){
6580     iunlockput(ip);
6581     goto bad;
6582   }
6583 
6584   memset(&de, 0, sizeof(de));
6585   if(writei(dp, (char*)&de, off, sizeof(de)) != sizeof(de))
6586     panic("unlink: writei");
6587   if(ip->type == T_DIR){
6588     dp->nlink--;
6589     iupdate(dp);
6590   }
6591   iunlockput(dp);
6592 
6593   ip->nlink--;
6594   iupdate(ip);
6595   iunlockput(ip);
6596 
6597   end_op();
6598 
6599   return 0;
6600 bad:
6601   iunlockput(dp);
6602   end_op();
6603   return -1;
6604 }
6605 
6606 static struct inode*
6607 create(char *path, short type, short major, short minor)
6608 {
6609   uint off;
6610   struct inode *ip, *dp;
6611   char name[DIRSIZ];
6612 
6613   if((dp = nameiparent(path, name)) == 0)
6614     return 0;
6615   ilock(dp);
6616 
6617   if((ip = dirlookup(dp, name, &off)) != 0){
6618     iunlockput(dp);
6619     ilock(ip);
6620     if(type == T_FILE && ip->type == T_FILE)
6621       return ip;
6622     iunlockput(ip);
6623     return 0;
6624   }
6625 
6626   if((ip = ialloc(dp->dev, type)) == 0)
6627     panic("create: ialloc");
6628 
6629   ilock(ip);
6630   ip->major = major;
6631   ip->minor = minor;
6632   ip->nlink = 1;
6633   iupdate(ip);
6634 
6635   if(type == T_DIR){  // Create . and .. entries.
6636     dp->nlink++;  // for ".."
6637     iupdate(dp);
6638     // No ip->nlink++ for ".": avoid cyclic ref count.
6639     if(dirlink(ip, ".", ip->inum) < 0 || dirlink(ip, "..", dp->inum) < 0)
6640       panic("create dots");
6641   }
6642 
6643   if(dirlink(dp, name, ip->inum) < 0)
6644     panic("create: dirlink");
6645 
6646   iunlockput(dp);
6647 
6648   return ip;
6649 }
6650 int
6651 sys_open(void)
6652 {
6653   char *path;
6654   int fd, omode;
6655   struct file *f;
6656   struct inode *ip;
6657 
6658   if(argstr(0, &path) < 0 || argint(1, &omode) < 0)
6659     return -1;
6660 
6661   begin_op();
6662 
6663   if(omode & O_CREATE){
6664     ip = create(path, T_FILE, 0, 0);
6665     if(ip == 0){
6666       end_op();
6667       return -1;
6668     }
6669   } else {
6670     if((ip = namei(path)) == 0){
6671       end_op();
6672       return -1;
6673     }
6674     ilock(ip);
6675     if(ip->type == T_DIR && omode != O_RDONLY){
6676       iunlockput(ip);
6677       end_op();
6678       return -1;
6679     }
6680   }
6681 
6682   if((f = filealloc()) == 0 || (fd = fdalloc(f)) < 0){
6683     if(f)
6684       fileclose(f);
6685     iunlockput(ip);
6686     end_op();
6687     return -1;
6688   }
6689   iunlock(ip);
6690   end_op();
6691 
6692   f->type = FD_INODE;
6693   f->ip = ip;
6694   f->off = 0;
6695   f->readable = !(omode & O_WRONLY);
6696   f->writable = (omode & O_WRONLY) || (omode & O_RDWR);
6697   return fd;
6698 }
6699 
6700 int
6701 sys_mkdir(void)
6702 {
6703   char *path;
6704   struct inode *ip;
6705 
6706   begin_op();
6707   if(argstr(0, &path) < 0 || (ip = create(path, T_DIR, 0, 0)) == 0){
6708     end_op();
6709     return -1;
6710   }
6711   iunlockput(ip);
6712   end_op();
6713   return 0;
6714 }
6715 
6716 int
6717 sys_mknod(void)
6718 {
6719   struct inode *ip;
6720   char *path;
6721   int major, minor;
6722 
6723   begin_op();
6724   if((argstr(0, &path)) < 0 ||
6725      argint(1, &major) < 0 ||
6726      argint(2, &minor) < 0 ||
6727      (ip = create(path, T_DEV, major, minor)) == 0){
6728     end_op();
6729     return -1;
6730   }
6731   iunlockput(ip);
6732   end_op();
6733   return 0;
6734 }
6735 
6736 
6737 
6738 
6739 
6740 
6741 
6742 
6743 
6744 
6745 
6746 
6747 
6748 
6749 
6750 int
6751 sys_chdir(void)
6752 {
6753   char *path;
6754   struct inode *ip;
6755   struct proc *curproc = myproc();
6756 
6757   begin_op();
6758   if(argstr(0, &path) < 0 || (ip = namei(path)) == 0){
6759     end_op();
6760     return -1;
6761   }
6762   ilock(ip);
6763   if(ip->type != T_DIR){
6764     iunlockput(ip);
6765     end_op();
6766     return -1;
6767   }
6768   iunlock(ip);
6769   iput(curproc->cwd);
6770   end_op();
6771   curproc->cwd = ip;
6772   return 0;
6773 }
6774 
6775 int
6776 sys_exec(void)
6777 {
6778   char *path, *argv[MAXARG];
6779   int i;
6780   uint uargv, uarg;
6781 
6782   if(argstr(0, &path) < 0 || argint(1, (int*)&uargv) < 0){
6783     return -1;
6784   }
6785   memset(argv, 0, sizeof(argv));
6786   for(i=0;; i++){
6787     if(i >= NELEM(argv))
6788       return -1;
6789     if(fetchint(uargv+4*i, (int*)&uarg) < 0)
6790       return -1;
6791     if(uarg == 0){
6792       argv[i] = 0;
6793       break;
6794     }
6795     if(fetchstr(uarg, &argv[i]) < 0)
6796       return -1;
6797   }
6798   return exec(path, argv);
6799 }
6800 int
6801 sys_pipe(void)
6802 {
6803   int *fd;
6804   struct file *rf, *wf;
6805   int fd0, fd1;
6806 
6807   if(argptr(0, (void*)&fd, 2*sizeof(fd[0])) < 0)
6808     return -1;
6809   if(pipealloc(&rf, &wf) < 0)
6810     return -1;
6811   fd0 = -1;
6812   if((fd0 = fdalloc(rf)) < 0 || (fd1 = fdalloc(wf)) < 0){
6813     if(fd0 >= 0)
6814       myproc()->ofile[fd0] = 0;
6815     fileclose(rf);
6816     fileclose(wf);
6817     return -1;
6818   }
6819   fd[0] = fd0;
6820   fd[1] = fd1;
6821   return 0;
6822 }
6823 
6824 
6825 
6826 
6827 
6828 
6829 
6830 
6831 
6832 
6833 
6834 
6835 
6836 
6837 
6838 
6839 
6840 
6841 
6842 
6843 
6844 
6845 
6846 
6847 
6848 
6849 
