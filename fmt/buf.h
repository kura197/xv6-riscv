4100 struct buf {
4101   int flags;
4102   uint dev;
4103   uint blockno;
4104   struct sleeplock lock;
4105   uint refcnt;
4106   struct buf *prev; // LRU cache list
4107   struct buf *next;
4108   struct buf *qnext; // disk queue
4109   uchar data[BSIZE];
4110 };
4111 #define B_VALID 0x2  // buffer has been read from disk
4112 #define B_DIRTY 0x4  // buffer needs to be written to disk
4113 
4114 
4115 
4116 
4117 
4118 
4119 
4120 
4121 
4122 
4123 
4124 
4125 
4126 
4127 
4128 
4129 
4130 
4131 
4132 
4133 
4134 
4135 
4136 
4137 
4138 
4139 
4140 
4141 
4142 
4143 
4144 
4145 
4146 
4147 
4148 
4149 
