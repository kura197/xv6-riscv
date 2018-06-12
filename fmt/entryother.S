0950 #include "asm.h"
0951 #include "memlayout.h"
0952 #include "mmu.h"
0953 
0954 # Each non-boot CPU ("AP") is started up in response to a STARTUP
0955 # IPI from the boot CPU.  Section B.4.2 of the Multi-Processor
0956 # Specification says that the AP will start in real mode with CS:IP
0957 # set to XY00:0000, where XY is an 8-bit value sent with the
0958 # STARTUP. Thus this code must start at a 4096-byte boundary.
0959 #
0960 # Because this code sets DS to zero, it must sit
0961 # at an address in the low 2^16 bytes.
0962 #
0963 # Startothers (in main.c) sends the STARTUPs one at a time.
0964 # It copies this code (start) at 0x7000.  It puts the address of
0965 # a newly allocated per-core stack in start-4,the address of the
0966 # place to jump to (mpenter) in start-8, and the physical address
0967 # of entrypgdir in start-12.
0968 #
0969 # This code combines elements of bootasm.S and entry.S.
0970 
0971 .code16
0972 .globl start
0973 start:
0974   cli
0975 
0976   # Zero data segment registers DS, ES, and SS.
0977   xorw    %ax,%ax
0978   movw    %ax,%ds
0979   movw    %ax,%es
0980   movw    %ax,%ss
0981 
0982   # Switch from real to protected mode.  Use a bootstrap GDT that makes
0983   # virtual addresses map directly to physical addresses so that the
0984   # effective memory map doesn't change during the transition.
0985   lgdt    gdtdesc
0986   movl    %cr0, %eax
0987   orl     $CR0_PE, %eax
0988   movl    %eax, %cr0
0989 
0990   # Complete the transition to 32-bit protected mode by using a long jmp
0991   # to reload %cs and %eip.  The segment descriptors are set up with no
0992   # translation, so that the mapping is still the identity mapping.
0993   ljmpl    $(SEG_KCODE<<3), $(start32)
0994 
0995 
0996 
0997 
0998 
0999 
1000 .code32  # Tell assembler to generate 32-bit code now.
1001 start32:
1002   # Set up the protected-mode data segment registers
1003   movw    $(SEG_KDATA<<3), %ax    # Our data segment selector
1004   movw    %ax, %ds                # -> DS: Data Segment
1005   movw    %ax, %es                # -> ES: Extra Segment
1006   movw    %ax, %ss                # -> SS: Stack Segment
1007   movw    $0, %ax                 # Zero segments not ready for use
1008   movw    %ax, %fs                # -> FS
1009   movw    %ax, %gs                # -> GS
1010 
1011   # Turn on page size extension for 4Mbyte pages
1012   movl    %cr4, %eax
1013   orl     $(CR4_PSE), %eax
1014   movl    %eax, %cr4
1015   # Use entrypgdir as our initial page table
1016   movl    (start-12), %eax
1017   movl    %eax, %cr3
1018   # Turn on paging.
1019   movl    %cr0, %eax
1020   orl     $(CR0_PE|CR0_PG|CR0_WP), %eax
1021   movl    %eax, %cr0
1022 
1023   # Switch to the stack allocated by startothers()
1024   movl    (start-4), %esp
1025   # Call mpenter()
1026   call	 *(start-8)
1027 
1028   movw    $0x8a00, %ax
1029   movw    %ax, %dx
1030   outw    %ax, %dx
1031   movw    $0x8ae0, %ax
1032   outw    %ax, %dx
1033 spin:
1034   jmp     spin
1035 
1036 .p2align 2
1037 gdt:
1038   SEG_NULLASM
1039   SEG_ASM(STA_X|STA_R, 0, 0xffffffff)
1040   SEG_ASM(STA_W, 0, 0xffffffff)
1041 
1042 
1043 gdtdesc:
1044   .word   (gdtdesc - gdt - 1)
1045   .long   gdt
1046 
1047 
1048 
1049 
