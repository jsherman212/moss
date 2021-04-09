#include <stdint.h>
#include <stdlib.h>

#include <panic.h>
#include <uart.h>

#include "asm.h"

uint64_t transtest_read(uint64_t);
uint64_t transtest_write(uint64_t);

asm(".align 2\n"
    "transtest_read:\n"
    "at s1e1r, x0\n"
    "isb sy\n"
    "mrs x0, par_el1\n"
    "ret\n"
   );

asm(".align 2\n"
    "transtest_write:\n"
    "at s1e1w, x0\n"
    "isb sy\n"
    "mrs x0, par_el1\n"
    "ret\n"
   );

uint64_t read_tpidr_el1(void);

asm(".align 2\n"
    "read_tpidr_el1:\n"
    "mrs x0, tpidr_el1\n"
    "ret\n"
   );

uint64_t read_ttbr1_el1(void);

asm(".align 2\n"
    "read_ttbr1_el1:\n"
    "mrs x0, ttbr1_el1\n"
    "ret\n"
   );

static void transtests(void){
    uart_printf("Translation test while in EL2 yielded %p\n\r", read_tpidr_el1());

    uint64_t first_page = 0xffffff8000000000;

    /* extern uint64_t kernel_level1_table[] asm("kernel_level1_table"); */

    uint64_t kernel_level1_table = read_ttbr1_el1();
    
    uint64_t l1_idx = (first_page >> 30) & 0x1ff;
    uint64_t *l1_ttep = (uint64_t *)((uint64_t)kernel_level1_table + (l1_idx * 8));

    uart_printf("%s: l1 table at %p\n\r", __func__, kernel_level1_table);
    uart_printf("%s: l1 idx %lld ttep %p tte %#llx\n\r", __func__, l1_idx,
            l1_ttep, *l1_ttep);

    uint64_t ttr = 0x41414141, ttw = 0x42424242;

    ttr = transtest_read(first_page);
    ttw = transtest_write(first_page);

    uart_printf("%#llx: ttr %#llx ttw %#llx\n\r", first_page, ttr, ttw);
}

__attribute__ ((noreturn)) void _main(void *dtb32, void *arg1, void *arg2,
        void *arg3){
    uart_init();
    uart_puts("-------------");
    uart_printf("moss, on EL%d\n\r", getel());

    transtests();

    panic("test panic");
    
    uint64_t tcr_el1, id_aa64mmfr0_el1;
    asm volatile("mrs %0, tcr_el1" : "=r" (tcr_el1));
    asm volatile("mrs %0, id_aa64mmfr0_el1" : "=r" (id_aa64mmfr0_el1));
    uart_printf("tcr %#llx id_aa64mmfr0_el1 %#llx\r\n",
            tcr_el1, id_aa64mmfr0_el1);

    for(;;){
        char input[0x200];
        uart_gets_with_echo(input, sizeof(input));
        input[sizeof(input) - 1] = '\0';
        uart_printf("\n\r\nYou typed: %s\r\n", input);
    }

    __builtin_unreachable();
}
