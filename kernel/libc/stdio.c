#include <stdarg.h>

#include <doprnt.h>

static void snprintf_putc(char c, struct doprnt_info *info){
    if(info->remaining > 1){
        *info->buf++ = c;
        info->remaining--;
    }
}

int snprintf(char *buf, size_t sz, const char *fmt, ...){
    va_list args;
    va_start(args, fmt);

    struct doprnt_info di;
    di.buf = buf;
    di.remaining = sz;
    di.written = 0;

    int w = doprnt(fmt, snprintf_putc, &di, args);

    va_end(args);

    if(di.remaining > 0)
        *di.buf = '\0';

    return w;
}
