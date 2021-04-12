#ifndef ASM
#define ASM

#include <stdint.h>

uint8_t curcpu(void);
uint64_t getel(void);
uint64_t read_ttbr1(void);
__attribute__ ((noreturn)) void spin_forever(void);
void tlb_flush(void);

#endif
