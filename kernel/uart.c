#include <stdarg.h>
#include <stdbool.h>
#include <stddef.h>

#include <doprnt.h>
#include <locks/spinlock.h>
#include <mmio.h>
#include <panic.h>

static bool uart_inited = false;

static splck_t uart_spinlock = SPLCK_INITIALIZER;

void uart_init(void){
    bool inited;

    splck_lck(&uart_spinlock);
    inited = uart_inited;
    splck_done(&uart_spinlock);

    if(inited)
        panic("uart_init called twice");

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

    splck_lck(&uart_spinlock);
    uart_inited = true;
    splck_done(&uart_spinlock);
}

static char uart_getc_internal(void){
    /* Wait until the receive FIFO holds a byte */
    while(!(rAUX_MU_LSR_REG & 0x1));
    return (char)(rAUX_MU_IO_REG & 0xff);
}

char uart_getc(void){
    char c;
    
    splck_lck(&uart_spinlock);
    c = uart_getc_internal();
    splck_done(&uart_spinlock);

    return c;
}

char uart_getc_unlocked(void){
    return uart_getc_internal();
}

static void uart_putc_internal(char c){
    /* Wait until transmitter is empty so we can accept a byte */
    while(!(rAUX_MU_LSR_REG & 0x20));
    rAUX_MU_IO_REG = c;
}

void uart_putc(char c){
    splck_lck(&uart_spinlock);
    uart_putc_internal(c);
    splck_done(&uart_spinlock);
}

void uart_putc_unlocked(char c){
    uart_putc_internal(c);
}

static void uart_gets_internal(char *buf, size_t buflen, bool echo){
    char *bufend = buf + buflen;

    while(buf < bufend){
        /* We could be locked */
        char got = uart_getc_unlocked();

        *buf++ = got;

        if(got != '\r' && got != '\n'){
            if(echo)
                uart_putc_internal(got);
        }
        else{
            *buf = '\0';
            return;
        }
    }
}

void uart_gets(char *buf, size_t buflen){
    splck_lck(&uart_spinlock);
    uart_gets_internal(buf, buflen, false);
    splck_done(&uart_spinlock);
}

void uart_gets_unlocked(char *buf, size_t buflen){
    uart_gets_internal(buf, buflen, false);
}

void uart_gets_with_echo(char *buf, size_t buflen){
    splck_lck(&uart_spinlock);
    uart_gets_internal(buf, buflen, true);
    splck_done(&uart_spinlock);
}

void uart_gets_with_echo_unlocked(char *buf, size_t buflen){
    uart_gets_internal(buf, buflen, true);
}

static void uart_puts_internal(const char *s){
    char *ss = (char *)s;

    while(*ss)
        uart_putc_unlocked(*ss++);

    uart_putc_unlocked('\r');
    uart_putc_unlocked('\n');
}

void uart_puts(const char *s){
    splck_lck(&uart_spinlock);
    uart_puts_internal(s);
    splck_done(&uart_spinlock);
}

void uart_puts_unlocked(const char *s){
    uart_puts_internal(s);
}

static void uart_printf_putc(char c, struct doprnt_info *info){
    info->written++;
    uart_putc_unlocked(c);
}

int uart_printf(const char *fmt, ...){
    va_list args;
    va_start(args, fmt);

    struct doprnt_info di;
    di.buf = NULL;
    di.remaining = 99999999;
    di.written = 0;

    int w;

    splck_lck(&uart_spinlock);
    w = doprnt(fmt, uart_printf_putc, &di, args);
    splck_done(&uart_spinlock);

    va_end(args);

    return w;
}

int uart_printf_unlocked(const char *fmt, ...){
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

    int w;

    splck_lck(&uart_spinlock);
    w = doprnt(fmt, uart_printf_putc, &di, args);
    splck_done(&uart_spinlock);

    return w;
}

int uart_vprintf_unlocked(const char *fmt, va_list args){
    struct doprnt_info di;
    di.buf = NULL;
    di.remaining = 99999999;
    di.written = 0;

    return doprnt(fmt, uart_printf_putc, &di, args);
}
