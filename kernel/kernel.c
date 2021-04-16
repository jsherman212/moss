#include <stdint.h>
#include <stdlib.h>

#include <debug.h>
#include <kernel.h>
#include <panic.h>
#include <uart.h>
#include <vm/vm.h>

#include "asm.h"

__attribute__ ((noreturn)) void _main(struct bootargs *args, void *arg1,
        void *arg2, void *arg3){
    uart_init();
    uart_puts("-------------");
    uart_printf("moss, on EL%d\n\r", getel());

    dump_bootargs(args);

    for(;;){
        char input[0x200];
        uart_gets_with_echo(input, sizeof(input));
        input[sizeof(input) - 1] = '\0';
        uart_printf("\n\r\nYou typed: %s\r\n", input);
    }

    __builtin_unreachable();
}
