3050 // Physical memory allocator, intended to allocate
3051 // memory for user processes, kernel stacks, page table pages,
3052 // and pipe buffers. Allocates 4096-byte pages.
3053 
3054 #include "types.h"
3055 #include "defs.h"
3056 #include "param.h"
3057 #include "memlayout.h"
3058 #include "mmu.h"
3059 #include "spinlock.h"
3060 
3061 void freerange(void *vstart, void *vend);
3062 extern char end[]; // first address after kernel loaded from ELF file
3063                    // defined by the kernel linker script in kernel.ld
3064 
3065 struct run {
3066   struct run *next;
3067 };
3068 
3069 struct {
3070   struct spinlock lock;
3071   int use_lock;
3072   struct run *freelist;
3073 } kmem;
3074 
3075 // Initialization happens in two phases.
3076 // 1. main() calls kinit1() while still using entrypgdir to place just
3077 // the pages mapped by entrypgdir on free list.
3078 // 2. main() calls kinit2() with the rest of the physical pages
3079 // after installing a full page table that maps them on all cores.
3080 void
3081 kinit1(void *vstart, void *vend)
3082 {
3083   initlock(&kmem.lock, "kmem");
3084   kmem.use_lock = 0;
3085   //vstart - vend の指す物理メモリを初期化？
3086   freerange(vstart, vend);
3087 }
3088 
3089 void
3090 kinit2(void *vstart, void *vend)
3091 {
3092   freerange(vstart, vend);
3093   kmem.use_lock = 1;
3094 }
3095 
3096 
3097 
3098 
3099 
3100 void
3101 freerange(void *vstart, void *vend)
3102 {
3103   char *p;
3104   p = (char*)PGROUNDUP((uint)vstart);
3105   for(; p + PGSIZE <= (char*)vend; p += PGSIZE)
3106     kfree(p);
3107 }
3108 
3109 // Free the page of physical memory pointed at by v,
3110 // which normally should have been returned by a
3111 // call to kalloc().  (The exception is when
3112 // initializing the allocator; see kinit above.)
3113 void
3114 kfree(char *v)
3115 {
3116   struct run *r;
3117 
3118   if((uint)v % PGSIZE || v < end || V2P(v) >= PHYSTOP)
3119     panic("kfree");
3120 
3121   // Fill with junk to catch dangling refs.
3122   memset(v, 1, PGSIZE);
3123 
3124   if(kmem.use_lock)
3125     acquire(&kmem.lock);
3126   r = (struct run*)v;
3127   r->next = kmem.freelist;
3128   kmem.freelist = r;
3129   if(kmem.use_lock)
3130     release(&kmem.lock);
3131 }
3132 
3133 // Allocate one 4096-byte page of physical memory.
3134 // Returns a pointer that the kernel can use.
3135 // Returns 0 if the memory cannot be allocated.
3136 char*
3137 kalloc(void)
3138 {
3139   struct run *r;
3140 
3141   if(kmem.use_lock)
3142     acquire(&kmem.lock);
3143   r = kmem.freelist;
3144   if(r)
3145     kmem.freelist = r->next;
3146 
3147   if(kmem.use_lock)
3148     release(&kmem.lock);
3149 
3150   return (char*)r;
3151 }
3152 
3153 
3154 
3155 
3156 
3157 
3158 
3159 
3160 
3161 
3162 
3163 
3164 
3165 
3166 
3167 
3168 
3169 
3170 
3171 
3172 
3173 
3174 
3175 
3176 
3177 
3178 
3179 
3180 
3181 
3182 
3183 
3184 
3185 
3186 
3187 
3188 
3189 
3190 
3191 
3192 
3193 
3194 
3195 
3196 
3197 
3198 
3199 
