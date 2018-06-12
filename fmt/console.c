8150 // Console input and output.
8151 // Input is from the keyboard or serial port.
8152 // Output is written to the screen and serial port.
8153 
8154 #include "types.h"
8155 #include "defs.h"
8156 #include "param.h"
8157 #include "traps.h"
8158 #include "spinlock.h"
8159 #include "sleeplock.h"
8160 #include "fs.h"
8161 #include "file.h"
8162 #include "memlayout.h"
8163 #include "mmu.h"
8164 #include "proc.h"
8165 #include "riscv.h"
8166 
8167 
8168 static void consputc(int);
8169 
8170 static int panicked = 0;
8171 
8172 static struct {
8173   struct spinlock lock;
8174   int locking;
8175 } cons;
8176 
8177 static void
8178 printint(int xx, int base, int sign)
8179 {
8180   static char digits[] = "0123456789abcdef";
8181   char buf[16];
8182   int i;
8183   uint x;
8184 
8185   if(sign && (sign = xx < 0))
8186     x = -xx;
8187   else
8188     x = xx;
8189 
8190   i = 0;
8191   do{
8192     buf[i++] = digits[x % base];
8193   }while((x /= base) != 0);
8194 
8195   if(sign)
8196     buf[i++] = '-';
8197 
8198 
8199 
8200   while(--i >= 0)
8201     consputc(buf[i]);
8202 }
8203 
8204 
8205 
8206 
8207 
8208 
8209 
8210 
8211 
8212 
8213 
8214 
8215 
8216 
8217 
8218 
8219 
8220 
8221 
8222 
8223 
8224 
8225 
8226 
8227 
8228 
8229 
8230 
8231 
8232 
8233 
8234 
8235 
8236 
8237 
8238 
8239 
8240 
8241 
8242 
8243 
8244 
8245 
8246 
8247 
8248 
8249 
8250 // Print to the console. only understands %d, %x, %p, %s.
8251 void
8252 cprintf(char *fmt, ...)
8253 {
8254   int i, c, locking;
8255   uint *argp;
8256   char *s;
8257 
8258   locking = cons.locking;
8259   if(locking)
8260     acquire(&cons.lock);
8261 
8262   if (fmt == 0)
8263     panic("null fmt");
8264 
8265   asm volatile("mv %0,s0" : "=r"(argp));
8266   argp++;
8267   //argp = (uint*)(void*)(&fmt + 1);
8268   for(i = 0; (c = fmt[i] & 0xff) != 0; i++){
8269     if(c != '%'){
8270       consputc(c);
8271       continue;
8272     }
8273     c = fmt[++i] & 0xff;
8274     if(c == 0)
8275       break;
8276     switch(c){
8277     case 'd':
8278       printint(*argp++, 10, 1);
8279       break;
8280     case 'x':
8281     case 'p':
8282       printint(*argp++, 16, 0);
8283       break;
8284     case 's':
8285       if((s = (char*)*argp++) == 0)
8286         s = "(null)";
8287       for(; *s; s++)
8288         consputc(*s);
8289       break;
8290     case '%':
8291       consputc('%');
8292       break;
8293     default:
8294       // Print unknown % sequence to draw attention.
8295       consputc('%');
8296       consputc(c);
8297       break;
8298     }
8299   }
8300   if(locking)
8301     release(&cons.lock);
8302 }
8303 
8304 void
8305 panic(char *s)
8306 {
8307   int i;
8308   uint pcs[10];
8309 
8310   cli();
8311   cons.locking = 0;
8312   // use lapiccpunum so that we can call panic from mycpu()
8313   cprintf("lapicid %d: panic: ", lapicid());
8314   cprintf(s);
8315   cprintf("\n");
8316   getcallerpcs(&s, pcs);
8317   for(i=0; i<10; i++)
8318     cprintf(" %p", pcs[i]);
8319   panicked = 1; // freeze other CPU
8320   for(;;)
8321     ;
8322 }
8323 
8324 
8325 
8326 
8327 
8328 
8329 
8330 
8331 
8332 
8333 
8334 
8335 
8336 
8337 
8338 
8339 
8340 
8341 
8342 
8343 
8344 
8345 
8346 
8347 
8348 
8349 
8350 #define BACKSPACE 0x100
8351 #define CRTPORT 0x3d4
8352 static ushort *crt = (ushort*)P2V(0xb8000);  // CGA memory
8353 
8354 static void
8355 cgaputc(int c)
8356 {
8357   int pos;
8358 
8359   // Cursor position: col + 80*row.
8360   outb(CRTPORT, 14);
8361   pos = inb(CRTPORT+1) << 8;
8362   outb(CRTPORT, 15);
8363   pos |= inb(CRTPORT+1);
8364 
8365   if(c == '\n')
8366     pos += 80 - pos%80;
8367   else if(c == BACKSPACE){
8368     if(pos > 0) --pos;
8369   } else
8370     crt[pos++] = (c&0xff) | 0x0700;  // black on white
8371 
8372   if(pos < 0 || pos > 25*80)
8373     panic("pos under/overflow");
8374 
8375   if((pos/80) >= 24){  // Scroll up.
8376     memmove(crt, crt+80, sizeof(crt[0])*23*80);
8377     pos -= 80;
8378     memset(crt+pos, 0, sizeof(crt[0])*(24*80 - pos));
8379   }
8380 
8381   outb(CRTPORT, 14);
8382   outb(CRTPORT+1, pos>>8);
8383   outb(CRTPORT, 15);
8384   outb(CRTPORT+1, pos);
8385   crt[pos] = ' ' | 0x0700;
8386 }
8387 
8388 
8389 
8390 
8391 
8392 
8393 
8394 
8395 
8396 
8397 
8398 
8399 
8400 void
8401 consputc(int c)
8402 {
8403   if(panicked){
8404     cli();
8405     for(;;)
8406       ;
8407   }
8408 
8409   if(c == BACKSPACE){
8410     uartputc('\b'); uartputc(' '); uartputc('\b');
8411   } else
8412     uartputc(c);
8413 
8414   if(c == -10000)
8415   cgaputc(c);
8416 }
8417 
8418 #define INPUT_BUF 128
8419 struct {
8420   char buf[INPUT_BUF];
8421   uint r;  // Read index
8422   uint w;  // Write index
8423   uint e;  // Edit index
8424 } input;
8425 
8426 #define C(x)  ((x)-'@')  // Control-x
8427 
8428 void
8429 consoleintr(int (*getc)(void))
8430 {
8431   int c, doprocdump = 0;
8432 
8433   acquire(&cons.lock);
8434   while((c = getc()) >= 0){
8435     switch(c){
8436     case C('P'):  // Process listing.
8437       // procdump() locks cons.lock indirectly; invoke later
8438       doprocdump = 1;
8439       break;
8440     case C('U'):  // Kill line.
8441       while(input.e != input.w &&
8442             input.buf[(input.e-1) % INPUT_BUF] != '\n'){
8443         input.e--;
8444         consputc(BACKSPACE);
8445       }
8446       break;
8447     case C('H'): case '\x7f':  // Backspace
8448       if(input.e != input.w){
8449         input.e--;
8450         consputc(BACKSPACE);
8451       }
8452       break;
8453     default:
8454       if(c != 0 && input.e-input.r < INPUT_BUF){
8455         c = (c == '\r') ? '\n' : c;
8456         input.buf[input.e++ % INPUT_BUF] = c;
8457         consputc(c);
8458         if(c == '\n' || c == C('D') || input.e == input.r+INPUT_BUF){
8459           input.w = input.e;
8460           wakeup(&input.r);
8461         }
8462       }
8463       break;
8464     }
8465   }
8466   release(&cons.lock);
8467   if(doprocdump) {
8468     procdump();  // now call procdump() wo. cons.lock held
8469   }
8470 }
8471 
8472 int
8473 consoleread(struct inode *ip, char *dst, int n)
8474 {
8475   uint target;
8476   int c;
8477 
8478   iunlock(ip);
8479   target = n;
8480   acquire(&cons.lock);
8481   while(n > 0){
8482     while(input.r == input.w){
8483       if(myproc()->killed){
8484         release(&cons.lock);
8485         ilock(ip);
8486         return -1;
8487       }
8488       sleep(&input.r, &cons.lock);
8489     }
8490     c = input.buf[input.r++ % INPUT_BUF];
8491     if(c == C('D')){  // EOF
8492       if(n < target){
8493         // Save ^D for next time, to make sure
8494         // caller gets a 0-byte result.
8495         input.r--;
8496       }
8497       break;
8498     }
8499     *dst++ = c;
8500     --n;
8501     if(c == '\n')
8502       break;
8503   }
8504   release(&cons.lock);
8505   ilock(ip);
8506 
8507   return target - n;
8508 }
8509 
8510 int
8511 consolewrite(struct inode *ip, char *buf, int n)
8512 {
8513   int i;
8514 
8515   iunlock(ip);
8516   acquire(&cons.lock);
8517   for(i = 0; i < n; i++)
8518     consputc(buf[i] & 0xff);
8519   release(&cons.lock);
8520   ilock(ip);
8521 
8522   return n;
8523 }
8524 
8525 void
8526 consoleinit(void)
8527 {
8528   initlock(&cons.lock, "console");
8529 
8530   devsw[CONSOLE].write = consolewrite;
8531   devsw[CONSOLE].read = consoleread;
8532   cons.locking = 1;
8533 
8534   //ioapicenable(IRQ_KBD, 0);
8535 }
8536 
8537 
8538 
8539 
8540 
8541 
8542 
8543 
8544 
8545 
8546 
8547 
8548 
8549 
