#include <stdint.h>
#include <stdlib.h>

#include <panic.h>
#include <uart.h>

#include "asm.h"

__attribute__ ((noreturn)) void _main(void *dtb32, void *arg1, void *arg2,
        void *arg3){
    uart_init();
    uart_puts("-------------");
    uart_printf("moss, on EL%d\n\r", getel());

    /* uint64_t sctlr_el1, cpacr_el1; */
    /* asm volatile("mrs %0, sctlr_el1" : "=r" (sctlr_el1)); */
    /* asm volatile("mrs %0, cpacr_el1" : "=r" (cpacr_el1)); */

    /* uart_printf("sctlr_el1 %#llx cpacr_el1 %#llx\n\r", sctlr_el1, cpacr_el1); */

    /* uart_printf("%s: dtb32 = %p x1 = %p x2 = %p x3 = %p\n\r", __func__, */
    /*         dtb32, x1, x2, x3); */
    uint64_t x0, x1, sp, spsel;
    asm volatile("mov %0, x0" : "=r" (x0));
    asm volatile("mov %0, x1" : "=r" (x1));
    asm volatile("mov %0, sp" : "=r" (sp));
    asm volatile("mrs %0, SPSel" : "=r" (spsel));
    uint64_t stk1 = 0x41414242;
    uint64_t stk2 = 0x43435555;
    uart_printf("about to execute svc... stk1 = %#llx stk2 = %#llx\r\n",
            stk1, stk2);
    uart_printf("before: sp %#llx spsel %lld\n", sp, spsel);

    asm volatile("mov x0, #-0x1");
    asm volatile("mov x1, #0x4242");
    asm volatile("mov w3, #0x1");
    asm volatile("mov x4, #0x4345");
    asm volatile("mov x5, #0x4345");
    asm volatile("mov x6, #0x4345");
    asm volatile("mov x7, #0x4345");
    asm volatile("mov x8, #0x4345");
    asm volatile("mov x9, #0x4345");
    asm volatile("mov x10, #0x4345");
    asm volatile("mov x11, #0x4345");
    asm volatile("mov x12, #0x4345");
    asm volatile("mov x13, #0x4345");
    asm volatile("mov x14, #0x4345");
    asm volatile("mov x15, #0x4345");
    asm volatile("mov x16, #0x4345");
    asm volatile("mov x17, #0x4345");
    asm volatile("mov x18, #0x4345");
    asm volatile("mov x19, #0x4345");
    asm volatile("mov x20, #0x4345");
    asm volatile("mov x21, #0x4345");
    asm volatile("mov x22, #0x4345");
    asm volatile("mov x23, #0x4345");
    asm volatile("mov x24, #0x4345");
    asm volatile("mov x25, #0x4345");
    asm volatile("mov x26, #0x4345");
    asm volatile("mov x27, #0x4345");
    asm volatile("svc 0x80");

    /* panic("test panic! stk1 = %llx stk2 = %llx\n", stk1, stk2); */
    /* uart_printf("Didn't crash???\r\n"); */
    uart_printf("\r\nback from svc... stk1 = %#llx stk2 = %#llx\r\n",
            stk1, stk2);

    asm volatile("mov %0, x0" : "=r" (x0));
    asm volatile("mov %0, x1" : "=r" (x1));
    asm volatile("mov %0, sp" : "=r" (sp));
    asm volatile("mrs %0, SPSel" : "=r" (spsel));
    /* uart_printf("after: x0 %#llx x1 %#llx sp %#llx\n", x0, x1, sp); */
    uart_printf("after: sp %#llx spsel %lld\n", sp, spsel);

    asm volatile("mov x0, #-1");
    asm volatile("str w3, [x0]");


    for(;;){
        char input[0x200];
        uart_gets_with_echo(input, sizeof(input));
        input[sizeof(input) - 1] = '\0';
        uart_printf("\n\r\nYou typed: %s\r\n", input);
    }

    __builtin_unreachable();
}
