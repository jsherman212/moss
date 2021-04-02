#ifndef ASM
#define ASM

#include <stdint.h>

uint8_t curcpu(void);
uint64_t getel(void);
__attribute__ ((noreturn)) void spin_forever(void);

#endif
