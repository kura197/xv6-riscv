7600 // The local APIC manages internal (non-I/O) interrupts.
7601 // See Chapter 8 & Appendix C of Intel processor manual volume 3.
7602 
7603 #include "param.h"
7604 #include "types.h"
7605 #include "defs.h"
7606 #include "date.h"
7607 #include "memlayout.h"
7608 #include "traps.h"
7609 #include "mmu.h"
7610 #include "riscv.h"
7611 
7612 
7613 // Local APIC registers, divided by 4 for use as uint[] indices.
7614 #define ID      (0x0020/4)   // ID
7615 #define VER     (0x0030/4)   // Version
7616 #define TPR     (0x0080/4)   // Task Priority
7617 #define EOI     (0x00B0/4)   // EOI
7618 #define SVR     (0x00F0/4)   // Spurious Interrupt Vector
7619   #define ENABLE     0x00000100   // Unit Enable
7620 #define ESR     (0x0280/4)   // Error Status
7621 #define ICRLO   (0x0300/4)   // Interrupt Command
7622   #define INIT       0x00000500   // INIT/RESET
7623   #define STARTUP    0x00000600   // Startup IPI
7624   #define DELIVS     0x00001000   // Delivery status
7625   #define ASSERT     0x00004000   // Assert interrupt (vs deassert)
7626   #define DEASSERT   0x00000000
7627   #define LEVEL      0x00008000   // Level triggered
7628   #define BCAST      0x00080000   // Send to all APICs, including self.
7629   #define BUSY       0x00001000
7630   #define FIXED      0x00000000
7631 #define ICRHI   (0x0310/4)   // Interrupt Command [63:32]
7632 //#define TIMER   (0x0320/4)   // Local Vector Table 0 (TIMER)
7633   #define X1         0x0000000B   // divide counts by 1
7634   #define PERIODIC   0x00020000   // Periodic
7635 #define PCINT   (0x0340/4)   // Performance Counter LVT
7636 #define LINT0   (0x0350/4)   // Local Vector Table 1 (LINT0)
7637 #define LINT1   (0x0360/4)   // Local Vector Table 2 (LINT1)
7638 #define ERROR   (0x0370/4)   // Local Vector Table 3 (ERROR)
7639   #define MASKED     0x00010000   // Interrupt masked
7640 #define TICR    (0x0380/4)   // Timer Initial Count
7641 #define TCCR    (0x0390/4)   // Timer Current Count
7642 #define TDCR    (0x03E0/4)   // Timer Divide Configuration
7643 
7644 volatile uint *lapic;  // Initialized in mp.c
7645 
7646 
7647 
7648 
7649 
7650 static void
7651 lapicw(int index, int value)
7652 {
7653   lapic[index] = value;
7654   lapic[ID];  // wait for write to finish, by reading
7655 }
7656 
7657 void
7658 lapicinit(void)
7659 {
7660   //if(!lapic)
7661   //  return;
7662   set_timecmp(FREQ_L(100),0);
7663   set_timecmp(0,1);
7664   en_intr(TIMER);
7665   en_intr(EXTERNAL);
7666 /*
7667   // Enable local APIC; set spurious interrupt vector.
7668   lapicw(SVR, ENABLE | (T_IRQ0 + IRQ_SPURIOUS));
7669 
7670   // The timer repeatedly counts down at bus frequency
7671   // from lapic[TICR] and then issues an interrupt.
7672   // If xv6 cared more about precise timekeeping,
7673   // TICR would be calibrated using an external time source.
7674   lapicw(TDCR, X1);
7675   lapicw(TIMER, PERIODIC | (T_IRQ0 + IRQ_TIMER));
7676   lapicw(TICR, 10000000);
7677 
7678   // Disable logical interrupt lines.
7679   lapicw(LINT0, MASKED);
7680   lapicw(LINT1, MASKED);
7681 
7682   // Disable performance counter overflow interrupts
7683   // on machines that provide that interrupt entry.
7684   if(((lapic[VER]>>16) & 0xFF) >= 4)
7685     lapicw(PCINT, MASKED);
7686 
7687   // Map error interrupt to IRQ_ERROR.
7688   lapicw(ERROR, T_IRQ0 + IRQ_ERROR);
7689 
7690   // Clear error status register (requires back-to-back writes).
7691   lapicw(ESR, 0);
7692   lapicw(ESR, 0);
7693 
7694   // Ack any outstanding interrupts.
7695   lapicw(EOI, 0);
7696 
7697 
7698 
7699 
7700   // Send an Init Level De-Assert to synchronise arbitration ID's.
7701   lapicw(ICRHI, 0);
7702   lapicw(ICRLO, BCAST | INIT | LEVEL);
7703   while(lapic[ICRLO] & DELIVS)
7704     ;
7705 
7706   // Enable interrupts on the APIC (but not on the processor).
7707   lapicw(TPR, 0);
7708   */
7709 }
7710 
7711 int
7712 lapicid(void)
7713 {
7714   if (!lapic)
7715     return 0;
7716   return lapic[ID] >> 24;
7717 }
7718 
7719 // Acknowledge interrupt.
7720 void
7721 lapiceoi(void)
7722 {
7723   if(lapic)
7724     lapicw(EOI, 0);
7725 }
7726 
7727 // Spin for a given number of microseconds.
7728 // On real hardware would want to tune this dynamically.
7729 void
7730 microdelay(int us)
7731 {
7732 }
7733 
7734 #define CMOS_PORT    0x70
7735 #define CMOS_RETURN  0x71
7736 
7737 // Start additional processor running entry code at addr.
7738 // See Appendix B of MultiProcessor Specification.
7739 void
7740 lapicstartap(uchar apicid, uint addr)
7741 {
7742   int i;
7743   ushort *wrv;
7744 
7745   // "The BSP must initialize CMOS shutdown code to 0AH
7746   // and the warm reset vector (DWORD based at 40:67) to point at
7747   // the AP startup code prior to the [universal startup algorithm]."
7748   outb(CMOS_PORT, 0xF);  // offset 0xF is shutdown code
7749   outb(CMOS_PORT+1, 0x0A);
7750   wrv = (ushort*)P2V((0x40<<4 | 0x67));  // Warm reset vector
7751   wrv[0] = 0;
7752   wrv[1] = addr >> 4;
7753 
7754   // "Universal startup algorithm."
7755   // Send INIT (level-triggered) interrupt to reset other CPU.
7756   lapicw(ICRHI, apicid<<24);
7757   lapicw(ICRLO, INIT | LEVEL | ASSERT);
7758   microdelay(200);
7759   lapicw(ICRLO, INIT | LEVEL);
7760   microdelay(100);    // should be 10ms, but too slow in Bochs!
7761 
7762   // Send startup IPI (twice!) to enter code.
7763   // Regular hardware is supposed to only accept a STARTUP
7764   // when it is in the halted state due to an INIT.  So the second
7765   // should be ignored, but it is part of the official Intel algorithm.
7766   // Bochs complains about the second one.  Too bad for Bochs.
7767   for(i = 0; i < 2; i++){
7768     lapicw(ICRHI, apicid<<24);
7769     lapicw(ICRLO, STARTUP | (addr>>12));
7770     microdelay(200);
7771   }
7772 }
7773 
7774 #define CMOS_STATA   0x0a
7775 #define CMOS_STATB   0x0b
7776 #define CMOS_UIP    (1 << 7)        // RTC update in progress
7777 
7778 #define SECS    0x00
7779 #define MINS    0x02
7780 #define HOURS   0x04
7781 #define DAY     0x07
7782 #define MONTH   0x08
7783 #define YEAR    0x09
7784 
7785 static uint cmos_read(uint reg)
7786 {
7787   outb(CMOS_PORT,  reg);
7788   microdelay(200);
7789 
7790   return inb(CMOS_RETURN);
7791 }
7792 
7793 
7794 
7795 
7796 
7797 
7798 
7799 
7800 static void fill_rtcdate(struct rtcdate *r)
7801 {
7802   r->second = cmos_read(SECS);
7803   r->minute = cmos_read(MINS);
7804   r->hour   = cmos_read(HOURS);
7805   r->day    = cmos_read(DAY);
7806   r->month  = cmos_read(MONTH);
7807   r->year   = cmos_read(YEAR);
7808 }
7809 
7810 // qemu seems to use 24-hour GWT and the values are BCD encoded
7811 void cmostime(struct rtcdate *r)
7812 {
7813   struct rtcdate t1, t2;
7814   int sb, bcd;
7815 
7816   sb = cmos_read(CMOS_STATB);
7817 
7818   bcd = (sb & (1 << 2)) == 0;
7819 
7820   // make sure CMOS doesn't modify time while we read it
7821   for(;;) {
7822     fill_rtcdate(&t1);
7823     if(cmos_read(CMOS_STATA) & CMOS_UIP)
7824         continue;
7825     fill_rtcdate(&t2);
7826     if(memcmp(&t1, &t2, sizeof(t1)) == 0)
7827       break;
7828   }
7829 
7830   // convert
7831   if(bcd) {
7832 #define    CONV(x)     (t1.x = ((t1.x >> 4) * 10) + (t1.x & 0xf))
7833     CONV(second);
7834     CONV(minute);
7835     CONV(hour  );
7836     CONV(day   );
7837     CONV(month );
7838     CONV(year  );
7839 #undef     CONV
7840   }
7841 
7842   *r = t1;
7843   r->year += 2000;
7844 }
7845 
7846 
7847 
7848 
7849 
