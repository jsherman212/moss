#ifndef PANIC
#define PANIC

/* #include "rstate.h" */

__attribute__ ((noreturn)) void panic(const char *, ...);
/* __attribute__ ((noreturn)) void panic_with_state(struct rstate *, */
/*         const char *, ...); */
__attribute__ ((noreturn)) void panic_with_state(void *,
        const char *, ...);

#endif
