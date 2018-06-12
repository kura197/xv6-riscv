1350 // Mutual exclusion lock.
1351 struct spinlock {
1352   uint locked;       // Is the lock held?
1353 
1354   // For debugging:
1355   char *name;        // Name of lock.
1356   struct cpu *cpu;   // The cpu holding the lock.
1357   uint pcs[10];      // The call stack (an array of program counters)
1358                      // that locked the lock.
1359 };
1360 
1361 
1362 
1363 
1364 
1365 
1366 
1367 
1368 
1369 
1370 
1371 
1372 
1373 
1374 
1375 
1376 
1377 
1378 
1379 
1380 
1381 
1382 
1383 
1384 
1385 
1386 
1387 
1388 
1389 
1390 
1391 
1392 
1393 
1394 
1395 
1396 
1397 
1398 
1399 
