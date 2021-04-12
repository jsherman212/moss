#ifndef VM
#define VM

/* Minimal phystokv to use with virtual addresses that were mapped
 * while we were in EL2. This won't work with virtual memory mapped
 * after pagetables for static memory were set up */
uint64_t linear_phystokv(uint64_t);
uint64_t kvtophys(uint64_t);

#endif
