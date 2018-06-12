2150 // Per-CPU state
2151 struct cpu {
2152   uchar apicid;                // Local APIC ID
2153   struct context *scheduler;   // swtch() here to enter scheduler
2154   //struct taskstate ts;         // Used by x86 to find stack for interrupt
2155   //struct segdesc gdt[NSEGS];   // x86 global descriptor table
2156   volatile uint started;       // Has the CPU started?
2157   int ncli;                    // Depth of pushcli nesting.
2158   int intena;                  // Were interrupts enabled before pushcli?
2159   struct proc *proc;           // The process running on this cpu or null
2160 };
2161 
2162 extern struct cpu cpus[NCPU];
2163 extern int ncpu;
2164 
2165 
2166 // Saved registers for kernel context switches.
2167 // Don't need to save all the segment registers (%cs, etc),
2168 // because they are constant across kernel contexts.
2169 // Don't need to save %eax, %ecx, %edx, because the
2170 // x86 convention is that the caller has saved them.
2171 // Contexts are stored at the bottom of the stack they
2172 // describe; the stack pointer is the address of the context.
2173 // The layout of the context matches the layout of the stack in swtch.S
2174 // at the "Switch stacks" comment. Switch doesn't save eip explicitly,
2175 // but it is on the stack and allocproc() manipulates it.
2176 struct context {
2177   //uint edi;
2178   //uint esi;
2179   //uint ebx;
2180   //uint ebp;
2181   //uint eip;
2182   uint s0;
2183   uint s1;
2184   uint s2;
2185   uint s3;
2186   uint s4;
2187   uint s5;
2188   uint s6;
2189   uint s7;
2190   uint s8;
2191   uint s9;
2192   uint s10;
2193   uint s11;
2194   uint ra;
2195 };
2196 
2197 
2198 
2199 
2200 enum procstate { UNUSED, EMBRYO, SLEEPING, RUNNABLE, RUNNING, ZOMBIE };
2201 
2202 // Per-process state
2203 struct proc {
2204   uint sz;                     // Size of process memory (bytes)
2205   pde_t* pgdir;                // Page table
2206   char *kstack;                // Bottom of kernel stack for this process
2207   enum procstate state;        // Process state
2208   int pid;                     // Process ID
2209   struct proc *parent;         // Parent process
2210   struct trapframe *tf;        // Trap frame for current syscall
2211   struct context *context;     // swtch() here to run process
2212   void *chan;                  // If non-zero, sleeping on chan
2213   int killed;                  // If non-zero, have been killed
2214   struct file *ofile[NOFILE];  // Open files
2215   struct inode *cwd;           // Current directory
2216   char name[16];               // Process name (debugging)
2217 };
2218 
2219 // Process memory is laid out contiguously, low addresses first:
2220 //   text
2221 //   original data and bss
2222 //   fixed-size stack
2223 //   expandable heap
2224 
2225 
2226 
2227 
2228 
2229 
2230 
2231 
2232 
2233 
2234 
2235 
2236 
2237 
2238 
2239 
2240 
2241 
2242 
2243 
2244 
2245 
2246 
2247 
2248 
2249 
