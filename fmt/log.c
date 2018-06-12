4950 #include "types.h"
4951 #include "defs.h"
4952 #include "param.h"
4953 #include "spinlock.h"
4954 #include "sleeplock.h"
4955 #include "fs.h"
4956 #include "buf.h"
4957 
4958 // Simple logging that allows concurrent FS system calls.
4959 //
4960 // A log transaction contains the updates of multiple FS system
4961 // calls. The logging system only commits when there are
4962 // no FS system calls active. Thus there is never
4963 // any reasoning required about whether a commit might
4964 // write an uncommitted system call's updates to disk.
4965 //
4966 // A system call should call begin_op()/end_op() to mark
4967 // its start and end. Usually begin_op() just increments
4968 // the count of in-progress FS system calls and returns.
4969 // But if it thinks the log is close to running out, it
4970 // sleeps until the last outstanding end_op() commits.
4971 //
4972 // The log is a physical re-do log containing disk blocks.
4973 // The on-disk log format:
4974 //   header block, containing block #s for block A, B, C, ...
4975 //   block A
4976 //   block B
4977 //   block C
4978 //   ...
4979 // Log appends are synchronous.
4980 
4981 // Contents of the header block, used for both the on-disk header block
4982 // and to keep track in memory of logged block# before commit.
4983 struct logheader {
4984   int n;
4985   int block[LOGSIZE];
4986 };
4987 
4988 struct log {
4989   struct spinlock lock;
4990   int start;
4991   int size;
4992   int outstanding; // how many FS sys calls are executing.
4993   int committing;  // in commit(), please wait.
4994   int dev;
4995   struct logheader lh;
4996 };
4997 
4998 
4999 
5000 struct log log;
5001 
5002 static void recover_from_log(void);
5003 static void commit();
5004 
5005 void
5006 initlog(int dev)
5007 {
5008   if (sizeof(struct logheader) >= BSIZE)
5009     panic("initlog: too big logheader");
5010 
5011   struct superblock sb;
5012   initlock(&log.lock, "log");
5013   readsb(dev, &sb);
5014   log.start = sb.logstart;
5015   log.size = sb.nlog;
5016   log.dev = dev;
5017   recover_from_log();
5018 }
5019 
5020 // Copy committed blocks from log to their home location
5021 static void
5022 install_trans(void)
5023 {
5024   int tail;
5025 
5026   for (tail = 0; tail < log.lh.n; tail++) {
5027     struct buf *lbuf = bread(log.dev, log.start+tail+1); // read log block
5028     struct buf *dbuf = bread(log.dev, log.lh.block[tail]); // read dst
5029     memmove(dbuf->data, lbuf->data, BSIZE);  // copy block to dst
5030     bwrite(dbuf);  // write dst to disk
5031     brelse(lbuf);
5032     brelse(dbuf);
5033   }
5034 }
5035 
5036 // Read the log header from disk into the in-memory log header
5037 static void
5038 read_head(void)
5039 {
5040   struct buf *buf = bread(log.dev, log.start);
5041   struct logheader *lh = (struct logheader *) (buf->data);
5042   int i;
5043   log.lh.n = lh->n;
5044   for (i = 0; i < log.lh.n; i++) {
5045     log.lh.block[i] = lh->block[i];
5046   }
5047   brelse(buf);
5048 }
5049 
5050 // Write in-memory log header to disk.
5051 // This is the true point at which the
5052 // current transaction commits.
5053 static void
5054 write_head(void)
5055 {
5056   struct buf *buf = bread(log.dev, log.start);
5057   struct logheader *hb = (struct logheader *) (buf->data);
5058   int i;
5059   hb->n = log.lh.n;
5060   for (i = 0; i < log.lh.n; i++) {
5061     hb->block[i] = log.lh.block[i];
5062   }
5063   bwrite(buf);
5064   brelse(buf);
5065 }
5066 
5067 static void
5068 recover_from_log(void)
5069 {
5070   read_head();
5071   install_trans(); // if committed, copy from log to disk
5072   log.lh.n = 0;
5073   write_head(); // clear the log
5074 }
5075 
5076 // called at the start of each FS system call.
5077 void
5078 begin_op(void)
5079 {
5080   acquire(&log.lock);
5081   while(1){
5082     if(log.committing){
5083       sleep(&log, &log.lock);
5084     } else if(log.lh.n + (log.outstanding+1)*MAXOPBLOCKS > LOGSIZE){
5085       // this op might exhaust log space; wait for commit.
5086       sleep(&log, &log.lock);
5087     } else {
5088       log.outstanding += 1;
5089       release(&log.lock);
5090       break;
5091     }
5092   }
5093 }
5094 
5095 
5096 
5097 
5098 
5099 
5100 // called at the end of each FS system call.
5101 // commits if this was the last outstanding operation.
5102 void
5103 end_op(void)
5104 {
5105   int do_commit = 0;
5106 
5107   acquire(&log.lock);
5108   log.outstanding -= 1;
5109   if(log.committing)
5110     panic("log.committing");
5111   if(log.outstanding == 0){
5112     do_commit = 1;
5113     log.committing = 1;
5114   } else {
5115     // begin_op() may be waiting for log space,
5116     // and decrementing log.outstanding has decreased
5117     // the amount of reserved space.
5118     wakeup(&log);
5119   }
5120   release(&log.lock);
5121 
5122   if(do_commit){
5123     // call commit w/o holding locks, since not allowed
5124     // to sleep with locks.
5125     commit();
5126     acquire(&log.lock);
5127     log.committing = 0;
5128     wakeup(&log);
5129     release(&log.lock);
5130   }
5131 }
5132 
5133 // Copy modified blocks from cache to log.
5134 static void
5135 write_log(void)
5136 {
5137   int tail;
5138 
5139   for (tail = 0; tail < log.lh.n; tail++) {
5140     struct buf *to = bread(log.dev, log.start+tail+1); // log block
5141     struct buf *from = bread(log.dev, log.lh.block[tail]); // cache block
5142     memmove(to->data, from->data, BSIZE);
5143     bwrite(to);  // write the log
5144     brelse(from);
5145     brelse(to);
5146   }
5147 }
5148 
5149 
5150 static void
5151 commit()
5152 {
5153   if (log.lh.n > 0) {
5154     write_log();     // Write modified blocks from cache to log
5155     write_head();    // Write header to disk -- the real commit
5156     install_trans(); // Now install writes to home locations
5157     log.lh.n = 0;
5158     write_head();    // Erase the transaction from the log
5159   }
5160 }
5161 
5162 // Caller has modified b->data and is done with the buffer.
5163 // Record the block number and pin in the cache with B_DIRTY.
5164 // commit()/write_log() will do the disk write.
5165 //
5166 // log_write() replaces bwrite(); a typical use is:
5167 //   bp = bread(...)
5168 //   modify bp->data[]
5169 //   log_write(bp)
5170 //   brelse(bp)
5171 void
5172 log_write(struct buf *b)
5173 {
5174   int i;
5175 
5176   if (log.lh.n >= LOGSIZE || log.lh.n >= log.size - 1)
5177     panic("too big a transaction");
5178   if (log.outstanding < 1)
5179     panic("log_write outside of trans");
5180 
5181   acquire(&log.lock);
5182   for (i = 0; i < log.lh.n; i++) {
5183     if (log.lh.block[i] == b->blockno)   // log absorbtion
5184       break;
5185   }
5186   log.lh.block[i] = b->blockno;
5187   if (i == log.lh.n)
5188     log.lh.n++;
5189   b->flags |= B_DIRTY; // prevent eviction
5190   release(&log.lock);
5191 }
5192 
5193 
5194 
5195 
5196 
5197 
5198 
5199 
