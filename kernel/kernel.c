#include <stdint.h>
#include <stdlib.h>

#include <asm.h>
#include <debug.h>
#include <fb/fb.h>
#include <kernel.h>
#include <libc/string.h>
#include <locks/spinlock.h>
#include <panic.h>
#include <uart.h>
#include <vm/vm.h>

struct bootargs *g_bootargsp = &g_bootargs;

static splck_t spinlock = SPLCK_INITIALIZER;
/* static splck_t *spinlockp = &spinlock; */

static uint64_t g_shared = 0;

static uint64_t tickarray[] = { 1000000000, 2000000000, 3000000000 };

__attribute__ ((noreturn)) void _othercore_entryp(void){
    /* delay_ticks(tickarray[curcpu()-1]); */

    /* splck_lck(&spinlock); */
    /* uart_printf_unlocked("cpu %d alive on EL%d\r\n", curcpu(), getel()); */
    uart_printf("cpu %d alive on EL%d\r\n", curcpu(), getel());
    dump_bootargs(g_bootargsp);

    /* uart_printf_unlocked("cpu%d sp_el0 %#llx sp_elx %#llx\r\n", curcpu(), */
    /*         get_sp_el0(), get_sp_elx()); */
    uart_printf("cpu%d sp_el0 %#llx sp_elx %#llx\r\n", curcpu(),
            get_sp_el0(), get_sp_elx());

    /* splck_done(&spinlock); */

    for(;;){
        g_shared++;
    }

    __builtin_unreachable();
}

/* cpu0 is always the only CPU that executes _main */
__attribute__ ((noreturn)) void _main(struct bootargs *args, void *arg1,
        void *arg2, void *arg3){
    uart_init();
    uart_puts("-------------");
    uart_printf("moss, on cpu %d, EL%d\n\r", curcpu(), getel());

    dump_bootargs(g_bootargsp);

    fb_init();

    /* Kernel bootstrapping complete, kick the other CPUs out of their
     * low power state */
    /* send_event(); */

    /* Drop into a shell. XXX: this will first be a 'task' once I
     * get a scheduler set up, then a userspace program that uses
     * system calls one I get those two things set up */
    for(;;){
        fb_printf("moss> ");

        char input[0x200];
        fb_gets_with_echo(input, sizeof(input));
        hexdump(input, strlen(input));
        size_t inputlen = strlen(input);
        /* Delete newline */
        input[inputlen - 1] = '\0';
        fb_printf("\nYou entered '%s'\n", input);
    }

    /* for(;;){ */
    /*     char input[0x200]; */
    /*     uart_gets_with_echo_unlocked(input, sizeof(input)); */
    /*     input[sizeof(input) - 1] = '\0'; */
    /*     uart_printf("\n\r\nYou typed: %s\r\n", input); */
    /* } */

    __builtin_unreachable();
}
