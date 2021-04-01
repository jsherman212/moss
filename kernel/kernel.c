#include <stdint.h>
#include <stdlib.h>

#include "uart.h"

__attribute__ ((noreturn)) void _main(void *dtb32, void *x1, void *x2,
        void *x3){
    uart_init();

    uart_puts("-------------");
    uart_puts("Hello, Justin");

    uart_printf("%s: dtb32 = %p x1 = %p x2 = %p x3 = %p\n\r", __func__,
            dtb32, x1, x2, x3);

    uart_printf("%% Hello world! %%\n\r");
    uart_printf("%% %c Hello%cworld! %%\n\r", 'A', '_');
    uart_printf("%% %c Hello%cworld! %s %%\n\r", 'A',
            '_', "Justin");
    uart_printf("%% %d %c Hello%cworld! %s %d %%\n\r", INT64_MAX+1,
            'A', '_', "Justin", 414243);
    uart_printf("%% %d %#x %c Hello%cworld! %s %d %%\n\r",
            INT64_MAX+1, 0x12345678, 'A', '_', "Justin", 414243);
    uart_printf("%% %p %lld %#llx %d %#x %c Hello%cworld! %s %d %%\n\r",
            _main, INT64_MAX, INT64_MAX, -1111, 0x11022, 'A', '_',
            "Justin", 414243);

    uart_printf("1. %10lld\r\n", 55uLL);
    uart_printf("2. %10lld\r\n", 555uLL);
    uart_printf("3. %-10lld\r\n", 55uLL);
    uart_printf("4. %-10.10lld\r\n", 55uLL);
    uart_printf("5. %10.10lld\r\n", 55uLL);
    uart_printf("6. %15.10lld\r\n", 55uLL);
    uart_printf("7. %11.10lld\r\n", 55uLL);
    uart_printf("8. %-.19lld\r\n", 445566uLL);
    uart_printf("9. %0.5lld\r\n", 11uLL);
    uart_printf("10. %-.5lld\r\n", 11uLL);
    uart_printf("11. %.5lld\r\n", 11uLL);
    uart_printf("12. %-15.10lld\r\n", 55uLL);
    uart_printf("13. %-15.10lld\r\n", 0uLL);

    for(;;){
        char input[0x200];
        uart_gets_with_echo(input, sizeof(input));
        input[sizeof(input) - 1] = '\0';
        uart_printf("\n\r\nYou typed: %s\r\n", input);
    }

    __builtin_unreachable();
}
