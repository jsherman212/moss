#include <stdint.h>
#include <stdlib.h>

#include <uart.h>

#include "asm.h"

__attribute__ ((noreturn)) void _main(void *dtb32, void *x1, void *x2,
        void *x3){
    uart_init();
    uart_puts("-------------");
    uart_printf("moss, on EL%d\n\r", getel());

    /* uint64_t sctlr_el1, cpacr_el1; */
    /* asm volatile("mrs %0, sctlr_el1" : "=r" (sctlr_el1)); */
    /* asm volatile("mrs %0, cpacr_el1" : "=r" (cpacr_el1)); */

    /* uart_printf("sctlr_el1 %#llx cpacr_el1 %#llx\n\r", sctlr_el1, cpacr_el1); */

    /* uart_printf("%s: dtb32 = %p x1 = %p x2 = %p x3 = %p\n\r", __func__, */
    /*         dtb32, x1, x2, x3); */
    uart_printf("about to trigger a data abort...\r\n");

    asm volatile("mov x0, #-0x1");
    asm volatile("mov x1, #0x4242");
    asm volatile("mov w3, #0x1");
    asm volatile("str w3, [x0]");
    /* asm volatile("svc 0x80"); */
    uart_printf("Didn't crash???\r\n");

    for(;;){
        char input[0x200];
        uart_gets_with_echo(input, sizeof(input));
        input[sizeof(input) - 1] = '\0';
        uart_printf("\n\r\nYou typed: %s\r\n", input);
    }

    __builtin_unreachable();
}
