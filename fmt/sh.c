8900 // Shell.
8901 
8902 #include "types.h"
8903 #include "user.h"
8904 #include "fcntl.h"
8905 
8906 // Parsed command representation
8907 #define EXEC  1
8908 #define REDIR 2
8909 #define PIPE  3
8910 #define LIST  4
8911 #define BACK  5
8912 
8913 #define MAXARGS 10
8914 
8915 struct cmd {
8916   int type;
8917 };
8918 
8919 struct execcmd {
8920   int type;
8921   char *argv[MAXARGS];
8922   char *eargv[MAXARGS];
8923 };
8924 
8925 struct redircmd {
8926   int type;
8927   struct cmd *cmd;
8928   char *file;
8929   char *efile;
8930   int mode;
8931   int fd;
8932 };
8933 
8934 struct pipecmd {
8935   int type;
8936   struct cmd *left;
8937   struct cmd *right;
8938 };
8939 
8940 struct listcmd {
8941   int type;
8942   struct cmd *left;
8943   struct cmd *right;
8944 };
8945 
8946 struct backcmd {
8947   int type;
8948   struct cmd *cmd;
8949 };
8950 int fork1(void);  // Fork but panics on failure.
8951 void panic(char*);
8952 struct cmd *parsecmd(char*);
8953 
8954 // Execute cmd.  Never returns.
8955 void
8956 runcmd(struct cmd *cmd)
8957 {
8958   int p[2];
8959   struct backcmd *bcmd;
8960   struct execcmd *ecmd;
8961   struct listcmd *lcmd;
8962   struct pipecmd *pcmd;
8963   struct redircmd *rcmd;
8964 
8965   if(cmd == 0)
8966     exit();
8967 
8968   switch(cmd->type){
8969   default:
8970     panic("runcmd");
8971 
8972   case EXEC:
8973     ecmd = (struct execcmd*)cmd;
8974     if(ecmd->argv[0] == 0)
8975       exit();
8976     exec(ecmd->argv[0], ecmd->argv);
8977     printf(2, "exec %s failed\n", ecmd->argv[0]);
8978     break;
8979 
8980   case REDIR:
8981     rcmd = (struct redircmd*)cmd;
8982     close(rcmd->fd);
8983     if(open(rcmd->file, rcmd->mode) < 0){
8984       printf(2, "open %s failed\n", rcmd->file);
8985       exit();
8986     }
8987     runcmd(rcmd->cmd);
8988     break;
8989 
8990   case LIST:
8991     lcmd = (struct listcmd*)cmd;
8992     if(fork1() == 0)
8993       runcmd(lcmd->left);
8994     wait();
8995     runcmd(lcmd->right);
8996     break;
8997 
8998 
8999 
9000   case PIPE:
9001     pcmd = (struct pipecmd*)cmd;
9002     if(pipe(p) < 0)
9003       panic("pipe");
9004     if(fork1() == 0){
9005       close(1);
9006       dup(p[1]);
9007       close(p[0]);
9008       close(p[1]);
9009       runcmd(pcmd->left);
9010     }
9011     if(fork1() == 0){
9012       close(0);
9013       dup(p[0]);
9014       close(p[0]);
9015       close(p[1]);
9016       runcmd(pcmd->right);
9017     }
9018     close(p[0]);
9019     close(p[1]);
9020     wait();
9021     wait();
9022     break;
9023 
9024   case BACK:
9025     bcmd = (struct backcmd*)cmd;
9026     if(fork1() == 0)
9027       runcmd(bcmd->cmd);
9028     break;
9029   }
9030   exit();
9031 }
9032 
9033 int
9034 getcmd(char *buf, int nbuf)
9035 {
9036   printf(2, "$ ");
9037   memset(buf, 0, nbuf);
9038   gets(buf, nbuf);
9039   if(buf[0] == 0) // EOF
9040     return -1;
9041   return 0;
9042 }
9043 
9044 
9045 
9046 
9047 
9048 
9049 
9050 int
9051 main(void)
9052 {
9053   static char buf[100];
9054   int fd;
9055 
9056   // Ensure that three file descriptors are open.
9057   while((fd = open("console", O_RDWR)) >= 0){
9058     if(fd >= 3){
9059       close(fd);
9060       break;
9061     }
9062   }
9063 
9064   // Read and run input commands.
9065   while(getcmd(buf, sizeof(buf)) >= 0){
9066     if(buf[0] == 'c' && buf[1] == 'd' && buf[2] == ' '){
9067       // Chdir must be called by the parent, not the child.
9068       buf[strlen(buf)-1] = 0;  // chop \n
9069       if(chdir(buf+3) < 0)
9070         printf(2, "cannot cd %s\n", buf+3);
9071       continue;
9072     }
9073     if(fork1() == 0)
9074       runcmd(parsecmd(buf));
9075     wait();
9076   }
9077   exit();
9078 }
9079 
9080 void
9081 panic(char *s)
9082 {
9083   printf(2, "%s\n", s);
9084   exit();
9085 }
9086 
9087 int
9088 fork1(void)
9089 {
9090   int pid;
9091 
9092   pid = fork();
9093   if(pid == -1)
9094     panic("fork");
9095   return pid;
9096 }
9097 
9098 
9099 
9100 // Constructors
9101 
9102 struct cmd*
9103 execcmd(void)
9104 {
9105   struct execcmd *cmd;
9106 
9107   cmd = malloc(sizeof(*cmd));
9108   memset(cmd, 0, sizeof(*cmd));
9109   cmd->type = EXEC;
9110   return (struct cmd*)cmd;
9111 }
9112 
9113 struct cmd*
9114 redircmd(struct cmd *subcmd, char *file, char *efile, int mode, int fd)
9115 {
9116   struct redircmd *cmd;
9117 
9118   cmd = malloc(sizeof(*cmd));
9119   memset(cmd, 0, sizeof(*cmd));
9120   cmd->type = REDIR;
9121   cmd->cmd = subcmd;
9122   cmd->file = file;
9123   cmd->efile = efile;
9124   cmd->mode = mode;
9125   cmd->fd = fd;
9126   return (struct cmd*)cmd;
9127 }
9128 
9129 struct cmd*
9130 pipecmd(struct cmd *left, struct cmd *right)
9131 {
9132   struct pipecmd *cmd;
9133 
9134   cmd = malloc(sizeof(*cmd));
9135   memset(cmd, 0, sizeof(*cmd));
9136   cmd->type = PIPE;
9137   cmd->left = left;
9138   cmd->right = right;
9139   return (struct cmd*)cmd;
9140 }
9141 
9142 
9143 
9144 
9145 
9146 
9147 
9148 
9149 
9150 struct cmd*
9151 listcmd(struct cmd *left, struct cmd *right)
9152 {
9153   struct listcmd *cmd;
9154 
9155   cmd = malloc(sizeof(*cmd));
9156   memset(cmd, 0, sizeof(*cmd));
9157   cmd->type = LIST;
9158   cmd->left = left;
9159   cmd->right = right;
9160   return (struct cmd*)cmd;
9161 }
9162 
9163 struct cmd*
9164 backcmd(struct cmd *subcmd)
9165 {
9166   struct backcmd *cmd;
9167 
9168   cmd = malloc(sizeof(*cmd));
9169   memset(cmd, 0, sizeof(*cmd));
9170   cmd->type = BACK;
9171   cmd->cmd = subcmd;
9172   return (struct cmd*)cmd;
9173 }
9174 
9175 
9176 
9177 
9178 
9179 
9180 
9181 
9182 
9183 
9184 
9185 
9186 
9187 
9188 
9189 
9190 
9191 
9192 
9193 
9194 
9195 
9196 
9197 
9198 
9199 
9200 // Parsing
9201 
9202 char whitespace[] = " \t\r\n\v";
9203 char symbols[] = "<|>&;()";
9204 
9205 int
9206 gettoken(char **ps, char *es, char **q, char **eq)
9207 {
9208   char *s;
9209   int ret;
9210 
9211   s = *ps;
9212   while(s < es && strchr(whitespace, *s))
9213     s++;
9214   if(q)
9215     *q = s;
9216   ret = *s;
9217   switch(*s){
9218   case 0:
9219     break;
9220   case '|':
9221   case '(':
9222   case ')':
9223   case ';':
9224   case '&':
9225   case '<':
9226     s++;
9227     break;
9228   case '>':
9229     s++;
9230     if(*s == '>'){
9231       ret = '+';
9232       s++;
9233     }
9234     break;
9235   default:
9236     ret = 'a';
9237     while(s < es && !strchr(whitespace, *s) && !strchr(symbols, *s))
9238       s++;
9239     break;
9240   }
9241   if(eq)
9242     *eq = s;
9243 
9244   while(s < es && strchr(whitespace, *s))
9245     s++;
9246   *ps = s;
9247   return ret;
9248 }
9249 
9250 int
9251 peek(char **ps, char *es, char *toks)
9252 {
9253   char *s;
9254 
9255   s = *ps;
9256   while(s < es && strchr(whitespace, *s))
9257     s++;
9258   *ps = s;
9259   return *s && strchr(toks, *s);
9260 }
9261 
9262 struct cmd *parseline(char**, char*);
9263 struct cmd *parsepipe(char**, char*);
9264 struct cmd *parseexec(char**, char*);
9265 struct cmd *nulterminate(struct cmd*);
9266 
9267 struct cmd*
9268 parsecmd(char *s)
9269 {
9270   char *es;
9271   struct cmd *cmd;
9272 
9273   es = s + strlen(s);
9274   cmd = parseline(&s, es);
9275   peek(&s, es, "");
9276   if(s != es){
9277     printf(2, "leftovers: %s\n", s);
9278     panic("syntax");
9279   }
9280   nulterminate(cmd);
9281   return cmd;
9282 }
9283 
9284 struct cmd*
9285 parseline(char **ps, char *es)
9286 {
9287   struct cmd *cmd;
9288 
9289   cmd = parsepipe(ps, es);
9290   while(peek(ps, es, "&")){
9291     gettoken(ps, es, 0, 0);
9292     cmd = backcmd(cmd);
9293   }
9294   if(peek(ps, es, ";")){
9295     gettoken(ps, es, 0, 0);
9296     cmd = listcmd(cmd, parseline(ps, es));
9297   }
9298   return cmd;
9299 }
9300 struct cmd*
9301 parsepipe(char **ps, char *es)
9302 {
9303   struct cmd *cmd;
9304 
9305   cmd = parseexec(ps, es);
9306   if(peek(ps, es, "|")){
9307     gettoken(ps, es, 0, 0);
9308     cmd = pipecmd(cmd, parsepipe(ps, es));
9309   }
9310   return cmd;
9311 }
9312 
9313 struct cmd*
9314 parseredirs(struct cmd *cmd, char **ps, char *es)
9315 {
9316   int tok;
9317   char *q, *eq;
9318 
9319   while(peek(ps, es, "<>")){
9320     tok = gettoken(ps, es, 0, 0);
9321     if(gettoken(ps, es, &q, &eq) != 'a')
9322       panic("missing file for redirection");
9323     switch(tok){
9324     case '<':
9325       cmd = redircmd(cmd, q, eq, O_RDONLY, 0);
9326       break;
9327     case '>':
9328       cmd = redircmd(cmd, q, eq, O_WRONLY|O_CREATE, 1);
9329       break;
9330     case '+':  // >>
9331       cmd = redircmd(cmd, q, eq, O_WRONLY|O_CREATE, 1);
9332       break;
9333     }
9334   }
9335   return cmd;
9336 }
9337 
9338 
9339 
9340 
9341 
9342 
9343 
9344 
9345 
9346 
9347 
9348 
9349 
9350 struct cmd*
9351 parseblock(char **ps, char *es)
9352 {
9353   struct cmd *cmd;
9354 
9355   if(!peek(ps, es, "("))
9356     panic("parseblock");
9357   gettoken(ps, es, 0, 0);
9358   cmd = parseline(ps, es);
9359   if(!peek(ps, es, ")"))
9360     panic("syntax - missing )");
9361   gettoken(ps, es, 0, 0);
9362   cmd = parseredirs(cmd, ps, es);
9363   return cmd;
9364 }
9365 
9366 struct cmd*
9367 parseexec(char **ps, char *es)
9368 {
9369   char *q, *eq;
9370   int tok, argc;
9371   struct execcmd *cmd;
9372   struct cmd *ret;
9373 
9374   if(peek(ps, es, "("))
9375     return parseblock(ps, es);
9376 
9377   ret = execcmd();
9378   cmd = (struct execcmd*)ret;
9379 
9380   argc = 0;
9381   ret = parseredirs(ret, ps, es);
9382   while(!peek(ps, es, "|)&;")){
9383     if((tok=gettoken(ps, es, &q, &eq)) == 0)
9384       break;
9385     if(tok != 'a')
9386       panic("syntax");
9387     cmd->argv[argc] = q;
9388     cmd->eargv[argc] = eq;
9389     argc++;
9390     if(argc >= MAXARGS)
9391       panic("too many args");
9392     ret = parseredirs(ret, ps, es);
9393   }
9394   cmd->argv[argc] = 0;
9395   cmd->eargv[argc] = 0;
9396   return ret;
9397 }
9398 
9399 
9400 // NUL-terminate all the counted strings.
9401 struct cmd*
9402 nulterminate(struct cmd *cmd)
9403 {
9404   int i;
9405   struct backcmd *bcmd;
9406   struct execcmd *ecmd;
9407   struct listcmd *lcmd;
9408   struct pipecmd *pcmd;
9409   struct redircmd *rcmd;
9410 
9411   if(cmd == 0)
9412     return 0;
9413 
9414   switch(cmd->type){
9415   case EXEC:
9416     ecmd = (struct execcmd*)cmd;
9417     for(i=0; ecmd->argv[i]; i++)
9418       *ecmd->eargv[i] = 0;
9419     break;
9420 
9421   case REDIR:
9422     rcmd = (struct redircmd*)cmd;
9423     nulterminate(rcmd->cmd);
9424     *rcmd->efile = 0;
9425     break;
9426 
9427   case PIPE:
9428     pcmd = (struct pipecmd*)cmd;
9429     nulterminate(pcmd->left);
9430     nulterminate(pcmd->right);
9431     break;
9432 
9433   case LIST:
9434     lcmd = (struct listcmd*)cmd;
9435     nulterminate(lcmd->left);
9436     nulterminate(lcmd->right);
9437     break;
9438 
9439   case BACK:
9440     bcmd = (struct backcmd*)cmd;
9441     nulterminate(bcmd->cmd);
9442     break;
9443   }
9444   return cmd;
9445 }
9446 
9447 
9448 
9449 
