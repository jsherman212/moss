#include <stdint.h>
#include <stdlib.h>

#include "uart.h"

__attribute__ ((noreturn)) void _main(void *dtb32, void *x1, void *x2,
        void *x3){
    uart_init();

    uart_puts("-------------");
    uart_puts("Hello, Justin");

    for(;;){
        char input[0x200];
        uart_gets_with_echo(input, sizeof(input));
        uart_puts("\r\nYou typed: ");
        uart_putc('\t');
        uart_puts(input);
        uart_puts("");
    }

    __builtin_unreachable();
}
