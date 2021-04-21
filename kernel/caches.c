#include <stddef.h>
#include <stdint.h>

static const size_t cache_line_size = 64;

void dcache_clean_PoU(void *address, size_t size){
    if(size & (cache_line_size - 1))
        size = (size + cache_line_size) & ~(cache_line_size - 1);

    uint64_t start = ((uint64_t)address & ~(cache_line_size - 1));
    uint64_t end = start + size;

    asm volatile("isb sy");

    do {
        asm volatile(""
                "dc cvau, %0\n"
                "dsb ish\n"
                "isb sy\n"
                : : "r" (start));

        start += cache_line_size;
    } while (start < end);
}

void dcache_clean_and_invalidate_PoC(void *address, size_t size){
    if(size & (cache_line_size - 1))
        size = (size + cache_line_size) & ~(cache_line_size - 1);

    uint64_t start = ((uint64_t)address & ~(cache_line_size - 1));
    uint64_t end = start + size;

    asm volatile("isb sy");

    do {
        asm volatile(""
                "dc civac, %0\n"
                "dsb ish\n"
                "isb sy\n"
                : : "r" (start));

        start += cache_line_size;
    } while (start < end);
}

void icache_invalidate_PoU(void *address, size_t size){
    if(size & (cache_line_size - 1))
        size = (size + cache_line_size) & ~(cache_line_size - 1);

    uint64_t start = ((uint64_t)address & ~(cache_line_size - 1));
    uint64_t end = start + size;

    asm volatile("isb sy");

    do {
        asm volatile(""
                "ic ivau, %0\n"
                "dsb ish\n"
                "isb sy\n"
                : : "r" (start));

        start += cache_line_size;
    } while (start < end);
}
