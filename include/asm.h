#ifndef ASM
#define ASM

#include <stdint.h>

uint64_t at_s1e1r(uint64_t);
uint64_t at_s1e1w(uint64_t);
uint64_t at_s1e0r(uint64_t);
uint64_t at_s1e0w(uint64_t);
uint8_t curcpu(void);
uint64_t current_thread(void);
void delay_ticks(uint64_t);
uint64_t getel(void);
uint64_t get_sp_el0(void);
uint64_t get_sp_elx(void);
uint64_t read_ttbr1(void);
void send_event(void);
__attribute__ ((noreturn)) void spin_forever(void);
void tlb_flush(void);

#endif
