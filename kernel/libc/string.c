#include <stddef.h>
#include <stdint.h>

void bzero(void *s, size_t n){
    uint8_t *s0 = s;
    uint8_t *s_end = s0 + n;

    while(s0 < s_end)
        *s0++ = '\0';
}

void *memcpy(void *dst, const void *src, size_t n){
    if(n == 0)
        return dst;

    if((uintptr_t)dst & 7uLL == 0 && (uintptr_t)src & 7uLL == 0 &&
            n & 7uLL == 0){
        uint64_t *dstp = dst;
        const uint64_t *srcp = src;

        for(size_t i=0; i<n/sizeof(uint64_t); i++)
            dstp[i] = srcp[i];
    }
    else if((uintptr_t)dst & 3uLL == 0 && (uintptr_t)src & 3uLL == 0 &&
            n & 3uLL == 0){
        uint32_t *dstp = dst;
        const uint32_t *srcp = src;

        for(size_t i=0; i<n/sizeof(uint32_t); i++)
            dstp[i] = srcp[i];
    }
    else{
        uint8_t *dstp = dst;
        const uint8_t *srcp = src;

        for(size_t i=0; i<n; i++)
            dstp[i] = srcp[i];
    }

    /* Who cares about memcpy return value */
    return dst;
}

char *strcpy(char *dest, const char *src){
    char *src0 = (char *)src;
    while((*dest++ = *src0++));
    *dest = '\0';
    /* Who cares about strcpy return value */
    return dest;
}
