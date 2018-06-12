1400 // Mutual exclusion spin locks.
1401 
1402 #include "types.h"
1403 #include "defs.h"
1404 #include "param.h"
1405 #include "memlayout.h"
1406 #include "mmu.h"
1407 #include "proc.h"
1408 #include "spinlock.h"
1409 #include "riscv.h"
1410 
1411 void
1412 initlock(struct spinlock *lk, char *name)
1413 {
1414   lk->name = name;
1415   lk->locked = 0;
1416   lk->cpu = 0;
1417 }
1418 
1419 // Acquire the lock.
1420 // Loops (spins) until the lock is acquired.
1421 // Holding a lock for a long time may cause
1422 // other CPUs to waste time spinning to acquire it.
1423 void
1424 acquire(struct spinlock *lk)
1425 {
1426   pushcli(); // disable interrupts to avoid deadlock.
1427   if(holding(lk))
1428     panic("acquire");
1429 
1430   // The xchg is atomic.
1431   while(xchg(&lk->locked, 1) != 0)
1432     ;
1433 
1434   // Tell the C compiler and the processor to not move loads or stores
1435   // past this point, to ensure that the critical section's memory
1436   // references happen after the lock is acquired.
1437   //
1438   //__sync_synchronize();
1439 
1440   // Record info about lock acquisition for debugging.
1441   lk->cpu = mycpu();
1442   getcallerpcs(&lk, lk->pcs);
1443 }
1444 
1445 
1446 
1447 
1448 
1449 
1450 // Release the lock.
1451 void
1452 release(struct spinlock *lk)
1453 {
1454   if(!holding(lk))
1455     panic("release");
1456 
1457   lk->pcs[0] = 0;
1458   lk->cpu = 0;
1459 
1460   // Tell the C compiler and the processor to not move loads or stores
1461   // past this point, to ensure that all the stores in the critical
1462   // section are visible to other cores before the lock is released.
1463   // Both the C compiler and the hardware may re-order loads and
1464   // stores; __sync_synchronize() tells them both not to.
1465   //
1466   //__sync_synchronize();
1467 
1468   // Release the lock, equivalent to lk->locked = 0.
1469   // This code can't use a C assignment, since it might
1470   // not be atomic. A real OS would use C atomics here.
1471   asm volatile("sw zero, %0" : "+m" (lk->locked) : );
1472 
1473   popcli();
1474 }
1475 
1476 // Record the current call stack in pcs[] by following the %ebp chain.
1477 void
1478 getcallerpcs(void *v, uint pcs[])
1479 {
1480 	/*
1481   uint *ebp;
1482   int i;
1483 
1484   ebp = (uint*)v - 2;
1485   for(i = 0; i < 10; i++){
1486     if(ebp == 0 || ebp < (uint*)KERNBASE || ebp == (uint*)0xffffffff)
1487       break;
1488     pcs[i] = ebp[1];     // saved %eip
1489     ebp = (uint*)ebp[0]; // saved %ebp
1490   }
1491   for(; i < 10; i++)
1492     pcs[i] = 0;
1493 	*/
1494 }
1495 
1496 
1497 
1498 
1499 
1500 // Check whether this cpu is holding the lock.
1501 int
1502 holding(struct spinlock *lock)
1503 {
1504   return lock->locked && lock->cpu == mycpu();
1505 }
1506 
1507 
1508 // Pushcli/popcli are like cli/sti except that they are matched:
1509 // it takes two popcli to undo two pushcli.  Also, if interrupts
1510 // are off, then pushcli, popcli leaves them off.
1511 
1512 void
1513 pushcli(void)
1514 {
1515   int eflags;
1516 
1517   //eflags = readeflags();
1518   eflags = read_status();
1519   cli();
1520   if(mycpu()->ncli == 0)
1521     mycpu()->intena = eflags & S_MIE;
1522   mycpu()->ncli += 1;
1523 }
1524 
1525 void
1526 popcli(void)
1527 {
1528   if(read_status()&S_MIE)
1529     panic("popcli - interruptible");
1530   if(--mycpu()->ncli < 0)
1531     panic("popcli");
1532   if(mycpu()->ncli == 0 && mycpu()->intena)
1533     sti();
1534 }
1535 
1536 
1537 
1538 
1539 
1540 
1541 
1542 
1543 
1544 
1545 
1546 
1547 
1548 
1549 
