6100 //
6101 // File descriptors
6102 //
6103 
6104 #include "types.h"
6105 #include "defs.h"
6106 #include "param.h"
6107 #include "fs.h"
6108 #include "spinlock.h"
6109 #include "sleeplock.h"
6110 #include "file.h"
6111 
6112 struct devsw devsw[NDEV];
6113 struct {
6114   struct spinlock lock;
6115   struct file file[NFILE];
6116 } ftable;
6117 
6118 void
6119 fileinit(void)
6120 {
6121   initlock(&ftable.lock, "ftable");
6122 }
6123 
6124 // Allocate a file structure.
6125 struct file*
6126 filealloc(void)
6127 {
6128   struct file *f;
6129 
6130   acquire(&ftable.lock);
6131   for(f = ftable.file; f < ftable.file + NFILE; f++){
6132     if(f->ref == 0){
6133       f->ref = 1;
6134       release(&ftable.lock);
6135       return f;
6136     }
6137   }
6138   release(&ftable.lock);
6139   return 0;
6140 }
6141 
6142 
6143 
6144 
6145 
6146 
6147 
6148 
6149 
6150 // Increment ref count for file f.
6151 struct file*
6152 filedup(struct file *f)
6153 {
6154   acquire(&ftable.lock);
6155   if(f->ref < 1)
6156     panic("filedup");
6157   f->ref++;
6158   release(&ftable.lock);
6159   return f;
6160 }
6161 
6162 // Close file f.  (Decrement ref count, close when reaches 0.)
6163 void
6164 fileclose(struct file *f)
6165 {
6166   struct file ff;
6167 
6168   acquire(&ftable.lock);
6169   if(f->ref < 1)
6170     panic("fileclose");
6171   if(--f->ref > 0){
6172     release(&ftable.lock);
6173     return;
6174   }
6175   ff = *f;
6176   f->ref = 0;
6177   f->type = FD_NONE;
6178   release(&ftable.lock);
6179 
6180   if(ff.type == FD_PIPE)
6181     pipeclose(ff.pipe, ff.writable);
6182   else if(ff.type == FD_INODE){
6183     begin_op();
6184     iput(ff.ip);
6185     end_op();
6186   }
6187 }
6188 
6189 
6190 
6191 
6192 
6193 
6194 
6195 
6196 
6197 
6198 
6199 
6200 // Get metadata about file f.
6201 int
6202 filestat(struct file *f, struct stat *st)
6203 {
6204   if(f->type == FD_INODE){
6205     ilock(f->ip);
6206     stati(f->ip, st);
6207     iunlock(f->ip);
6208     return 0;
6209   }
6210   return -1;
6211 }
6212 
6213 // Read from file f.
6214 int
6215 fileread(struct file *f, char *addr, int n)
6216 {
6217   int r;
6218 
6219   if(f->readable == 0)
6220     return -1;
6221   if(f->type == FD_PIPE)
6222     return piperead(f->pipe, addr, n);
6223   if(f->type == FD_INODE){
6224     ilock(f->ip);
6225     if((r = readi(f->ip, addr, f->off, n)) > 0)
6226       f->off += r;
6227     iunlock(f->ip);
6228     return r;
6229   }
6230   panic("fileread");
6231 }
6232 
6233 
6234 
6235 
6236 
6237 
6238 
6239 
6240 
6241 
6242 
6243 
6244 
6245 
6246 
6247 
6248 
6249 
6250 // Write to file f.
6251 int
6252 filewrite(struct file *f, char *addr, int n)
6253 {
6254   int r;
6255 
6256   if(f->writable == 0)
6257     return -1;
6258   if(f->type == FD_PIPE)
6259     return pipewrite(f->pipe, addr, n);
6260   if(f->type == FD_INODE){
6261     // write a few blocks at a time to avoid exceeding
6262     // the maximum log transaction size, including
6263     // i-node, indirect block, allocation blocks,
6264     // and 2 blocks of slop for non-aligned writes.
6265     // this really belongs lower down, since writei()
6266     // might be writing a device like the console.
6267     int max = ((MAXOPBLOCKS-1-1-2) / 2) * 512;
6268     int i = 0;
6269     while(i < n){
6270       int n1 = n - i;
6271       if(n1 > max)
6272         n1 = max;
6273 
6274       begin_op();
6275       ilock(f->ip);
6276       if ((r = writei(f->ip, addr + i, f->off, n1)) > 0)
6277         f->off += r;
6278       iunlock(f->ip);
6279       end_op();
6280 
6281       if(r < 0)
6282         break;
6283       if(r != n1)
6284         panic("short filewrite");
6285       i += r;
6286     }
6287     return i == n ? n : -1;
6288   }
6289   panic("filewrite");
6290 }
6291 
6292 
6293 
6294 
6295 
6296 
6297 
6298 
6299 
