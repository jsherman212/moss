#ifndef FB
#define FB

#include <stdarg.h>
#include <stddef.h>

void fb_init(void);

void fb_putc(char);

void fb_gets_with_echo(char *, size_t);

void fb_puts(const char *s);

int fb_printf(const char *, ...);

#endif
