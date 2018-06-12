8550 // Intel 8250 serial port (UART).
8551 
8552 #include "types.h"
8553 #include "defs.h"
8554 #include "param.h"
8555 #include "traps.h"
8556 #include "spinlock.h"
8557 #include "sleeplock.h"
8558 #include "fs.h"
8559 #include "file.h"
8560 #include "mmu.h"
8561 #include "proc.h"
8562 #include "riscv.h"
8563 
8564 #define COM1    0x3f8
8565 
8566 static int uart;    // is there a uart?
8567 
8568 void
8569 uartinit(void)
8570 {
8571   char *p;
8572 
8573   // Turn off the FIFO
8574   outb(COM1+2, 0);
8575 
8576   // 9600 baud, 8 data bits, 1 stop bit, parity off.
8577   outb(COM1+3, 0x80);    // Unlock divisor
8578   outb(COM1+0, 115200/9600);
8579   outb(COM1+1, 0);
8580   outb(COM1+3, 0x03);    // Lock divisor, 8 data bits.
8581   outb(COM1+4, 0);
8582   outb(COM1+1, 0x01);    // Enable receive interrupts.
8583 
8584   // If status is 0xFF, no serial port.
8585   if(inb(COM1+5) == 0xFF)
8586     return;
8587   uart = 1;
8588 
8589   // Acknowledge pre-existing interrupt conditions;
8590   // enable interrupts.
8591   inb(COM1+2);
8592   inb(COM1+0);
8593   //ioapicenable(IRQ_COM1, 0);
8594 
8595   // Announce that we're here.
8596   for(p="xv6...\n"; *p; p++)
8597     uartputc(*p);
8598 }
8599 
8600 void
8601 uartputc(int c)
8602 {
8603   int i;
8604 
8605   if(!uart)
8606     return;
8607   for(i = 0; i < 128 && !(inb(COM1+5) & 0x20); i++)
8608     microdelay(10);
8609   outb(COM1+0, c);
8610 }
8611 
8612 static int
8613 uartgetc(void)
8614 {
8615   if(!uart)
8616     return -1;
8617   if(!(inb(COM1+5) & 0x01))
8618     return -1;
8619   return inb(COM1+0);
8620 }
8621 
8622 void
8623 uartintr(void)
8624 {
8625   consoleintr(uartgetc);
8626 }
8627 
8628 
8629 
8630 
8631 
8632 
8633 
8634 
8635 
8636 
8637 
8638 
8639 
8640 
8641 
8642 
8643 
8644 
8645 
8646 
8647 
8648 
8649 
