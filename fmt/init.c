8850 // init: The initial user-level program
8851 
8852 #include "types.h"
8853 #include "stat.h"
8854 #include "user.h"
8855 #include "fcntl.h"
8856 
8857 char *argv[] = { "sh", 0 };
8858 
8859 int
8860 main(void)
8861 {
8862   int pid, wpid;
8863 
8864   if(open("console", O_RDWR) < 0){
8865     mknod("console", 1, 1);
8866     open("console", O_RDWR);
8867   }
8868   dup(0);  // stdout
8869   dup(0);  // stderr
8870 
8871   for(;;){
8872     printf(1, "init: starting sh\n");
8873     pid = fork();
8874     if(pid < 0){
8875       printf(1, "init: fork failed\n");
8876       exit();
8877     }
8878     if(pid == 0){
8879       exec("sh", argv);
8880       printf(1, "init: exec sh failed\n");
8881       exit();
8882     }
8883     while((wpid=wait()) >= 0 && wpid != pid)
8884       printf(1, "zombie!\n");
8885   }
8886 }
8887 
8888 
8889 
8890 
8891 
8892 
8893 
8894 
8895 
8896 
8897 
8898 
8899 
