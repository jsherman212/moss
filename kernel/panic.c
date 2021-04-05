#include <stdarg.h>

#include <uart.h>

#include "asm.h"
#include "rstate.h"

static int g_panic_count = 0;

static void nested_panic_check(void){
    g_panic_count++;

    if(g_panic_count > 2){
        uart_printf("nested panic, stopping here...\n\r");
        spin_forever();
    }
}

__attribute__ ((noreturn)) void panic(const char *fmt, ...){
    nested_panic_check();

    va_list args;
    va_start(args, fmt);
    uart_printf("panic(cpu %d): ", curcpu());
    uart_vprintf(fmt, args);
    va_end(args);

    spin_forever();
}

__attribute__ ((noreturn)) void panic_with_state(struct rstate *state,
        const char *fmt, ...){
    nested_panic_check();

    va_list args;
    va_start(args, fmt);
    uart_printf("panic(cpu %d): ", curcpu());
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

    struct frame {
        struct frame *fp;
        uint64_t lr;
    };

    /* This could be corrupted as well, so... */
    struct frame *f = (struct frame *)state->x[29];
    int fnum = 0;

    while(f){
        uart_printf("Frame %d: fp 0x%-16.16llx lr 0x%-16.16llx\n\r", fnum,
                (uint64_t)f->fp, f->lr);
        f = f->fp;
        fnum++;
    }
    
    uart_printf("\n\r--- Could not unwind past frame %d\r\n", fnum - 1);

    spin_forever();
}
