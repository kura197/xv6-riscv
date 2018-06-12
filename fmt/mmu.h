0500 // This file contains definitions for the
0501 // x86 memory management unit (MMU).
0502 
0503 /*
0504 // Eflags register
0505 #define FL_CF           0x00000001      // Carry Flag
0506 #define FL_PF           0x00000004      // Parity Flag
0507 #define FL_AF           0x00000010      // Auxiliary carry Flag
0508 #define FL_ZF           0x00000040      // Zero Flag
0509 #define FL_SF           0x00000080      // Sign Flag
0510 #define FL_TF           0x00000100      // Trap Flag
0511 #define FL_IF           0x00000200      // Interrupt Enable
0512 #define FL_DF           0x00000400      // Direction Flag
0513 #define FL_OF           0x00000800      // Overflow Flag
0514 #define FL_IOPL_MASK    0x00003000      // I/O Privilege Level bitmask
0515 #define FL_IOPL_0       0x00000000      //   IOPL == 0
0516 #define FL_IOPL_1       0x00001000      //   IOPL == 1
0517 #define FL_IOPL_2       0x00002000      //   IOPL == 2
0518 #define FL_IOPL_3       0x00003000      //   IOPL == 3
0519 #define FL_NT           0x00004000      // Nested Task
0520 #define FL_RF           0x00010000      // Resume Flag
0521 #define FL_VM           0x00020000      // Virtual 8086 mode
0522 #define FL_AC           0x00040000      // Alignment Check
0523 #define FL_VIF          0x00080000      // Virtual Interrupt Flag
0524 #define FL_VIP          0x00100000      // Virtual Interrupt Pending
0525 #define FL_ID           0x00200000      // ID flag
0526 */
0527 
0528 //status register
0529 #define S_MIE	0x00000008
0530 #define S_MPIE	0x00000080
0531 
0532 // Control Register flags
0533 #define CR0_PE          0x00000001      // Protection Enable
0534 #define CR0_MP          0x00000002      // Monitor coProcessor
0535 #define CR0_EM          0x00000004      // Emulation
0536 #define CR0_TS          0x00000008      // Task Switched
0537 #define CR0_ET          0x00000010      // Extension Type
0538 #define CR0_NE          0x00000020      // Numeric Errror
0539 #define CR0_WP          0x00010000      // Write Protect
0540 #define CR0_AM          0x00040000      // Alignment Mask
0541 #define CR0_NW          0x20000000      // Not Writethrough
0542 #define CR0_CD          0x40000000      // Cache Disable
0543 #define CR0_PG          0x80000000      // Paging
0544 
0545 #define CR4_PSE         0x00000010      // Page size extension
0546 
0547 
0548 
0549 
0550 // various segment selectors.
0551 #define SEG_KCODE 1  // kernel code
0552 #define SEG_KDATA 2  // kernel data+stack
0553 #define SEG_UCODE 3  // user code
0554 #define SEG_UDATA 4  // user data+stack
0555 #define SEG_TSS   5  // this process's task state
0556 
0557 // cpu->gdt[NSEGS] holds the above segments.
0558 #define NSEGS     6
0559 
0560 #ifndef __ASSEMBLER__
0561 // Segment Descriptor
0562 struct segdesc {
0563   uint lim_15_0 : 16;  // Low bits of segment limit
0564   uint base_15_0 : 16; // Low bits of segment base address
0565   uint base_23_16 : 8; // Middle bits of segment base address
0566   uint type : 4;       // Segment type (see STS_ constants)
0567   uint s : 1;          // 0 = system, 1 = application
0568   uint dpl : 2;        // Descriptor Privilege Level
0569   uint p : 1;          // Present
0570   uint lim_19_16 : 4;  // High bits of segment limit
0571   uint avl : 1;        // Unused (available for software use)
0572   uint rsv1 : 1;       // Reserved
0573   uint db : 1;         // 0 = 16-bit segment, 1 = 32-bit segment
0574   uint g : 1;          // Granularity: limit scaled by 4K when set
0575   uint base_31_24 : 8; // High bits of segment base address
0576 };
0577 
0578 // Normal segment
0579 #define SEG(type, base, lim, dpl) (struct segdesc)    \
0580 { ((lim) >> 12) & 0xffff, (uint)(base) & 0xffff,      \
0581   ((uint)(base) >> 16) & 0xff, type, 1, dpl, 1,       \
0582   (uint)(lim) >> 28, 0, 0, 1, 1, (uint)(base) >> 24 }
0583 #define SEG16(type, base, lim, dpl) (struct segdesc)  \
0584 { (lim) & 0xffff, (uint)(base) & 0xffff,              \
0585   ((uint)(base) >> 16) & 0xff, type, 1, dpl, 1,       \
0586   (uint)(lim) >> 16, 0, 0, 1, 0, (uint)(base) >> 24 }
0587 #endif
0588 
0589 #define DPL_USER    0x3     // User DPL
0590 
0591 // Application segment type bits
0592 #define STA_X       0x8     // Executable segment
0593 #define STA_E       0x4     // Expand down (non-executable segments)
0594 #define STA_C       0x4     // Conforming code segment (executable only)
0595 #define STA_W       0x2     // Writeable (non-executable segments)
0596 #define STA_R       0x2     // Readable (executable segments)
0597 #define STA_A       0x1     // Accessed
0598 
0599 
0600 // System segment type bits
0601 #define STS_T16A    0x1     // Available 16-bit TSS
0602 #define STS_LDT     0x2     // Local Descriptor Table
0603 #define STS_T16B    0x3     // Busy 16-bit TSS
0604 #define STS_CG16    0x4     // 16-bit Call Gate
0605 #define STS_TG      0x5     // Task Gate / Coum Transmitions
0606 #define STS_IG16    0x6     // 16-bit Interrupt Gate
0607 #define STS_TG16    0x7     // 16-bit Trap Gate
0608 #define STS_T32A    0x9     // Available 32-bit TSS
0609 #define STS_T32B    0xB     // Busy 32-bit TSS
0610 #define STS_CG32    0xC     // 32-bit Call Gate
0611 #define STS_IG32    0xE     // 32-bit Interrupt Gate
0612 #define STS_TG32    0xF     // 32-bit Trap Gate
0613 
0614 // A virtual address 'la' has a three-part structure as follows:
0615 //
0616 // +--------10------+-------10-------+---------12----------+
0617 // | Page Directory |   Page Table   | Offset within Page  |
0618 // |      Index     |      Index     |                     |
0619 // +----------------+----------------+---------------------+
0620 //  \--- PDX(va) --/ \--- PTX(va) --/
0621 
0622 // page directory index
0623 #define PDX(va)         (((uint)(va) >> PDXSHIFT) & 0x3FF)
0624 
0625 // page table index
0626 #define PTX(va)         (((uint)(va) >> PTXSHIFT) & 0x3FF)
0627 
0628 // construct virtual address from indexes and offset
0629 #define PGADDR(d, t, o) ((uint)((d) << PDXSHIFT | (t) << PTXSHIFT | (o)))
0630 
0631 // Page directory and page table constants.
0632 #define NPDENTRIES      1024    // # directory entries per page directory
0633 #define NPTENTRIES      1024    // # PTEs per page table
0634 #define PGSIZE          4096    // bytes mapped by a page
0635 
0636 #define PGSHIFT         12      // log2(PGSIZE)
0637 #define PTXSHIFT        12      // offset of PTX in a linear address
0638 #define PDXSHIFT        22      // offset of PDX in a linear address
0639 
0640 #define PGROUNDUP(sz)  (((sz)+PGSIZE-1) & ~(PGSIZE-1))
0641 #define PGROUNDDOWN(a) (((a)) & ~(PGSIZE-1))
0642 
0643 // Page table/directory entry flags.
0644 //#define PTE_P           0x001   // Present
0645 //#define PTE_W           0x002   // Writeable
0646 //#define PTE_U           0x004   // User
0647 //#define PTE_PWT         0x008   // Write-Through
0648 //#define PTE_PCD         0x010   // Cache-Disable
0649 //#define PTE_A           0x020   // Accessed
0650 //#define PTE_D           0x040   // Dirty
0651 //#define PTE_PS          0x080   // Page Size
0652 //#define PTE_MBZ         0x180   // Bits must be zero
0653 #define PTE_V           0x001   // Present
0654 #define PTE_R           0x002   // Writeable
0655 #define PTE_W           0x004   // User
0656 #define PTE_X         0x008   // Write-Through
0657 #define PTE_U         0x010   // Cache-Disable
0658 #define PTE_G           0x020   // Accessed
0659 #define PTE_A           0x040   // Dirty
0660 #define PTE_D          0x080   // Page Size
0661 //#define PTE_MBZ         0x180   // Bits must be zero
0662 
0663 // Address in page table or page directory entry
0664 //#define PTE_ADDR(pte)   ((uint)(pte) & ~0xFFF)
0665 //#define PTE_FLAGS(pte)  ((uint)(pte) &  0xFFF)
0666 #define PTE_ADDR(pte)   ((uint)(pte) >> 10) * PGSIZE
0667 #define PTE_FLAGS(pte)  ((uint)(pte) &  0xFF)
0668 
0669 #ifndef __ASSEMBLER__
0670 typedef uint pte_t;
0671 
0672 // Task state segment format
0673 struct taskstate {
0674   uint link;         // Old ts selector
0675   uint esp0;         // Stack pointers and segment selectors
0676   ushort ss0;        //   after an increase in privilege level
0677   ushort padding1;
0678   uint *esp1;
0679   ushort ss1;
0680   ushort padding2;
0681   uint *esp2;
0682   ushort ss2;
0683   ushort padding3;
0684   void *cr3;         // Page directory base
0685   uint *eip;         // Saved state from last task switch
0686   uint eflags;
0687   uint eax;          // More saved state (registers)
0688   uint ecx;
0689   uint edx;
0690   uint ebx;
0691   uint *esp;
0692   uint *ebp;
0693   uint esi;
0694   uint edi;
0695   ushort es;         // Even more saved state (segment selectors)
0696   ushort padding4;
0697   ushort cs;
0698   ushort padding5;
0699   ushort ss;
0700   ushort padding6;
0701   ushort ds;
0702   ushort padding7;
0703   ushort fs;
0704   ushort padding8;
0705   ushort gs;
0706   ushort padding9;
0707   ushort ldt;
0708   ushort padding10;
0709   ushort t;          // Trap on task switch
0710   ushort iomb;       // I/O map base address
0711 };
0712 
0713 
0714 // Gate descriptors for interrupts and traps
0715 /*
0716 struct gatedesc {
0717   uint off_15_0 : 16;   // low 16 bits of offset in segment
0718   uint cs : 16;         // code segment selector
0719   uint args : 5;        // # args, 0 for interrupt/trap gates
0720   uint rsv1 : 3;        // reserved(should be zero I guess)
0721   uint type : 4;        // type(STS_{TG,IG32,TG32})
0722   uint s : 1;           // must be 0 (system)
0723   uint dpl : 2;         // descriptor(meaning new) privilege level
0724   uint p : 1;           // Present
0725   uint off_31_16 : 16;  // high bits of offset in segment
0726 };
0727 */
0728 
0729 // Set up a normal interrupt/trap gate descriptor.
0730 // - istrap: 1 for a trap (= exception) gate, 0 for an interrupt gate.
0731 //   interrupt gate clears FL_IF, trap gate leaves FL_IF alone
0732 // - sel: Code segment selector for interrupt/trap handler
0733 // - off: Offset in code segment for interrupt/trap handler
0734 // - dpl: Descriptor Privilege Level -
0735 //        the privilege level required for software to invoke
0736 //        this interrupt/trap gate explicitly using an int instruction.
0737 /*
0738 #define SETGATE(gate, istrap, sel, off, d)                \
0739 {                                                         \
0740   (gate).off_15_0 = (uint)(off) & 0xffff;                \
0741   (gate).cs = (sel);                                      \
0742   (gate).args = 0;                                        \
0743   (gate).rsv1 = 0;                                        \
0744   (gate).type = (istrap) ? STS_TG32 : STS_IG32;           \
0745   (gate).s = 0;                                           \
0746   (gate).dpl = (d);                                       \
0747   (gate).p = 1;                                           \
0748   (gate).off_31_16 = (uint)(off) >> 16;                  \
0749 }
0750 */
0751 #endif
0752 
0753 
0754 
0755 
0756 
0757 
0758 
0759 
0760 
0761 
0762 
0763 
0764 
0765 
0766 
0767 
0768 
0769 
0770 
0771 
0772 
0773 
0774 
0775 
0776 
0777 
0778 
0779 
0780 
0781 
0782 
0783 
0784 
0785 
0786 
0787 
0788 
0789 
0790 
0791 
0792 
0793 
0794 
0795 
0796 
0797 
0798 
0799 
