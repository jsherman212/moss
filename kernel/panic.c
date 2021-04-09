#include <stdarg.h>

#include <uart.h>
#include <rstate.h>

#include "asm.h"

static int g_panic_count = 0;

static void nested_panic_check(void){
    uart_printf("%s: panic count %d\r\n", __func__, g_panic_count);
    g_panic_count++;

    if(g_panic_count > 2){
        uart_printf("nested panic, stopping here...\n\r");
        spin_forever();
    }
}

struct frame {
    struct frame *fp;
    uint64_t lr;
};

static void backtrace(struct frame *f){
    int fnum = 0;

    while(f){
        uart_printf("\tFrame %d: fp 0x%-16.16llx lr 0x%-16.16llx\n\r", fnum,
                (uint64_t)f->fp, f->lr);
        f = f->fp;
        fnum++;
    }
    
    uart_printf("\n\r--- Could not unwind past frame %d\r\n", fnum - 1);
}

__attribute__ ((noreturn)) void panic(const char *fmt, ...){
    nested_panic_check();

    void *caller = __builtin_return_address(0);

    va_list args;
    va_start(args, fmt);
    uart_printf("\r\npanic(cpu %d caller %p): ", curcpu(), caller);
    uart_vprintf(fmt, args);
    uart_printf("\r\n");
    va_end(args);

    backtrace(__builtin_frame_address(1));

    spin_forever();
}

__attribute__ ((noreturn)) void panic_with_state(struct rstate *state,
        const char *fmt, ...){
    nested_panic_check();

    va_list args;
    va_start(args, fmt);
    uart_printf("\r\npanic(cpu %d): ", curcpu());
    uart_vprintf(fmt, args);
    va_end(args);

    uart_printf("\n\r\t");

    for(int i=0; i<sizeof(state->x)/sizeof(*state->x); i++){
        uart_printf("x%d: 0x%-16.16llx\t", i, state->x[i]);

        if(i > 0 && (i+1)%5 == 0)
            uart_printf("\r\n\t");
    }

    uart_printf("lr: 0x%-16.16llx\tsp: 0x%-16.16llx\tpc: 0x%-16.16llx  ",
            state->lr, state->sp, state->pc);

    uint64_t esr, far;
    asm volatile("mrs %0, esr_el1" : "=r" (esr));
    asm volatile("mrs %0, far_el1" : "=r" (far));

    uart_printf("ESR_EL1: 0x%-8.8x\tFAR_EL1: 0x%-16.16llx\n\n\r", esr, far);

    backtrace((struct frame *)state->x[29]);

    spin_forever();
}
