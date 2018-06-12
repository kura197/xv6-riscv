1050 #include "types.h"
1051 #include "defs.h"
1052 #include "param.h"
1053 #include "memlayout.h"
1054 #include "mmu.h"
1055 #include "proc.h"
1056 #include "riscv.h"
1057 
1058 //static void startothers(void);
1059 static void mpmain(void)  __attribute__((noreturn));
1060 extern pde_t *kpgdir;
1061 extern char end[]; // first address after kernel loaded from ELF file
1062 
1063 // Bootstrap processor starts running C code here.
1064 // Allocate a real stack and switch to it, first
1065 // doing some setup required for memory allocator to work.
1066 int
1067 main(void)
1068 {
1069 //kalloc.c:4MB分を割り当て?
1070   kinit1(end, P2V(4*1024*1024)); // phys page allocator
1071 //vm.c:kmapをcr3へ
1072   kvmalloc();      // kernel page table
1073 //mp.c: ?? skip
1074   //mpinit();        // detect other processors
1075 //lapic.c:割り込み　各種初期化
1076   lapicinit();     // interrupt controller
1077 //vm.c:セグメント初期化
1078  // seginit();       // segment descriptors
1079 //picirq.c:disable interrupt
1080   //picinit();       // disable pic
1081 //ioapic.c:disable? interrupt
1082   //ioapicinit();    // another interrupt controller
1083 //console.c: enable console functions & interrupt
1084   consoleinit();   // console hardware
1085 //uart.c:uart setup, enable interrupt
1086   uartinit();      // serial port
1087 //proc.c:pnit() -> spinlock:initlock():
1088 //set up ptable
1089   pinit();         // process table
1090 //make interrupt & trap vectors
1091   tvinit();        // trap vectors
1092 //ide cache
1093   binit();         // buffer cache
1094 //set up ftable
1095   fileinit();      // file table
1096 //ide.c skip
1097   ideinit();       // disk
1098 //other cpu
1099   //startothers();   // start other processors
1100 //kalloc.c: PHYSTOPまでのメモリ割り当て
1101   kinit2(P2V(4*1024*1024), P2V(PHYSTOP)); // must come after startothers()
1102 //main.c: initcode.Sをva 0番地にセットしたプロセスを用意?
1103   userinit();      // first user process
1104 //main.c:
1105   mpmain();        // finish this processor's setup
1106 }
1107 
1108 // Other CPUs jump here from entryother.S.
1109 // 必要なし??
1110 /*
1111 static void
1112 mpenter(void)
1113 {
1114   switchkvm();
1115   seginit();
1116   lapicinit();
1117   mpmain();
1118 }
1119 */
1120 
1121 // Common CPU setup code.
1122 static void
1123 mpmain(void)
1124 {
1125   cprintf("cpu%d: starting %d\n", cpuid(), cpuid());
1126   //lidt
1127   idtinit();       // load idt register
1128   xchg(&(mycpu()->started), 1); // tell startothers() we're up
1129   //proc.c
1130   scheduler();     // start running processes
1131 }
1132 
1133 pde_t entrypgdir[];  // For entry.S
1134 
1135 // Start the non-boot (AP) processors.
1136 // 必要なし??
1137 /*
1138 static void
1139 startothers(void)
1140 {
1141   extern uchar _binary_entryother_start[], _binary_entryother_size[];
1142   uchar *code;
1143   struct cpu *c;
1144   char *stack;
1145 
1146   // Write entry code to unused memory at 0x7000.
1147   // The linker has placed the image of entryother.S in
1148   // _binary_entryother_start.
1149   code = P2V(0x7000);
1150   memmove(code, _binary_entryother_start, (uint)_binary_entryother_size);
1151 
1152   for(c = cpus; c < cpus+ncpu; c++){
1153     if(c == mycpu())  // We've started already.
1154       continue;
1155 
1156     // Tell entryother.S what stack to use, where to enter, and what
1157     // pgdir to use. We cannot use kpgdir yet, because the AP processor
1158     // is running in low  memory, so we use entrypgdir for the APs too.
1159     stack = kalloc();
1160     *(void**)(code-4) = stack + KSTACKSIZE;
1161     *(void**)(code-8) = mpenter;
1162     *(int**)(code-12) = (void *) V2P(entrypgdir);
1163 
1164     lapicstartap(c->apicid, V2P(code));
1165 
1166     // wait for cpu to finish mpmain()
1167     while(c->started == 0)
1168       ;
1169   }
1170 }
1171 */
1172 
1173 // The boot page table used in entry.S and entryother.S.
1174 // Page directories (and page tables) must start on page boundaries,
1175 // hence the __aligned__ attribute.
1176 // PTE_PS in a page directory entry enables 4Mbyte pages.
1177 
1178 __attribute__((__aligned__(PGSIZE)))
1179 pde_t entrypgdir[NPDENTRIES] = {
1180   // Map VA's [0, 4MB) to PA's [0, 4MB)
1181  // [0] = (0) | PTE_V | PTE_W | PTE_PS,
1182   [0] = (0) | PTE_V | PTE_R | PTE_X | PTE_W,
1183   // Map VA's [KERNBASE, KERNBASE+4MB) to PA's [0, 4MB)
1184   //[KERNBASE>>PDXSHIFT] = (0) | PTE_V | PTE_W | PTE_PS,
1185   [KERNBASE>>PDXSHIFT] = (0) | PTE_V | PTE_R | PTE_X | PTE_W,
1186 };
1187 
1188 
1189 
1190 
1191 
1192 
1193 
1194 
1195 
1196 
1197 
1198 
1199 
1200 // Blank page.
1201 
1202 
1203 
1204 
1205 
1206 
1207 
1208 
1209 
1210 
1211 
1212 
1213 
1214 
1215 
1216 
1217 
1218 
1219 
1220 
1221 
1222 
1223 
1224 
1225 
1226 
1227 
1228 
1229 
1230 
1231 
1232 
1233 
1234 
1235 
1236 
1237 
1238 
1239 
1240 
1241 
1242 
1243 
1244 
1245 
1246 
1247 
1248 
1249 
1250 // Blank page.
1251 
1252 
1253 
1254 
1255 
1256 
1257 
1258 
1259 
1260 
1261 
1262 
1263 
1264 
1265 
1266 
1267 
1268 
1269 
1270 
1271 
1272 
1273 
1274 
1275 
1276 
1277 
1278 
1279 
1280 
1281 
1282 
1283 
1284 
1285 
1286 
1287 
1288 
1289 
1290 
1291 
1292 
1293 
1294 
1295 
1296 
1297 
1298 
1299 
1300 // Blank page.
1301 
1302 
1303 
1304 
1305 
1306 
1307 
1308 
1309 
1310 
1311 
1312 
1313 
1314 
1315 
1316 
1317 
1318 
1319 
1320 
1321 
1322 
1323 
1324 
1325 
1326 
1327 
1328 
1329 
1330 
1331 
1332 
1333 
1334 
1335 
1336 
1337 
1338 
1339 
1340 
1341 
1342 
1343 
1344 
1345 
1346 
1347 
1348 
1349 
