0450 //
0451 // assembler macros to create x86 segments
0452 //
0453 
0454 #define SEG_NULLASM                                             \
0455         .word 0, 0;                                             \
0456         .byte 0, 0, 0, 0
0457 
0458 // The 0xC0 means the limit is in 4096-byte units
0459 // and (for executable segments) 32-bit mode.
0460 #define SEG_ASM(type,base,lim)                                  \
0461         .word (((lim) >> 12) & 0xffff), ((base) & 0xffff);      \
0462         .byte (((base) >> 16) & 0xff), (0x90 | (type)),         \
0463                 (0xC0 | (((lim) >> 28) & 0xf)), (((base) >> 24) & 0xff)
0464 
0465 #define STA_X     0x8       // Executable segment
0466 #define STA_E     0x4       // Expand down (non-executable segments)
0467 #define STA_C     0x4       // Conforming code segment (executable only)
0468 #define STA_W     0x2       // Writeable (non-executable segments)
0469 #define STA_R     0x2       // Readable (executable segments)
0470 #define STA_A     0x1       // Accessed
0471 
0472 
0473 
0474 
0475 
0476 
0477 
0478 
0479 
0480 
0481 
0482 
0483 
0484 
0485 
0486 
0487 
0488 
0489 
0490 
0491 
0492 
0493 
0494 
0495 
0496 
0497 
0498 
0499 
