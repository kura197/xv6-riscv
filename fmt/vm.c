1550 #include "param.h"
1551 #include "types.h"
1552 #include "defs.h"
1553 #include "riscv.h"
1554 #include "memlayout.h"
1555 #include "mmu.h"
1556 #include "proc.h"
1557 #include "elf.h"
1558 
1559 extern char data[];  // defined by kernel.ld
1560 pde_t *kpgdir;  // for use in scheduler()
1561 
1562 // Set up CPU's kernel segment descriptors.
1563 // Run once on entry on each CPU.
1564 /*
1565 void
1566 seginit(void)
1567 {
1568   struct cpu *c;
1569 
1570   // Map "logical" addresses to virtual addresses using identity map.
1571   // Cannot share a CODE descriptor for both kernel and user
1572   // because it would have to have DPL_USR, but the CPU forbids
1573   // an interrupt from CPL=0 to DPL=3.
1574   c = &cpus[cpuid()];
1575   c->gdt[SEG_KCODE] = SEG(STA_X|STA_R, 0, 0xffffffff, 0);
1576   c->gdt[SEG_KDATA] = SEG(STA_W, 0, 0xffffffff, 0);
1577   c->gdt[SEG_UCODE] = SEG(STA_X|STA_R, 0, 0xffffffff, DPL_USER);
1578   c->gdt[SEG_UDATA] = SEG(STA_W, 0, 0xffffffff, DPL_USER);
1579   lgdt(c->gdt, sizeof(c->gdt));
1580 }
1581 */
1582 
1583 // Return the address of the PTE in page table pgdir
1584 // that corresponds to virtual address va.  If alloc!=0,
1585 // create any required page table pages.
1586 static pte_t *
1587 walkpgdir(pde_t *pgdir, const void *va, int alloc)
1588 {
1589   pde_t *pde;
1590   pte_t *pgtab;
1591 
1592   pde = &pgdir[PDX(va)];
1593   if(*pde & PTE_V){
1594     pgtab = (pte_t*)P2V(PTE_ADDR(*pde));
1595   } else {
1596     if(!alloc || (pgtab = (pte_t*)kalloc()) == 0)
1597       return 0;
1598     // Make sure all those PTE_P bits are zero.
1599     memset(pgtab, 0, PGSIZE);
1600     // The permissions here are overly generous, but they can
1601     // be further restricted by the permissions in the page table
1602     // entries, if necessary.
1603     //*pde = (V2P(pgtab) >> 2)  | PTE_V ;
1604     *pde = (V2P(pgtab) >> 2)  | PTE_V ;
1605     //*pde = (V2P(pgtab))  | PTE_V ;
1606   }
1607   return &pgtab[PTX(va)];
1608 }
1609 
1610 
1611 // Create PTEs for virtual addresses starting at va that refer to
1612 // physical addresses starting at pa. va and size might not
1613 // be page-aligned.
1614 static int
1615 mappages(pde_t *pgdir, void *va, uint size, uint pa, int perm)
1616 {
1617   char *a, *last;
1618   pte_t *pte;
1619 
1620   //size = 0x6f6000;
1621   //size = 0x26a001;
1622   a = (char*)PGROUNDDOWN((uint)va);
1623   last = (char*)PGROUNDDOWN(((uint)va) + size - 1);
1624   for(;;){
1625 	//vm.c:virtual addressがaであるページを指すPTEを割り当てる
1626 	if((pte = walkpgdir(pgdir, a, 1)) == 0)
1627 		  return -1;
1628     if(*pte & PTE_V)
1629       panic("remap");
1630     *pte = (pa >> 2) | perm | PTE_V;
1631     if(a == last){
1632       break;
1633 	}
1634     a += PGSIZE;
1635     pa += PGSIZE;
1636   }
1637   return 0;
1638 }
1639 
1640 
1641 
1642 
1643 
1644 
1645 
1646 
1647 
1648 
1649 
1650 // There is one page table per process, plus one that's used when
1651 // a CPU is not running any process (kpgdir). The kernel uses the
1652 // current process's page table during system calls and interrupts;
1653 // page protection bits prevent user code from using the kernel's
1654 // mappings.
1655 //
1656 // setupkvm() and exec() set up every page table like this:
1657 //
1658 //   0..KERNBASE: user memory (text+data+stack+heap), mapped to
1659 //                phys memory allocated by the kernel
1660 //   KERNBASE..KERNBASE+EXTMEM: mapped to 0..EXTMEM (for I/O space)
1661 //   KERNBASE+EXTMEM..data: mapped to EXTMEM..V2P(data)
1662 //                for the kernel's instructions and r/o data
1663 //   data..KERNBASE+PHYSTOP: mapped to V2P(data)..PHYSTOP,
1664 //                                  rw data + free physical memory
1665 //   0xfe000000..0: mapped direct (devices such as ioapic)
1666 //
1667 // The kernel allocates physical memory for its heap and for user memory
1668 // between V2P(end) and the end of physical memory (PHYSTOP)
1669 // (directly addressable from end..P2V(PHYSTOP)).
1670 
1671 // This table defines the kernel's mappings, which are present in
1672 // every process's page table.
1673 static struct kmap {
1674   void *virt;
1675   uint phys_start;
1676   uint phys_end;
1677   int perm;
1678 } kmap[] = {
1679  { (void*)KERNBASE, 0,             EXTMEM,    PTE_W | PTE_R}, // I/O space
1680  { (void*)KERNLINK, V2P(KERNLINK), V2P(data), PTE_X | PTE_R},     // kern text+rodata
1681  { (void*)data,     V2P(data),     PHYSTOP,   PTE_W | PTE_R}, // kern data+memory
1682  { (void*)DEVSPACE, DEVSPACE,      0,         PTE_W | PTE_R}, // more devices
1683 };
1684 
1685 // Set up kernel part of a page table.
1686 pde_t*
1687 setupkvm(void)
1688 {
1689   pde_t *pgdir;
1690   struct kmap *k;
1691 
1692   //kalloc.c:kmemの次の空いている4096byteメモリを取得
1693   if((pgdir = (pde_t*)kalloc()) == 0)
1694     return 0;
1695   memset(pgdir, 0, PGSIZE);
1696   if (P2V(PHYSTOP) > (void*)DEVSPACE)
1697     panic("PHYSTOP too high");
1698   for(k = kmap; k < &kmap[NELEM(kmap)]; k++)
1699 	//vm.c:物理メモリphys_startに対応する、仮想メモリvirtを指すPTEをend-start分pgdirに追加
1700     if(mappages(pgdir, k->virt, k->phys_end - k->phys_start,
1701                 (uint)k->phys_start, k->perm) < 0) {
1702       freevm(pgdir);
1703       return 0;
1704     }
1705   return pgdir;
1706 }
1707 
1708 // Allocate one page table for the machine for the kernel address
1709 // space for scheduler processes.
1710 void
1711 kvmalloc(void)
1712 {
1713   //kmapをvirtual addressへセット
1714   kpgdir = setupkvm();
1715   //cr3にPTEセット
1716   switchkvm();
1717 }
1718 
1719 // Switch h/w page table register to the kernel-only page table,
1720 // for when no process is running.
1721 void
1722 switchkvm(void)
1723 {
1724   lcr3(V2P(kpgdir));   // switch to the kernel page table
1725 }
1726 
1727 // Switch TSS and h/w page table to correspond to process p.
1728 void
1729 switchuvm(struct proc *p)
1730 {
1731   if(p == 0)
1732     panic("switchuvm: no process");
1733   if(p->kstack == 0)
1734     panic("switchuvm: no kstack");
1735   if(p->pgdir == 0)
1736     panic("switchuvm: no pgdir");
1737 
1738   pushcli();
1739   //あとで
1740   /*
1741   mycpu()->gdt[SEG_TSS] = SEG16(STS_T32A, &mycpu()->ts,
1742                                 sizeof(mycpu()->ts)-1, 0);
1743   mycpu()->gdt[SEG_TSS].s = 0;
1744   mycpu()->ts.ss0 = SEG_KDATA << 3;
1745   mycpu()->ts.esp0 = (uint)p->kstack + KSTACKSIZE;
1746   // setting IOPL=0 in eflags *and* iomb beyond the tss segment limit
1747   // forbids I/O instructions (e.g., inb and outb) from user space
1748   mycpu()->ts.iomb = (ushort) 0xFFFF;
1749   ltr(SEG_TSS << 3);
1750   */
1751   //mycpu()->ksp = (uint)p->kstack + KSTACKSIZE;
1752   write_scratch((uint)p->kstack + KSTACKSIZE);
1753   lcr3(V2P(p->pgdir));  // switch to process's address space
1754   popcli();
1755 }
1756 
1757 // Load the initcode into address 0 of pgdir.
1758 // sz must be less than a page.
1759 void
1760 inituvm(pde_t *pgdir, char *init, uint sz)
1761 {
1762   char *mem;
1763 
1764   if(sz >= PGSIZE)
1765     panic("inituvm: more than a page");
1766   mem = kalloc();
1767   memset(mem, 0, PGSIZE);
1768   mappages(pgdir, 0, PGSIZE, V2P(mem), PTE_W|PTE_U|PTE_R|PTE_X);
1769   memmove(mem, init, sz);
1770 }
1771 
1772 // Load a program segment into pgdir.  addr must be page-aligned
1773 // and the pages from addr to addr+sz must already be mapped.
1774 int
1775 loaduvm(pde_t *pgdir, char *addr, struct inode *ip, uint offset, uint sz)
1776 {
1777   uint i, pa, n;
1778   pte_t *pte;
1779 
1780   if((uint) addr % PGSIZE != 0)
1781     panic("loaduvm: addr must be page aligned");
1782   for(i = 0; i < sz; i += PGSIZE){
1783     if((pte = walkpgdir(pgdir, addr+i, 0)) == 0)
1784       panic("loaduvm: address should exist");
1785     pa = PTE_ADDR(*pte);
1786     if(sz - i < PGSIZE)
1787       n = sz - i;
1788     else
1789       n = PGSIZE;
1790     if(readi(ip, P2V(pa), offset+i, n) != n)
1791       return -1;
1792   }
1793   return 0;
1794 }
1795 
1796 
1797 
1798 
1799 
1800 // Allocate page tables and physical memory to grow process from oldsz to
1801 // newsz, which need not be page aligned.  Returns new size or 0 on error.
1802 int
1803 allocuvm(pde_t *pgdir, uint oldsz, uint newsz)
1804 {
1805   char *mem;
1806   uint a;
1807 
1808   if(newsz >= KERNBASE)
1809     return 0;
1810   if(newsz < oldsz)
1811     return oldsz;
1812 
1813   a = PGROUNDUP(oldsz);
1814   for(; a < newsz; a += PGSIZE){
1815     mem = kalloc();
1816     if(mem == 0){
1817       cprintf("allocuvm out of memory\n");
1818       deallocuvm(pgdir, newsz, oldsz);
1819       return 0;
1820     }
1821     memset(mem, 0, PGSIZE);
1822     if(mappages(pgdir, (char*)a, PGSIZE, V2P(mem), PTE_W|PTE_U|PTE_R|PTE_X) < 0){
1823       cprintf("allocuvm out of memory (2)\n");
1824       deallocuvm(pgdir, newsz, oldsz);
1825       kfree(mem);
1826       return 0;
1827     }
1828   }
1829   return newsz;
1830 }
1831 
1832 // Deallocate user pages to bring the process size from oldsz to
1833 // newsz.  oldsz and newsz need not be page-aligned, nor does newsz
1834 // need to be less than oldsz.  oldsz can be larger than the actual
1835 // process size.  Returns the new process size.
1836 int
1837 deallocuvm(pde_t *pgdir, uint oldsz, uint newsz)
1838 {
1839   pte_t *pte;
1840   uint a, pa;
1841 
1842   if(newsz >= oldsz)
1843     return oldsz;
1844 
1845   a = PGROUNDUP(newsz);
1846   for(; a  < oldsz; a += PGSIZE){
1847     pte = walkpgdir(pgdir, (char*)a, 0);
1848     if(!pte)
1849       a = PGADDR(PDX(a) + 1, 0, 0) - PGSIZE;
1850     else if((*pte & PTE_V) != 0){
1851       pa = PTE_ADDR(*pte);
1852       if(pa == 0)
1853         panic("kfree");
1854       char *v = P2V(pa);
1855       kfree(v);
1856       *pte = 0;
1857     }
1858   }
1859   return newsz;
1860 }
1861 
1862 // Free a page table and all the physical memory pages
1863 // in the user part.
1864 void
1865 freevm(pde_t *pgdir)
1866 {
1867   uint i;
1868 
1869   if(pgdir == 0)
1870     panic("freevm: no pgdir");
1871   deallocuvm(pgdir, KERNBASE, 0);
1872   for(i = 0; i < NPDENTRIES; i++){
1873     if(pgdir[i] & PTE_V){
1874       char * v = P2V(PTE_ADDR(pgdir[i]));
1875       kfree(v);
1876     }
1877   }
1878   kfree((char*)pgdir);
1879 }
1880 
1881 // Clear PTE_U on a page. Used to create an inaccessible
1882 // page beneath the user stack.
1883 void
1884 clearpteu(pde_t *pgdir, char *uva)
1885 {
1886   pte_t *pte;
1887 
1888   pte = walkpgdir(pgdir, uva, 0);
1889   if(pte == 0)
1890     panic("clearpteu");
1891   *pte &= ~PTE_U;
1892 }
1893 
1894 
1895 
1896 
1897 
1898 
1899 
1900 // Given a parent process's page table, create a copy
1901 // of it for a child.
1902 pde_t*
1903 copyuvm(pde_t *pgdir, uint sz)
1904 {
1905   pde_t *d;
1906   pte_t *pte;
1907   uint pa, i, flags;
1908   char *mem;
1909 
1910   if((d = setupkvm()) == 0)
1911     return 0;
1912   for(i = 0; i < sz; i += PGSIZE){
1913     if((pte = walkpgdir(pgdir, (void *) i, 0)) == 0)
1914       panic("copyuvm: pte should exist");
1915     if(!(*pte & PTE_V))
1916       panic("copyuvm: page not present");
1917     pa = PTE_ADDR(*pte);
1918     flags = PTE_FLAGS(*pte);
1919     if((mem = kalloc()) == 0)
1920       goto bad;
1921     memmove(mem, (char*)P2V(pa), PGSIZE);
1922     if(mappages(d, (void*)i, PGSIZE, V2P(mem), flags) < 0)
1923       goto bad;
1924   }
1925   return d;
1926 
1927 bad:
1928   freevm(d);
1929   return 0;
1930 }
1931 
1932 
1933 
1934 
1935 
1936 
1937 
1938 
1939 
1940 
1941 
1942 
1943 
1944 
1945 
1946 
1947 
1948 
1949 
1950 // Map user virtual address to kernel address.
1951 char*
1952 uva2ka(pde_t *pgdir, char *uva)
1953 {
1954   pte_t *pte;
1955 
1956   pte = walkpgdir(pgdir, uva, 0);
1957   if((*pte & PTE_V) == 0)
1958     return 0;
1959   if((*pte & PTE_U) == 0)
1960     return 0;
1961   return (char*)P2V(PTE_ADDR(*pte));
1962 }
1963 
1964 // Copy len bytes from p to user address va in page table pgdir.
1965 // Most useful when pgdir is not the current page table.
1966 // uva2ka ensures this only works for PTE_U pages.
1967 int
1968 copyout(pde_t *pgdir, uint va, void *p, uint len)
1969 {
1970   char *buf, *pa0;
1971   uint n, va0;
1972 
1973   buf = (char*)p;
1974   while(len > 0){
1975     va0 = (uint)PGROUNDDOWN(va);
1976     pa0 = uva2ka(pgdir, (char*)va0);
1977     if(pa0 == 0)
1978       return -1;
1979     n = PGSIZE - (va - va0);
1980     if(n > len)
1981       n = len;
1982     memmove(pa0 + (va - va0), buf, n);
1983     len -= n;
1984     buf += n;
1985     va = va0 + PGSIZE;
1986   }
1987   return 0;
1988 }
1989 
1990 
1991 
1992 
1993 
1994 
1995 
1996 
1997 
1998 
1999 
2000 // Blank page.
2001 
2002 
2003 
2004 
2005 
2006 
2007 
2008 
2009 
2010 
2011 
2012 
2013 
2014 
2015 
2016 
2017 
2018 
2019 
2020 
2021 
2022 
2023 
2024 
2025 
2026 
2027 
2028 
2029 
2030 
2031 
2032 
2033 
2034 
2035 
2036 
2037 
2038 
2039 
2040 
2041 
2042 
2043 
2044 
2045 
2046 
2047 
2048 
2049 
2050 // Blank page.
2051 
2052 
2053 
2054 
2055 
2056 
2057 
2058 
2059 
2060 
2061 
2062 
2063 
2064 
2065 
2066 
2067 
2068 
2069 
2070 
2071 
2072 
2073 
2074 
2075 
2076 
2077 
2078 
2079 
2080 
2081 
2082 
2083 
2084 
2085 
2086 
2087 
2088 
2089 
2090 
2091 
2092 
2093 
2094 
2095 
2096 
2097 
2098 
2099 
2100 // Blank page.
2101 
2102 
2103 
2104 
2105 
2106 
2107 
2108 
2109 
2110 
2111 
2112 
2113 
2114 
2115 
2116 
2117 
2118 
2119 
2120 
2121 
2122 
2123 
2124 
2125 
2126 
2127 
2128 
2129 
2130 
2131 
2132 
2133 
2134 
2135 
2136 
2137 
2138 
2139 
2140 
2141 
2142 
2143 
2144 
2145 
2146 
2147 
2148 
2149 
