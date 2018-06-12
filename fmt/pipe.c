7000 #include "types.h"
7001 #include "defs.h"
7002 #include "param.h"
7003 #include "mmu.h"
7004 #include "proc.h"
7005 #include "fs.h"
7006 #include "spinlock.h"
7007 #include "sleeplock.h"
7008 #include "file.h"
7009 
7010 #define PIPESIZE 512
7011 
7012 struct pipe {
7013   struct spinlock lock;
7014   char data[PIPESIZE];
7015   uint nread;     // number of bytes read
7016   uint nwrite;    // number of bytes written
7017   int readopen;   // read fd is still open
7018   int writeopen;  // write fd is still open
7019 };
7020 
7021 int
7022 pipealloc(struct file **f0, struct file **f1)
7023 {
7024   struct pipe *p;
7025 
7026   p = 0;
7027   *f0 = *f1 = 0;
7028   if((*f0 = filealloc()) == 0 || (*f1 = filealloc()) == 0)
7029     goto bad;
7030   if((p = (struct pipe*)kalloc()) == 0)
7031     goto bad;
7032   p->readopen = 1;
7033   p->writeopen = 1;
7034   p->nwrite = 0;
7035   p->nread = 0;
7036   initlock(&p->lock, "pipe");
7037   (*f0)->type = FD_PIPE;
7038   (*f0)->readable = 1;
7039   (*f0)->writable = 0;
7040   (*f0)->pipe = p;
7041   (*f1)->type = FD_PIPE;
7042   (*f1)->readable = 0;
7043   (*f1)->writable = 1;
7044   (*f1)->pipe = p;
7045   return 0;
7046 
7047 
7048 
7049 
7050  bad:
7051   if(p)
7052     kfree((char*)p);
7053   if(*f0)
7054     fileclose(*f0);
7055   if(*f1)
7056     fileclose(*f1);
7057   return -1;
7058 }
7059 
7060 void
7061 pipeclose(struct pipe *p, int writable)
7062 {
7063   acquire(&p->lock);
7064   if(writable){
7065     p->writeopen = 0;
7066     wakeup(&p->nread);
7067   } else {
7068     p->readopen = 0;
7069     wakeup(&p->nwrite);
7070   }
7071   if(p->readopen == 0 && p->writeopen == 0){
7072     release(&p->lock);
7073     kfree((char*)p);
7074   } else
7075     release(&p->lock);
7076 }
7077 
7078 
7079 int
7080 pipewrite(struct pipe *p, char *addr, int n)
7081 {
7082   int i;
7083 
7084   acquire(&p->lock);
7085   for(i = 0; i < n; i++){
7086     while(p->nwrite == p->nread + PIPESIZE){  //DOC: pipewrite-full
7087       if(p->readopen == 0 || myproc()->killed){
7088         release(&p->lock);
7089         return -1;
7090       }
7091       wakeup(&p->nread);
7092       sleep(&p->nwrite, &p->lock);  //DOC: pipewrite-sleep
7093     }
7094     p->data[p->nwrite++ % PIPESIZE] = addr[i];
7095   }
7096   wakeup(&p->nread);  //DOC: pipewrite-wakeup1
7097   release(&p->lock);
7098   return n;
7099 }
7100 int
7101 piperead(struct pipe *p, char *addr, int n)
7102 {
7103   int i;
7104 
7105   acquire(&p->lock);
7106   while(p->nread == p->nwrite && p->writeopen){  //DOC: pipe-empty
7107     if(myproc()->killed){
7108       release(&p->lock);
7109       return -1;
7110     }
7111     sleep(&p->nread, &p->lock); //DOC: piperead-sleep
7112   }
7113   for(i = 0; i < n; i++){  //DOC: piperead-copy
7114     if(p->nread == p->nwrite)
7115       break;
7116     addr[i] = p->data[p->nread++ % PIPESIZE];
7117   }
7118   wakeup(&p->nwrite);  //DOC: piperead-wakeup
7119   release(&p->lock);
7120   return i;
7121 }
7122 
7123 
7124 
7125 
7126 
7127 
7128 
7129 
7130 
7131 
7132 
7133 
7134 
7135 
7136 
7137 
7138 
7139 
7140 
7141 
7142 
7143 
7144 
7145 
7146 
7147 
7148 
7149 
