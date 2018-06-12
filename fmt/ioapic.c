7850 // The I/O APIC manages hardware interrupts for an SMP system.
7851 // http://www.intel.com/design/chipsets/datashts/29056601.pdf
7852 // See also picirq.c.
7853 
7854 #include "types.h"
7855 #include "defs.h"
7856 #include "traps.h"
7857 
7858 #define IOAPIC  0xFEC00000   // Default physical address of IO APIC
7859 
7860 #define REG_ID     0x00  // Register index: ID
7861 #define REG_VER    0x01  // Register index: version
7862 #define REG_TABLE  0x10  // Redirection table base
7863 
7864 // The redirection table starts at REG_TABLE and uses
7865 // two registers to configure each interrupt.
7866 // The first (low) register in a pair contains configuration bits.
7867 // The second (high) register contains a bitmask telling which
7868 // CPUs can serve that interrupt.
7869 #define INT_DISABLED   0x00010000  // Interrupt disabled
7870 #define INT_LEVEL      0x00008000  // Level-triggered (vs edge-)
7871 #define INT_ACTIVELOW  0x00002000  // Active low (vs high)
7872 #define INT_LOGICAL    0x00000800  // Destination is CPU id (vs APIC ID)
7873 
7874 volatile struct ioapic *ioapic;
7875 
7876 // IO APIC MMIO structure: write reg, then read or write data.
7877 struct ioapic {
7878   uint reg;
7879   uint pad[3];
7880   uint data;
7881 };
7882 
7883 static uint
7884 ioapicread(int reg)
7885 {
7886   ioapic->reg = reg;
7887   return ioapic->data;
7888 }
7889 
7890 static void
7891 ioapicwrite(int reg, uint data)
7892 {
7893   ioapic->reg = reg;
7894   ioapic->data = data;
7895 }
7896 
7897 
7898 
7899 
7900 void
7901 ioapicinit(void)
7902 {
7903   int i, id, maxintr;
7904 
7905   ioapic = (volatile struct ioapic*)IOAPIC;
7906   maxintr = (ioapicread(REG_VER) >> 16) & 0xFF;
7907   id = ioapicread(REG_ID) >> 24;
7908   if(id != ioapicid)
7909     cprintf("ioapicinit: id isn't equal to ioapicid; not a MP\n");
7910 
7911   // Mark all interrupts edge-triggered, active high, disabled,
7912   // and not routed to any CPUs.
7913   for(i = 0; i <= maxintr; i++){
7914     ioapicwrite(REG_TABLE+2*i, INT_DISABLED | (T_IRQ0 + i));
7915     ioapicwrite(REG_TABLE+2*i+1, 0);
7916   }
7917 }
7918 
7919 void
7920 ioapicenable(int irq, int cpunum)
7921 {
7922   // Mark interrupt edge-triggered, active high,
7923   // enabled, and routed to the given cpunum,
7924   // which happens to be that cpu's APIC ID.
7925   ioapicwrite(REG_TABLE+2*irq, T_IRQ0 + irq);
7926   ioapicwrite(REG_TABLE+2*irq+1, cpunum << 24);
7927 }
7928 
7929 
7930 
7931 
7932 
7933 
7934 
7935 
7936 
7937 
7938 
7939 
7940 
7941 
7942 
7943 
7944 
7945 
7946 
7947 
7948 
7949 
