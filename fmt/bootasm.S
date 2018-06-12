9450 #include "asm.h"
9451 #include "memlayout.h"
9452 #include "mmu.h"
9453 
9454 # Start the first CPU: switch to 32-bit protected mode, jump into C.
9455 # The BIOS loads this code from the first sector of the hard disk into
9456 # memory at physical address 0x7c00 and starts executing in real mode
9457 # with %cs=0 %ip=7c00.
9458 
9459 #.code16                       # Assemble for 16-bit mode
9460 .globl start
9461 start:
9462   la    sp, start
9463   j    bootmain
9464 
9465 
9466 
9467 
9468 
9469 
9470 
9471 
9472 
9473 
9474 
9475 
9476 
9477 
9478 
9479 
9480 
9481 
9482 
9483 
9484 
9485 
9486 
9487 
9488 
9489 
9490 
9491 
9492 
9493 
9494 
9495 
9496 
9497 
9498 
9499 
