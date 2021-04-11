#include <stdint.h>
#include <stdlib.h>

#include <debug.h>
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

uint64_t read_tpidr_el0(void);

asm(".align 2\n"
    "read_tpidr_el0:\n"
    "mrs x0, tpidr_el0\n"
    "ret\n"
   );

/* uint64_t read_ttbr1_el1(void); */

/* asm(".align 2\n" */
/*     "read_ttbr1_el1:\n" */
/*     "mrs x0, ttbr1_el1\n" */
/*     "ret\n" */
/*    ); */

static void transtests(void){
    uart_printf("Hello\n\r");
    uint64_t addr = read_tpidr_el0();
    uart_printf("Translation test while in EL2 for %#llx "
            "yielded %p\n\r", addr, read_tpidr_el1());

    /* addr = 0xffffff8000084000; */

    /* extern uint64_t kernel_level1_table[] asm("kernel_level1_table"); */

    dump_kva_space();

    /* uint64_t kernel_level1_table = read_ttbr1_el1(); */
    
    /* uint64_t l1_idx = (addr >> 30) & 0x1ff; */
    /* uint64_t *l1_ttep = (uint64_t *)((uint64_t)kernel_level1_table + (l1_idx * 8)); */

    /* uart_printf("%s: l1 table at %p\n\r", __func__, kernel_level1_table); */
    /* uart_printf("%s: l1 idx %lld ttep %p tte %#llx\n\r", __func__, l1_idx, */
    /*         l1_ttep, *l1_ttep); */

    /* hexdump((void*)kernel_level1_table, 0x20); */

    /* uint64_t l2_idx = (addr >> 21) & 0x1ff; */
    /* uint64_t l2_tbl = *l1_ttep & 0xfffffffff000; */
    /* uint64_t *l2_ttep = (uint64_t *)(l2_tbl + (l2_idx * 8)); */

    /* uart_printf("%s: l2 idx %lld ttep %p tte %#llx\n\r", __func__, l2_idx, */
    /*         l2_ttep, *l2_ttep); */
    /* hexdump((void*)l2_tbl, 0x1000); */

    /* uint64_t l3_idx = (addr >> 12) & 0x1ff; */
    /* uint64_t l3_tbl = *l2_ttep & 0xfffffffff000; */
    /* uint64_t *l3_ptep = (uint64_t *)(l3_tbl + (l3_idx * 8)); */

    /* uart_printf("%s: l3 idx %lld ptep %p pte %#llx\n\r", __func__, l3_idx, */
    /*         l3_ptep, *l3_ptep); */

    /* hexdump((void*)l3_tbl, 0x1000); */

    /* uint64_t ttr = 0x41414141, ttw = 0x42424242; */

    /* ttr = transtest_read(addr); */
    /* ttw = transtest_write(addr); */

    /* uart_printf("%#llx: ttr %#llx ttw %#llx\n\r", addr, ttr, ttw); */
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
