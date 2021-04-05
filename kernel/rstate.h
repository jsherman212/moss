#ifndef RSTATE
#define RSTATE

#include <stdint.h>

struct rstate {
    uint64_t x[30];
    uint64_t lr;
    uint64_t sp;
    uint64_t pc;
    __uint128_t q[32];
};

#endif
