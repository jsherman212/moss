#ifndef CACHES
#define CACHES

#include <stddef.h>

void dcache_clean_PoU(void *, size_t);
void dcache_clean_and_invalidate_PoC(void *, size_t);
void icache_invalidate_PoU(void *, size_t);

#endif
