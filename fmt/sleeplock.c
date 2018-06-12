4850 // Sleeping locks
4851 
4852 #include "types.h"
4853 #include "defs.h"
4854 #include "param.h"
4855 #include "riscv.h"
4856 #include "memlayout.h"
4857 #include "mmu.h"
4858 #include "proc.h"
4859 #include "spinlock.h"
4860 #include "sleeplock.h"
4861 
4862 void
4863 initsleeplock(struct sleeplock *lk, char *name)
4864 {
4865   initlock(&lk->lk, "sleep lock");
4866   lk->name = name;
4867   lk->locked = 0;
4868   lk->pid = 0;
4869 }
4870 
4871 void
4872 acquiresleep(struct sleeplock *lk)
4873 {
4874   acquire(&lk->lk);
4875   while (lk->locked) {
4876     sleep(lk, &lk->lk);
4877   }
4878   lk->locked = 1;
4879   lk->pid = myproc()->pid;
4880   release(&lk->lk);
4881 }
4882 
4883 void
4884 releasesleep(struct sleeplock *lk)
4885 {
4886   acquire(&lk->lk);
4887   lk->locked = 0;
4888   lk->pid = 0;
4889   wakeup(lk);
4890   release(&lk->lk);
4891 }
4892 
4893 
4894 
4895 
4896 
4897 
4898 
4899 
4900 int
4901 holdingsleep(struct sleeplock *lk)
4902 {
4903   int r;
4904 
4905   acquire(&lk->lk);
4906   r = lk->locked;
4907   release(&lk->lk);
4908   return r;
4909 }
4910 
4911 
4912 
4913 
4914 
4915 
4916 
4917 
4918 
4919 
4920 
4921 
4922 
4923 
4924 
4925 
4926 
4927 
4928 
4929 
4930 
4931 
4932 
4933 
4934 
4935 
4936 
4937 
4938 
4939 
4940 
4941 
4942 
4943 
4944 
4945 
4946 
4947 
4948 
4949 
