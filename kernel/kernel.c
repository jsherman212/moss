#include <stdint.h>
#include <stdlib.h>

__attribute__ ((noreturn)) void _main(void *dtb32, void *x1, void *x2,
        void *x3){
    asm volatile("b .");
    __builtin_unreachable();
}
