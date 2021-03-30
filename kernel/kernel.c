#include <stdint.h>
#include <stdlib.h>

#include "uart.h"

__attribute__ ((noreturn)) void _main(void *dtb32, void *x1, void *x2,
        void *x3){
    uart_init();
    for(;;);
    __builtin_unreachable();
}
