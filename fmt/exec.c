6850 #include "types.h"
6851 #include "param.h"
6852 #include "memlayout.h"
6853 #include "mmu.h"
6854 #include "proc.h"
6855 #include "defs.h"
6856 #include "elf.h"
6857 #include "riscv.h"
6858 
6859 int
6860 exec(char *path, char **argv)
6861 {
6862   char *s, *last;
6863   int i, off;
6864   uint argc, sz, sp, ustack[3+MAXARG+1];
6865   struct elfhdr elf;
6866   struct inode *ip;
6867   struct proghdr ph;
6868   pde_t *pgdir, *oldpgdir;
6869   struct proc *curproc = myproc();
6870 
6871   begin_op();
6872 
6873   if((ip = namei(path)) == 0){
6874     end_op();
6875     cprintf("exec: fail\n");
6876     return -1;
6877   }
6878   ilock(ip);
6879   pgdir = 0;
6880 
6881   // Check ELF header
6882   if(readi(ip, (char*)&elf, 0, sizeof(elf)) != sizeof(elf))
6883     goto bad;
6884   if(elf.magic != ELF_MAGIC)
6885     goto bad;
6886 
6887   if((pgdir = setupkvm()) == 0)
6888     goto bad;
6889 
6890   // Load program into memory.
6891   sz = 0;
6892   for(i=0, off=elf.phoff; i<elf.phnum; i++, off+=sizeof(ph)){
6893     if(readi(ip, (char*)&ph, off, sizeof(ph)) != sizeof(ph))
6894       goto bad;
6895     if(ph.type != ELF_PROG_LOAD)
6896       continue;
6897     if(ph.memsz < ph.filesz)
6898       goto bad;
6899     if(ph.vaddr + ph.memsz < ph.vaddr)
6900       goto bad;
6901     if((sz = allocuvm(pgdir, sz, ph.vaddr + ph.memsz)) == 0)
6902       goto bad;
6903     if(ph.vaddr % PGSIZE != 0)
6904       goto bad;
6905     if(loaduvm(pgdir, (char*)ph.vaddr, ip, ph.off, ph.filesz) < 0)
6906       goto bad;
6907   }
6908   iunlockput(ip);
6909   end_op();
6910   ip = 0;
6911 
6912   // Allocate two pages at the next page boundary.
6913   // Make the first inaccessible.  Use the second as the user stack.
6914   sz = PGROUNDUP(sz);
6915   if((sz = allocuvm(pgdir, sz, sz + 2*PGSIZE)) == 0)
6916     goto bad;
6917   clearpteu(pgdir, (char*)(sz - 2*PGSIZE));
6918   sp = sz;
6919 
6920   // Push argument strings, prepare rest of stack in ustack.
6921   for(argc = 0; argv[argc]; argc++) {
6922     if(argc >= MAXARG)
6923       goto bad;
6924     sp = (sp - (strlen(argv[argc]) + 1)) & ~3;
6925     if(copyout(pgdir, sp, argv[argc], strlen(argv[argc]) + 1) < 0)
6926       goto bad;
6927     ustack[3+argc] = sp;
6928   }
6929   ustack[3+argc] = 0;
6930 
6931   ustack[0] = 0xffffffff;  // fake return PC
6932   ustack[1] = argc;
6933   ustack[2] = sp - (argc+1)*4;  // argv pointer
6934 
6935   sp -= (3+argc+1) * 4;
6936   if(copyout(pgdir, sp, ustack, (3+argc+1)*4) < 0)
6937     goto bad;
6938 
6939   // Save program name for debugging.
6940   for(last=s=path; *s; s++)
6941     if(*s == '/')
6942       last = s+1;
6943   safestrcpy(curproc->name, last, sizeof(curproc->name));
6944 
6945   // Commit to the user image.
6946   oldpgdir = curproc->pgdir;
6947   curproc->pgdir = pgdir;
6948   curproc->sz = sz;
6949   curproc->tf->mepc = elf.entry;  // main
6950   curproc->tf->sp = sp;
6951 
6952   curproc->tf->gp = sp;
6953   curproc->tf->t5 = ustack[1];
6954   curproc->tf->t6 = ustack[2];
6955 
6956   switchuvm(curproc);
6957   freevm(oldpgdir);
6958   return 0;
6959 
6960  bad:
6961   if(pgdir)
6962     freevm(pgdir);
6963   if(ip){
6964     iunlockput(ip);
6965     end_op();
6966   }
6967   return -1;
6968 }
6969 
6970 
6971 
6972 
6973 
6974 
6975 
6976 
6977 
6978 
6979 
6980 
6981 
6982 
6983 
6984 
6985 
6986 
6987 
6988 
6989 
6990 
6991 
6992 
6993 
6994 
6995 
6996 
6997 
6998 
6999 
