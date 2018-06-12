2250 #include "types.h"
2251 #include "defs.h"
2252 #include "param.h"
2253 #include "memlayout.h"
2254 #include "mmu.h"
2255 #include "proc.h"
2256 #include "spinlock.h"
2257 #include "riscv.h"
2258 
2259 struct {
2260   struct spinlock lock;
2261   struct proc proc[NPROC];
2262 } ptable;
2263 
2264 static struct proc *initproc;
2265 
2266 int nextpid = 1;
2267 extern void forkret(void);
2268 extern void trapret(void);
2269 
2270 static void wakeup1(void *chan);
2271 
2272 void
2273 pinit(void)
2274 {
2275   initlock(&ptable.lock, "ptable");
2276 }
2277 
2278 // Must be called with interrupts disabled
2279 int
2280 cpuid() {
2281   return mycpu()-cpus;
2282 }
2283 
2284 // Must be called with interrupts disabled to avoid the caller being
2285 // rescheduled between reading lapicid and running through the loop.
2286 struct cpu*
2287 mycpu(void)
2288 {
2289   int apicid, i;
2290 
2291   if(read_status()&S_MIE)
2292     panic("mycpu called with interrupts enabled\n");
2293 
2294   apicid = lapicid();
2295   // APIC IDs are not guaranteed to be contiguous. Maybe we should have
2296   // a reverse map, or reserve a register to store &cpus[i].
2297   //for (i = 0; i < ncpu; ++i) {
2298   for (i = 0; i < 1; ++i) {
2299     if (cpus[i].apicid == apicid)
2300       return &cpus[i];
2301   }
2302   panic("unknown apicid\n");
2303 }
2304 
2305 // Disable interrupts so that we are not rescheduled
2306 // while reading proc from the cpu structure
2307 struct proc*
2308 myproc(void) {
2309   struct cpu *c;
2310   struct proc *p;
2311   pushcli();
2312   c = mycpu();
2313   p = c->proc;
2314   popcli();
2315   return p;
2316 }
2317 
2318 
2319 
2320 
2321 
2322 
2323 
2324 
2325 
2326 
2327 
2328 
2329 
2330 
2331 
2332 
2333 
2334 
2335 
2336 
2337 
2338 
2339 
2340 
2341 
2342 
2343 
2344 
2345 
2346 
2347 
2348 
2349 
2350 // Look in the process table for an UNUSED proc.
2351 // If found, change state to EMBRYO and initialize
2352 // state required to run in the kernel.
2353 // Otherwise return 0.
2354 static struct proc*
2355 allocproc(void)
2356 {
2357   struct proc *p;
2358   char *sp;
2359 
2360   acquire(&ptable.lock);
2361 
2362   for(p = ptable.proc; p < &ptable.proc[NPROC]; p++)
2363     if(p->state == UNUSED)
2364       goto found;
2365 
2366   release(&ptable.lock);
2367   return 0;
2368 
2369 found:
2370   p->state = EMBRYO;
2371   p->pid = nextpid++;
2372 
2373   release(&ptable.lock);
2374 
2375   // Allocate kernel stack.
2376   if((p->kstack = kalloc()) == 0){
2377     p->state = UNUSED;
2378     return 0;
2379   }
2380   sp = p->kstack + KSTACKSIZE;
2381 
2382   // Leave room for trap frame.
2383   sp -= sizeof *p->tf;
2384   p->tf = (struct trapframe*)sp;
2385 
2386   // Set up new context to start executing at forkret,
2387   // which returns to trapret.
2388   sp -= 4;
2389   *(uint*)sp = (uint)trapret;
2390 
2391   sp -= sizeof *p->context;
2392   p->context = (struct context*)sp;
2393   memset(p->context, 0, sizeof *p->context);
2394   p->context->ra = (uint)forkret;
2395 
2396   return p;
2397 }
2398 
2399 
2400 // Set up first user process.
2401 void
2402 userinit(void)
2403 {
2404   struct proc *p;
2405   extern char _binary_initcode_start[], _binary_initcode_size[];
2406 
2407   p = allocproc();
2408 
2409   initproc = p;
2410   if((p->pgdir = setupkvm()) == 0)
2411     panic("userinit: out of memory?");
2412   inituvm(p->pgdir, _binary_initcode_start, (int)_binary_initcode_size);
2413   p->sz = PGSIZE;
2414   memset(p->tf, 0, sizeof(*p->tf));
2415 
2416   //p->tf->cs = (SEG_UCODE << 3) | DPL_USER;
2417   //p->tf->ds = (SEG_UDATA << 3) | DPL_USER;
2418   //p->tf->es = p->tf->ds;
2419   //p->tf->ss = p->tf->ds;
2420   p->tf->mstatus = read_status() | S_MIE;
2421   p->tf->sp = PGSIZE;
2422   p->tf->mepc = 0;  // beginning of initcode.S
2423 
2424   safestrcpy(p->name, "initcode", sizeof(p->name));
2425   p->cwd = namei("/");
2426 
2427   // this assignment to p->state lets other cores
2428   // run this process. the acquire forces the above
2429   // writes to be visible, and the lock is also needed
2430   // because the assignment might not be atomic.
2431   acquire(&ptable.lock);
2432 
2433   p->state = RUNNABLE;
2434 
2435   release(&ptable.lock);
2436 }
2437 
2438 
2439 
2440 
2441 
2442 
2443 
2444 
2445 
2446 
2447 
2448 
2449 
2450 // Grow current process's memory by n bytes.
2451 // Return 0 on success, -1 on failure.
2452 int
2453 growproc(int n)
2454 {
2455   uint sz;
2456   struct proc *curproc = myproc();
2457 
2458   sz = curproc->sz;
2459   if(n > 0){
2460     if((sz = allocuvm(curproc->pgdir, sz, sz + n)) == 0)
2461       return -1;
2462   } else if(n < 0){
2463     if((sz = deallocuvm(curproc->pgdir, sz, sz + n)) == 0)
2464       return -1;
2465   }
2466   curproc->sz = sz;
2467   switchuvm(curproc);
2468   return 0;
2469 }
2470 
2471 // Create a new process copying p as the parent.
2472 // Sets up stack to return as if from system call.
2473 // Caller must set state of returned proc to RUNNABLE.
2474 int
2475 fork(void)
2476 {
2477   int i, pid;
2478   struct proc *np;
2479   struct proc *curproc = myproc();
2480 
2481   // Allocate process.
2482   if((np = allocproc()) == 0){
2483     return -1;
2484   }
2485 
2486   // Copy process state from proc.
2487   if((np->pgdir = copyuvm(curproc->pgdir, curproc->sz)) == 0){
2488     kfree(np->kstack);
2489     np->kstack = 0;
2490     np->state = UNUSED;
2491     return -1;
2492   }
2493   np->sz = curproc->sz;
2494   np->parent = curproc;
2495   *np->tf = *curproc->tf;
2496 
2497   // Clear %eax so that fork returns 0 in the child.
2498   np->tf->a0 = 0;
2499 
2500   for(i = 0; i < NOFILE; i++)
2501     if(curproc->ofile[i])
2502       np->ofile[i] = filedup(curproc->ofile[i]);
2503   np->cwd = idup(curproc->cwd);
2504 
2505   safestrcpy(np->name, curproc->name, sizeof(curproc->name));
2506 
2507   pid = np->pid;
2508 
2509   acquire(&ptable.lock);
2510 
2511   np->state = RUNNABLE;
2512 
2513   release(&ptable.lock);
2514 
2515   return pid;
2516 }
2517 
2518 // Exit the current process.  Does not return.
2519 // An exited process remains in the zombie state
2520 // until its parent calls wait() to find out it exited.
2521 void
2522 exit(void)
2523 {
2524   struct proc *curproc = myproc();
2525   struct proc *p;
2526   int fd;
2527 
2528   if(curproc == initproc)
2529     panic("init exiting");
2530 
2531   // Close all open files.
2532   for(fd = 0; fd < NOFILE; fd++){
2533     if(curproc->ofile[fd]){
2534       fileclose(curproc->ofile[fd]);
2535       curproc->ofile[fd] = 0;
2536     }
2537   }
2538 
2539   begin_op();
2540   iput(curproc->cwd);
2541   end_op();
2542   curproc->cwd = 0;
2543 
2544   acquire(&ptable.lock);
2545 
2546   // Parent might be sleeping in wait().
2547   wakeup1(curproc->parent);
2548 
2549 
2550   // Pass abandoned children to init.
2551   for(p = ptable.proc; p < &ptable.proc[NPROC]; p++){
2552     if(p->parent == curproc){
2553       p->parent = initproc;
2554       if(p->state == ZOMBIE)
2555         wakeup1(initproc);
2556     }
2557   }
2558 
2559   // Jump into the scheduler, never to return.
2560   curproc->state = ZOMBIE;
2561   sched();
2562   panic("zombie exit");
2563 }
2564 
2565 // Wait for a child process to exit and return its pid.
2566 // Return -1 if this process has no children.
2567 int
2568 wait(void)
2569 {
2570   struct proc *p;
2571   int havekids, pid;
2572   struct proc *curproc = myproc();
2573 
2574   acquire(&ptable.lock);
2575   for(;;){
2576     // Scan through table looking for exited children.
2577     havekids = 0;
2578     for(p = ptable.proc; p < &ptable.proc[NPROC]; p++){
2579       if(p->parent != curproc)
2580         continue;
2581       havekids = 1;
2582       if(p->state == ZOMBIE){
2583         // Found one.
2584         pid = p->pid;
2585         kfree(p->kstack);
2586         p->kstack = 0;
2587         freevm(p->pgdir);
2588         p->pid = 0;
2589         p->parent = 0;
2590         p->name[0] = 0;
2591         p->killed = 0;
2592         p->state = UNUSED;
2593         release(&ptable.lock);
2594         return pid;
2595       }
2596     }
2597 
2598 
2599 
2600     // No point waiting if we don't have any children.
2601     if(!havekids || curproc->killed){
2602       release(&ptable.lock);
2603       return -1;
2604     }
2605 
2606     // Wait for children to exit.  (See wakeup1 call in proc_exit.)
2607     sleep(curproc, &ptable.lock);  //DOC: wait-sleep
2608   }
2609 }
2610 
2611 
2612 
2613 
2614 
2615 
2616 
2617 
2618 
2619 
2620 
2621 
2622 
2623 
2624 
2625 
2626 
2627 
2628 
2629 
2630 
2631 
2632 
2633 
2634 
2635 
2636 
2637 
2638 
2639 
2640 
2641 
2642 
2643 
2644 
2645 
2646 
2647 
2648 
2649 
2650 // Per-CPU process scheduler.
2651 // Each CPU calls scheduler() after setting itself up.
2652 // Scheduler never returns.  It loops, doing:
2653 //  - choose a process to run
2654 //  - swtch to start running that process
2655 //  - eventually that process transfers control
2656 //      via swtch back to the scheduler.
2657 void
2658 scheduler(void)
2659 {
2660   struct proc *p;
2661   struct cpu *c = mycpu();
2662   c->proc = 0;
2663 
2664   for(;;){
2665     // Enable interrupts on this processor.
2666     sti();
2667 
2668     // Loop over process table looking for process to run.
2669     acquire(&ptable.lock);
2670     for(p = ptable.proc; p < &ptable.proc[NPROC]; p++){
2671       if(p->state != RUNNABLE)
2672         continue;
2673 
2674       // Switch to chosen process.  It is the process's job
2675       // to release ptable.lock and then reacquire it
2676       // before jumping back to us.
2677       c->proc = p;
2678 	  //??
2679       switchuvm(p);
2680       p->state = RUNNING;
2681 
2682       swtch(&(c->scheduler), p->context);
2683       switchkvm();
2684 
2685       // Process is done running for now.
2686       // It should have changed its p->state before coming back.
2687       c->proc = 0;
2688     }
2689     release(&ptable.lock);
2690 
2691   }
2692 }
2693 
2694 
2695 
2696 
2697 
2698 
2699 
2700 // Enter scheduler.  Must hold only ptable.lock
2701 // and have changed proc->state. Saves and restores
2702 // intena because intena is a property of this
2703 // kernel thread, not this CPU. It should
2704 // be proc->intena and proc->ncli, but that would
2705 // break in the few places where a lock is held but
2706 // there's no process.
2707 void
2708 sched(void)
2709 {
2710   int intena;
2711   struct proc *p = myproc();
2712 
2713   if(!holding(&ptable.lock))
2714     panic("sched ptable.lock");
2715   if(mycpu()->ncli != 1)
2716     panic("sched locks");
2717   if(p->state == RUNNING)
2718     panic("sched running");
2719   if(read_status()&S_MIE)
2720     panic("sched interruptible");
2721   intena = mycpu()->intena;
2722   swtch(&p->context, mycpu()->scheduler);
2723   mycpu()->intena = intena;
2724 }
2725 
2726 // Give up the CPU for one scheduling round.
2727 void
2728 yield(void)
2729 {
2730   acquire(&ptable.lock);  //DOC: yieldlock
2731   myproc()->state = RUNNABLE;
2732   sched();
2733   release(&ptable.lock);
2734 }
2735 
2736 
2737 
2738 
2739 
2740 
2741 
2742 
2743 
2744 
2745 
2746 
2747 
2748 
2749 
2750 // A fork child's very first scheduling by scheduler()
2751 // will swtch here.  "Return" to user space.
2752 void
2753 forkret(void)
2754 {
2755   static int first = 1;
2756   // Still holding ptable.lock from scheduler.
2757   release(&ptable.lock);
2758 
2759   if (first) {
2760     // Some initialization functions must be run in the context
2761     // of a regular process (e.g., they call sleep), and thus cannot
2762     // be run from main().
2763     first = 0;
2764     iinit(ROOTDEV);
2765     initlog(ROOTDEV);
2766   }
2767 
2768   // Return to "caller", actually trapret (see allocproc).
2769 }
2770 
2771 // Atomically release lock and sleep on chan.
2772 // Reacquires lock when awakened.
2773 void
2774 sleep(void *chan, struct spinlock *lk)
2775 {
2776   struct proc *p = myproc();
2777 
2778   if(p == 0)
2779     panic("sleep");
2780 
2781   if(lk == 0)
2782     panic("sleep without lk");
2783 
2784   // Must acquire ptable.lock in order to
2785   // change p->state and then call sched.
2786   // Once we hold ptable.lock, we can be
2787   // guaranteed that we won't miss any wakeup
2788   // (wakeup runs with ptable.lock locked),
2789   // so it's okay to release lk.
2790   if(lk != &ptable.lock){  //DOC: sleeplock0
2791     acquire(&ptable.lock);  //DOC: sleeplock1
2792     release(lk);
2793   }
2794   // Go to sleep.
2795   p->chan = chan;
2796   p->state = SLEEPING;
2797 
2798   sched();
2799 
2800   // Tidy up.
2801   p->chan = 0;
2802 
2803   // Reacquire original lock.
2804   if(lk != &ptable.lock){  //DOC: sleeplock2
2805     release(&ptable.lock);
2806     acquire(lk);
2807   }
2808 }
2809 
2810 
2811 
2812 
2813 
2814 
2815 
2816 
2817 
2818 
2819 
2820 
2821 
2822 
2823 
2824 
2825 
2826 
2827 
2828 
2829 
2830 
2831 
2832 
2833 
2834 
2835 
2836 
2837 
2838 
2839 
2840 
2841 
2842 
2843 
2844 
2845 
2846 
2847 
2848 
2849 
2850 // Wake up all processes sleeping on chan.
2851 // The ptable lock must be held.
2852 static void
2853 wakeup1(void *chan)
2854 {
2855   struct proc *p;
2856 
2857   for(p = ptable.proc; p < &ptable.proc[NPROC]; p++)
2858     if(p->state == SLEEPING && p->chan == chan)
2859       p->state = RUNNABLE;
2860 }
2861 
2862 // Wake up all processes sleeping on chan.
2863 void
2864 wakeup(void *chan)
2865 {
2866   acquire(&ptable.lock);
2867   wakeup1(chan);
2868   release(&ptable.lock);
2869 }
2870 
2871 // Kill the process with the given pid.
2872 // Process won't exit until it returns
2873 // to user space (see trap in trap.c).
2874 int
2875 kill(int pid)
2876 {
2877   struct proc *p;
2878 
2879   acquire(&ptable.lock);
2880   for(p = ptable.proc; p < &ptable.proc[NPROC]; p++){
2881     if(p->pid == pid){
2882       p->killed = 1;
2883       // Wake process from sleep if necessary.
2884       if(p->state == SLEEPING)
2885         p->state = RUNNABLE;
2886       release(&ptable.lock);
2887       return 0;
2888     }
2889   }
2890   release(&ptable.lock);
2891   return -1;
2892 }
2893 
2894 
2895 
2896 
2897 
2898 
2899 
2900 // Print a process listing to console.  For debugging.
2901 // Runs when user types ^P on console.
2902 // No lock to avoid wedging a stuck machine further.
2903 void
2904 procdump(void)
2905 {
2906   static char *states[] = {
2907   [UNUSED]    "unused",
2908   [EMBRYO]    "embryo",
2909   [SLEEPING]  "sleep ",
2910   [RUNNABLE]  "runble",
2911   [RUNNING]   "run   ",
2912   [ZOMBIE]    "zombie"
2913   };
2914   int i;
2915   struct proc *p;
2916   char *state;
2917   uint pc[10];
2918 
2919   for(p = ptable.proc; p < &ptable.proc[NPROC]; p++){
2920     if(p->state == UNUSED)
2921       continue;
2922     if(p->state >= 0 && p->state < NELEM(states) && states[p->state])
2923       state = states[p->state];
2924     else
2925       state = "???";
2926     cprintf("%d %s %s", p->pid, state, p->name);
2927     if(p->state == SLEEPING){
2928       getcallerpcs((uint*)p->context->ra, pc);
2929       for(i=0; i<10 && pc[i] != 0; i++)
2930         cprintf(" %p", pc[i]);
2931     }
2932     cprintf("\n");
2933   }
2934 }
2935 
2936 
2937 
2938 
2939 
2940 
2941 
2942 
2943 
2944 
2945 
2946 
2947 
2948 
2949 
