#include <stdint.h>
#include <stdlib.h>

#include <debug.h>
#include <kernel.h>
#include <panic.h>
#include <uart.h>
#include <vm/vm.h>

#include "asm.h"

/* struct bootargs *g_bootargs = NULL; */

static uint64_t g_shared = 0;

__attribute__ ((noreturn)) void _othercore_entryp(struct bootargs *args){
    uart_printf("cpu %d alive on EL%d\r\n", curcpu(), getel());
    dump_bootargs(args);

    for(;;){
        g_shared++;
    }

    __builtin_unreachable();
}

void kickstart_other_cores(void);

/* Kick the other cores out of the WFE sleep */
asm(".align 2\n"
    ".global kickstart_other_cores\n"
    "kickstart_other_cores:\n"
    "sev\n"
    "ret\n"
    );

/* cpu0 is always the only CPU that executes _main */
__attribute__ ((noreturn)) void _main(struct bootargs *args, void *arg1,
        void *arg2, void *arg3){
    /* g_bootargs = args; */

    uart_init();
    uart_puts("-------------");
    uart_printf("moss, on cpu %d, EL%d\n\r", curcpu(), getel());

    /* dump_kva_space(); */

    dump_bootargs(args);

    /* kickstart_other_cores(); */

    for(;;){
        char input[0x200];
        uart_gets_with_echo(input, sizeof(input));
        input[sizeof(input) - 1] = '\0';
        uart_printf("\n\r\nYou typed: %s\r\n", input);
    }

    __builtin_unreachable();
}
