#ifndef UART
#define UART

#include <stdarg.h>
#include <stddef.h>

void uart_init(void);

char uart_getc(void);
void uart_putc(char);

void uart_gets(char *, size_t);
void uart_gets_with_echo(char *, size_t);
void uart_puts(const char *);

int uart_printf(const char *, ...);
int uart_vprintf(const char *, va_list);

#endif
