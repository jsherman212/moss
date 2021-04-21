#ifndef DEBUG
#define DEBUG

#include <stddef.h>
#include <kernel.h>
#include <uart.h>

#ifdef MOSS_DEBUG
#define MOSSDBG(fmt, args...) do { uart_printf(fmt, ##args); } while (0)
#else
#define MOSSDBG(fmt, args...)
#endif

void dump_bootargs(struct bootargs *);
void dump_kva_space(void);
void hexdump(void *, size_t);

#endif
