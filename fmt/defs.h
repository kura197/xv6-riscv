0250 struct buf;
0251 struct context;
0252 struct file;
0253 struct inode;
0254 struct pipe;
0255 struct proc;
0256 struct rtcdate;
0257 struct spinlock;
0258 struct sleeplock;
0259 struct stat;
0260 struct superblock;
0261 
0262 // bio.c
0263 void            binit(void);
0264 struct buf*     bread(uint, uint);
0265 void            brelse(struct buf*);
0266 void            bwrite(struct buf*);
0267 
0268 // console.c
0269 void            consoleinit(void);
0270 void            cprintf(char*, ...);
0271 void            consoleintr(int(*)(void));
0272 void            panic(char*) __attribute__((noreturn));
0273 
0274 // exec.c
0275 int             exec(char*, char**);
0276 
0277 // file.c
0278 struct file*    filealloc(void);
0279 void            fileclose(struct file*);
0280 struct file*    filedup(struct file*);
0281 void            fileinit(void);
0282 int             fileread(struct file*, char*, int n);
0283 int             filestat(struct file*, struct stat*);
0284 int             filewrite(struct file*, char*, int n);
0285 
0286 // fs.c
0287 void            readsb(int dev, struct superblock *sb);
0288 int             dirlink(struct inode*, char*, uint);
0289 struct inode*   dirlookup(struct inode*, char*, uint*);
0290 struct inode*   ialloc(uint, short);
0291 struct inode*   idup(struct inode*);
0292 void            iinit(int dev);
0293 void            ilock(struct inode*);
0294 void            iput(struct inode*);
0295 void            iunlock(struct inode*);
0296 void            iunlockput(struct inode*);
0297 void            iupdate(struct inode*);
0298 int             namecmp(const char*, const char*);
0299 struct inode*   namei(char*);
0300 struct inode*   nameiparent(char*, char*);
0301 int             readi(struct inode*, char*, uint, uint);
0302 void            stati(struct inode*, struct stat*);
0303 int             writei(struct inode*, char*, uint, uint);
0304 
0305 // ide.c
0306 void            ideinit(void);
0307 void            ideintr(void);
0308 void            iderw(struct buf*);
0309 
0310 // ioapic.c
0311 void            ioapicenable(int irq, int cpu);
0312 extern uchar    ioapicid;
0313 void            ioapicinit(void);
0314 
0315 // kalloc.c
0316 char*           kalloc(void);
0317 void            kfree(char*);
0318 void            kinit1(void*, void*);
0319 void            kinit2(void*, void*);
0320 
0321 // kbd.c
0322 void            kbdintr(void);
0323 
0324 // lapic.c
0325 void            cmostime(struct rtcdate *r);
0326 int             lapicid(void);
0327 extern volatile uint*    lapic;
0328 void            lapiceoi(void);
0329 void            lapicinit(void);
0330 void            lapicstartap(uchar, uint);
0331 void            microdelay(int);
0332 
0333 // log.c
0334 void            initlog(int dev);
0335 void            log_write(struct buf*);
0336 void            begin_op();
0337 void            end_op();
0338 
0339 // mp.c
0340 extern int      ismp;
0341 void            mpinit(void);
0342 
0343 // picirq.c
0344 void            picenable(int);
0345 void            picinit(void);
0346 
0347 
0348 
0349 
0350 // pipe.c
0351 int             pipealloc(struct file**, struct file**);
0352 void            pipeclose(struct pipe*, int);
0353 int             piperead(struct pipe*, char*, int);
0354 int             pipewrite(struct pipe*, char*, int);
0355 
0356 
0357 // proc.c
0358 int             cpuid(void);
0359 void            exit(void);
0360 int             fork(void);
0361 int             growproc(int);
0362 int             kill(int);
0363 struct cpu*     mycpu(void);
0364 struct proc*    myproc();
0365 void            pinit(void);
0366 void            procdump(void);
0367 void            scheduler(void) __attribute__((noreturn));
0368 void            sched(void);
0369 void            setproc(struct proc*);
0370 void            sleep(void*, struct spinlock*);
0371 void            userinit(void);
0372 int             wait(void);
0373 void            wakeup(void*);
0374 void            yield(void);
0375 
0376 // swtch.S
0377 void            swtch(struct context**, struct context*);
0378 
0379 // spinlock.c
0380 void            acquire(struct spinlock*);
0381 void            getcallerpcs(void*, uint*);
0382 int             holding(struct spinlock*);
0383 void            initlock(struct spinlock*, char*);
0384 void            release(struct spinlock*);
0385 void            pushcli(void);
0386 void            popcli(void);
0387 
0388 // sleeplock.c
0389 void            acquiresleep(struct sleeplock*);
0390 void            releasesleep(struct sleeplock*);
0391 int             holdingsleep(struct sleeplock*);
0392 void            initsleeplock(struct sleeplock*, char*);
0393 
0394 // string.c
0395 int             memcmp(const void*, const void*, uint);
0396 void*           memmove(void*, const void*, uint);
0397 void*           memset(void*, int, uint);
0398 char*           safestrcpy(char*, const char*, int);
0399 int             strlen(const char*);
0400 int             strncmp(const char*, const char*, uint);
0401 char*           strncpy(char*, const char*, int);
0402 
0403 // syscall.c
0404 int             argint(int, int*);
0405 int             argptr(int, char**, int);
0406 int             argstr(int, char**);
0407 int             fetchint(uint, int*);
0408 int             fetchstr(uint, char**);
0409 void            syscall(void);
0410 
0411 // timer.c
0412 void            timerinit(void);
0413 
0414 // trap.c
0415 void            idtinit(void);
0416 extern uint     ticks;
0417 void            tvinit(void);
0418 extern struct spinlock tickslock;
0419 
0420 // uart.c
0421 void            uartinit(void);
0422 void            uartintr(void);
0423 void            uartputc(int);
0424 
0425 // vm.c
0426 void            seginit(void);
0427 void            kvmalloc(void);
0428 pde_t*          setupkvm(void);
0429 char*           uva2ka(pde_t*, char*);
0430 int             allocuvm(pde_t*, uint, uint);
0431 int             deallocuvm(pde_t*, uint, uint);
0432 void            freevm(pde_t*);
0433 void            inituvm(pde_t*, char*, uint);
0434 int             loaduvm(pde_t*, char*, struct inode*, uint, uint);
0435 pde_t*          copyuvm(pde_t*, uint);
0436 void            switchuvm(struct proc*);
0437 void            switchkvm(void);
0438 int             copyout(pde_t*, uint, void*, uint);
0439 void            clearpteu(pde_t *pgdir, char *uva);
0440 
0441 // number of elements in fixed-size array
0442 #define NELEM(x) (sizeof(x)/sizeof((x)[0]))
0443 
0444 
0445 
0446 
0447 
0448 
0449 
