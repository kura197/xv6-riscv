3800 #include "types.h"
3801 #include "defs.h"
3802 #include "param.h"
3803 #include "memlayout.h"
3804 #include "mmu.h"
3805 #include "proc.h"
3806 #include "syscall.h"
3807 #include "riscv.h"
3808 
3809 // User code makes a system call with INT T_SYSCALL.
3810 // System call number in %eax.
3811 // Arguments on the stack, from the user call to the C
3812 // library system call function. The saved user %esp points
3813 // to a saved program counter, and then the first argument.
3814 
3815 // Fetch the int at addr from the current process.
3816 int
3817 fetchint(uint addr, int *ip)
3818 {
3819   struct proc *curproc = myproc();
3820 
3821   if(addr >= curproc->sz || addr+4 > curproc->sz)
3822     return -1;
3823   *ip = *(int*)(addr);
3824   return 0;
3825 }
3826 
3827 // Fetch the nul-terminated string at addr from the current process.
3828 // Doesn't actually copy the string - just sets *pp to point at it.
3829 // Returns length of string, not including nul.
3830 int
3831 fetchstr(uint addr, char **pp)
3832 {
3833   char *s, *ep;
3834   struct proc *curproc = myproc();
3835 
3836   if(addr >= curproc->sz)
3837     return -1;
3838   *pp = (char*)addr;
3839   ep = (char*)curproc->sz;
3840   for(s = *pp; s < ep; s++){
3841     if(*s == 0)
3842       return (s - *pp);
3843   }
3844   return -1;
3845 }
3846 
3847 
3848 
3849 
3850 // Fetch the nth 32-bit system call argument.
3851 int
3852 argint(int n, int *ip)
3853 {
3854 	//
3855   return fetchint((myproc()->tf->sp) + 4 + 4*n, ip);
3856 }
3857 
3858 // Fetch the nth word-sized system call argument as a pointer
3859 // to a block of memory of size bytes.  Check that the pointer
3860 // lies within the process address space.
3861 int
3862 argptr(int n, char **pp, int size)
3863 {
3864   int i;
3865   struct proc *curproc = myproc();
3866 
3867   if(argint(n, &i) < 0)
3868     return -1;
3869   if(size < 0 || (uint)i >= curproc->sz || (uint)i+size > curproc->sz)
3870     return -1;
3871   *pp = (char*)i;
3872   return 0;
3873 }
3874 
3875 // Fetch the nth word-sized system call argument as a string pointer.
3876 // Check that the pointer is valid and the string is nul-terminated.
3877 // (There is no shared writable memory, so the string can't change
3878 // between this check and being used by the kernel.)
3879 int
3880 argstr(int n, char **pp)
3881 {
3882   int addr;
3883   if(argint(n, &addr) < 0)
3884     return -1;
3885   return fetchstr(addr, pp);
3886 }
3887 
3888 
3889 
3890 
3891 
3892 
3893 
3894 
3895 
3896 
3897 
3898 
3899 
3900 extern int sys_chdir(void);
3901 extern int sys_close(void);
3902 extern int sys_dup(void);
3903 extern int sys_exec(void);
3904 extern int sys_exit(void);
3905 extern int sys_fork(void);
3906 extern int sys_fstat(void);
3907 extern int sys_getpid(void);
3908 extern int sys_kill(void);
3909 extern int sys_link(void);
3910 extern int sys_mkdir(void);
3911 extern int sys_mknod(void);
3912 extern int sys_open(void);
3913 extern int sys_pipe(void);
3914 extern int sys_read(void);
3915 extern int sys_sbrk(void);
3916 extern int sys_sleep(void);
3917 extern int sys_unlink(void);
3918 extern int sys_wait(void);
3919 extern int sys_write(void);
3920 extern int sys_uptime(void);
3921 
3922 static int (*syscalls[])(void) = {
3923 [SYS_fork]    sys_fork,
3924 [SYS_exit]    sys_exit,
3925 [SYS_wait]    sys_wait,
3926 [SYS_pipe]    sys_pipe,
3927 [SYS_read]    sys_read,
3928 [SYS_kill]    sys_kill,
3929 [SYS_exec]    sys_exec,
3930 [SYS_fstat]   sys_fstat,
3931 [SYS_chdir]   sys_chdir,
3932 [SYS_dup]     sys_dup,
3933 [SYS_getpid]  sys_getpid,
3934 [SYS_sbrk]    sys_sbrk,
3935 [SYS_sleep]   sys_sleep,
3936 [SYS_uptime]  sys_uptime,
3937 [SYS_open]    sys_open,
3938 [SYS_write]   sys_write,
3939 [SYS_mknod]   sys_mknod,
3940 [SYS_unlink]  sys_unlink,
3941 [SYS_link]    sys_link,
3942 [SYS_mkdir]   sys_mkdir,
3943 [SYS_close]   sys_close,
3944 };
3945 
3946 
3947 
3948 
3949 
3950 void
3951 syscall(void)
3952 {
3953   int num;
3954   struct proc *curproc = myproc();
3955 
3956   //num = curproc->tf->eax;
3957   num = curproc->tf->a0;
3958   if(num > 0 && num < NELEM(syscalls) && syscalls[num]) {
3959     //curproc->tf->eax = syscalls[num]();
3960     curproc->tf->a0 = syscalls[num]();
3961   } else {
3962     cprintf("%d %s: unknown sys call %d\n",
3963             curproc->pid, curproc->name, num);
3964 	//
3965     curproc->tf->a0 = -1;
3966   }
3967 }
3968 
3969 
3970 
3971 
3972 
3973 
3974 
3975 
3976 
3977 
3978 
3979 
3980 
3981 
3982 
3983 
3984 
3985 
3986 
3987 
3988 
3989 
3990 
3991 
3992 
3993 
3994 
3995 
3996 
3997 
3998 
3999 
