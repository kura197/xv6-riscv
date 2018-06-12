8100 #include "types.h"
8101 #include "defs.h"
8102 #include "kbd.h"
8103 #include "riscv.h"
8104 
8105 int
8106 kbdgetc(void)
8107 {
8108   static uint shift;
8109   static uchar *charcode[4] = {
8110     normalmap, shiftmap, ctlmap, ctlmap
8111   };
8112   uint st, data, c;
8113 
8114   st = inb(KBSTATP);
8115   if((st & KBS_DIB) == 0)
8116     return -1;
8117   data = inb(KBDATAP);
8118 
8119   if(data == 0xE0){
8120     shift |= E0ESC;
8121     return 0;
8122   } else if(data & 0x80){
8123     // Key released
8124     data = (shift & E0ESC ? data : data & 0x7F);
8125     shift &= ~(shiftcode[data] | E0ESC);
8126     return 0;
8127   } else if(shift & E0ESC){
8128     // Last character was an E0 escape; or with 0x80
8129     data |= 0x80;
8130     shift &= ~E0ESC;
8131   }
8132 
8133   shift |= shiftcode[data];
8134   shift ^= togglecode[data];
8135   c = charcode[shift & (CTL | SHIFT)][data];
8136   if(shift & CAPSLOCK){
8137     if('a' <= c && c <= 'z')
8138       c += 'A' - 'a';
8139     else if('A' <= c && c <= 'Z')
8140       c += 'a' - 'A';
8141   }
8142   return c;
8143 }
8144 
8145 void
8146 kbdintr(void)
8147 {
8148   consoleintr(kbdgetc);
8149 }
