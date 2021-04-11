#include <stdarg.h>
#include <stddef.h>

#include <doprnt.h>
#include <mmio.h>

void uart_init(void){
    uint32_t gpfsel1 = rGPFSEL1;
    
    /* Select alternate function TXD1 for GPIO pin 14 */
    gpfsel1 &= ~(7 << 12);
    gpfsel1 |= 2 << 12;

    /* Select alternate function RXD1 for GPIO pin 15 */
    gpfsel1 &= ~(7 << 15);
    gpfsel1 |= 2 << 15;

    rGPFSEL1 = gpfsel1;

    /* Set pull-state for GPIO14 and GPIO15 to neither pull-up nor
     * pull-down */
    rGPIO_PUP_PDN_CNTRL_REG0 &= 0xfffffff;

    /* For AUX register accesses */
    rAUX_ENABLES = 1;
    rAUX_MU_CNTL_REG = 0;
    rAUX_MU_IER_REG = 0;
    rAUX_MU_LCR_REG = 3;
    rAUX_MU_MCR_REG = 0;
    rAUX_MU_BAUD_REG = 541;

    /* Enable RX/TX */
    rAUX_MU_CNTL_REG = 3;
}

char uart_getc(void){
    /* Wait until the receive FIFO holds a byte */
    while(!(rAUX_MU_LSR_REG & 0x1));
    return (char)(rAUX_MU_IO_REG & 0xff);
}

void uart_putc(char c){
    /* Wait until transmitter is empty so we can accept a byte */
    while(!(rAUX_MU_LSR_REG & 0x20));
    rAUX_MU_IO_REG = c;
}

void uart_gets(char *buf, size_t buflen){
    char *bufend = buf + buflen;

    while(buf < bufend){
        char got = uart_getc();
        *buf++ = got;

        if(got == '\r' || got == '\n'){
            *buf = '\0';
            return;
        }
    }
}

void uart_gets_with_echo(char *buf, size_t buflen){
    char *bufend = buf + buflen;

    while(buf < bufend){
        char got = uart_getc();
        *buf++ = got;

        if(got != '\r' && got != '\n')
            uart_putc(got);
        else{
            *buf = '\0';
            return;
        }
    }
}

void uart_puts(const char *s){
    char *ss = (char *)s;

    while(*ss)
        uart_putc(*ss++);

    uart_putc('\r');
    uart_putc('\n');
}

static void uart_printf_putc(char c, struct doprnt_info *info){
    info->written++;
    uart_putc(c);
}

int uart_printf(const char *fmt, ...){
    va_list args;
    va_start(args, fmt);

    struct doprnt_info di;
    di.buf = NULL;
    di.remaining = 99999999;
    di.written = 0;

    int w = doprnt(fmt, uart_printf_putc, &di, args);

    va_end(args);

    return w;
}

int uart_vprintf(const char *fmt, va_list args){
    struct doprnt_info di;
    di.buf = NULL;
    di.remaining = 99999999;
    di.written = 0;

    return doprnt(fmt, uart_printf_putc, &di, args);
}
