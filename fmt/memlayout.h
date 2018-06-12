0200 // Memory layout
0201 
0202 #define EXTMEM  0x100000            // Start of extended memory
0203 //#define PHYSTOP 0x8000000           // Top physical memory
0204 #define PHYSTOP 0x800000           // Top physical memory
0205 //#define PHYSTOP 0x400000           // Top physical memory	 4MB
0206 //#define PHYSTOP 0x4000000           // Top physical memory 64MB
0207 //#define DEVSPACE 0xFE000000         // Other devices are at high addresses
0208 #define DEVSPACE 0xFFFFF000         // Other devices are at high addresses
0209 
0210 // Key addresses for address space layout (see kmap in vm.c for layout)
0211 #define KERNBASE 0x80000000         // First kernel virtual address
0212 #define KERNLINK (KERNBASE+EXTMEM)  // Address where kernel is linked
0213 
0214 #define V2P(a) (((uint) (a)) - KERNBASE)
0215 #define P2V(a) (((void *) (a)) + KERNBASE)
0216 
0217 #define V2P_WO(x) ((x) - KERNBASE)    // same as V2P, but without casts
0218 #define P2V_WO(x) ((x) + KERNBASE)    // same as P2V, but without casts
0219 
0220 
0221 
0222 
0223 
0224 
0225 
0226 
0227 
0228 
0229 
0230 
0231 
0232 
0233 
0234 
0235 
0236 
0237 
0238 
0239 
0240 
0241 
0242 
0243 
0244 
0245 
0246 
0247 
0248 
0249 
