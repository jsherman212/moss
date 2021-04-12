#include <stdint.h>

#include <vm/vm_constants.h>

uint64_t linear_phystokv(uint64_t pa){
    return MIN_KERNEL_VA + (pa - KERNEL_BASE_PA);
}

asm(""
    ".align 2\n"
    ".global kvtophys\n"
    "kvtophys:\n"
    "mrs x1, DAIF\n"
    "msr DAIFSet, #0xf\n"
    "at s1e1r, x0\n"
    "mrs x2, par_el1\n"
    "msr DAIF, x1\n"
    "tbnz x2, #0x0, 2f\n"
    "and x2, x2, #0x7ffffffff000\n"
    "and x1, x0, #0xfff\n"
    "orr x0, x2, x1\n"
    "b 1f\n"
    "2:\n"
    "mov x0, xzr\n"
    "1:\n"
    "ret\n"
   );
