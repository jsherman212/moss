#ifndef LIBC_STRING
#define LIBC_STRING

#include <stddef.h>
#include <stdint.h>

void bzero(void *, size_t);
void *memcpy(void *, const void *, size_t);
char *strcpy(char *, const char *);
size_t strlen(char *);

#endif
