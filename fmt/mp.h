7300 // See MultiProcessor Specification Version 1.[14]
7301 
7302 struct mp {             // floating pointer
7303   uchar signature[4];           // "_MP_"
7304   void *physaddr;               // phys addr of MP config table
7305   uchar length;                 // 1
7306   uchar specrev;                // [14]
7307   uchar checksum;               // all bytes must add up to 0
7308   uchar type;                   // MP system config type
7309   uchar imcrp;
7310   uchar reserved[3];
7311 };
7312 
7313 struct mpconf {         // configuration table header
7314   uchar signature[4];           // "PCMP"
7315   ushort length;                // total table length
7316   uchar version;                // [14]
7317   uchar checksum;               // all bytes must add up to 0
7318   uchar product[20];            // product id
7319   uint *oemtable;               // OEM table pointer
7320   ushort oemlength;             // OEM table length
7321   ushort entry;                 // entry count
7322   uint *lapicaddr;              // address of local APIC
7323   ushort xlength;               // extended table length
7324   uchar xchecksum;              // extended table checksum
7325   uchar reserved;
7326 };
7327 
7328 struct mpproc {         // processor table entry
7329   uchar type;                   // entry type (0)
7330   uchar apicid;                 // local APIC id
7331   uchar version;                // local APIC verison
7332   uchar flags;                  // CPU flags
7333     #define MPBOOT 0x02           // This proc is the bootstrap processor.
7334   uchar signature[4];           // CPU signature
7335   uint feature;                 // feature flags from CPUID instruction
7336   uchar reserved[8];
7337 };
7338 
7339 struct mpioapic {       // I/O APIC table entry
7340   uchar type;                   // entry type (2)
7341   uchar apicno;                 // I/O APIC id
7342   uchar version;                // I/O APIC version
7343   uchar flags;                  // I/O APIC flags
7344   uint *addr;                  // I/O APIC address
7345 };
7346 
7347 
7348 
7349 
7350 // Table entry types
7351 #define MPPROC    0x00  // One per processor
7352 #define MPBUS     0x01  // One per bus
7353 #define MPIOAPIC  0x02  // One per I/O APIC
7354 #define MPIOINTR  0x03  // One per bus interrupt source
7355 #define MPLINTR   0x04  // One per system interrupt source
7356 
7357 
7358 
7359 
7360 
7361 
7362 
7363 
7364 
7365 
7366 
7367 
7368 
7369 
7370 
7371 
7372 
7373 
7374 
7375 
7376 
7377 
7378 
7379 
7380 
7381 
7382 
7383 
7384 
7385 
7386 
7387 
7388 
7389 
7390 
7391 
7392 
7393 
7394 
7395 
7396 
7397 
7398 
7399 
7400 // Blank page.
7401 
7402 
7403 
7404 
7405 
7406 
7407 
7408 
7409 
7410 
7411 
7412 
7413 
7414 
7415 
7416 
7417 
7418 
7419 
7420 
7421 
7422 
7423 
7424 
7425 
7426 
7427 
7428 
7429 
7430 
7431 
7432 
7433 
7434 
7435 
7436 
7437 
7438 
7439 
7440 
7441 
7442 
7443 
7444 
7445 
7446 
7447 
7448 
7449 
