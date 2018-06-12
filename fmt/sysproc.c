4000 #include "types.h"
4001 #include "riscv.h"
4002 #include "defs.h"
4003 #include "date.h"
4004 #include "param.h"
4005 #include "memlayout.h"
4006 #include "mmu.h"
4007 #include "proc.h"
4008 
4009 int
4010 sys_fork(void)
4011 {
4012   return fork();
4013 }
4014 
4015 int
4016 sys_exit(void)
4017 {
4018   exit();
4019   return 0;  // not reached
4020 }
4021 
4022 int
4023 sys_wait(void)
4024 {
4025   return wait();
4026 }
4027 
4028 int
4029 sys_kill(void)
4030 {
4031   int pid;
4032 
4033   if(argint(0, &pid) < 0)
4034     return -1;
4035   return kill(pid);
4036 }
4037 
4038 int
4039 sys_getpid(void)
4040 {
4041   return myproc()->pid;
4042 }
4043 
4044 
4045 
4046 
4047 
4048 
4049 
4050 int
4051 sys_sbrk(void)
4052 {
4053   int addr;
4054   int n;
4055 
4056   if(argint(0, &n) < 0)
4057     return -1;
4058   addr = myproc()->sz;
4059   if(growproc(n) < 0)
4060     return -1;
4061   return addr;
4062 }
4063 
4064 int
4065 sys_sleep(void)
4066 {
4067   int n;
4068   uint ticks0;
4069 
4070   if(argint(0, &n) < 0)
4071     return -1;
4072   acquire(&tickslock);
4073   ticks0 = ticks;
4074   while(ticks - ticks0 < n){
4075     if(myproc()->killed){
4076       release(&tickslock);
4077       return -1;
4078     }
4079     sleep(&ticks, &tickslock);
4080   }
4081   release(&tickslock);
4082   return 0;
4083 }
4084 
4085 // return how many clock tick interrupts have occurred
4086 // since start.
4087 int
4088 sys_uptime(void)
4089 {
4090   uint xticks;
4091 
4092   acquire(&tickslock);
4093   xticks = ticks;
4094   release(&tickslock);
4095   return xticks;
4096 }
4097 
4098 
4099 
