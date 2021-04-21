#ifndef UART
#define UART

#include <stdarg.h>
#include <stddef.h>

void uart_init(void);

/* "unlocked" routines do not take a lock when doing whatever they do,
 * routines without "unlocked" do take a lock */

char uart_getc(void);
void uart_putc(char);

/* XXX: why would I ever use the locked versions of gets and
 * gets_with_echo? */
void uart_gets(char *, size_t);
void uart_gets_with_echo(char *, size_t);
void uart_puts(const char *);

int uart_printf(const char *, ...);
int uart_vprintf(const char *, va_list);

char uart_getc_unlocked(void);
void uart_putc_unlocked(char);

void uart_gets_unlocked(char *, size_t);
void uart_gets_with_echo_unlocked(char *, size_t);
void uart_puts_unlocked(const char *);

int uart_printf_unlocked(const char *, ...);
int uart_vprintf_unlocked(const char *, va_list);

#endif
