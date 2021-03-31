#ifndef UART
#define UART

void uart_init(void);

char uart_getc(void);
void uart_putc(char);
void uart_gets(char *, size_t);
void uart_gets_with_echo(char *, size_t);
void uart_puts(const char *);

#endif
