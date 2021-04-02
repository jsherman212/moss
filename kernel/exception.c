#include <panic.h>

#include "rstate.h"

void handle_sync_exc_1(struct rstate *state, uint32_t esr){
    uint64_t off;

    off = __builtin_offsetof(struct rstate, x[7]);
    off = __builtin_offsetof(struct rstate, x[15]);
    off = __builtin_offsetof(struct rstate, x[29]);
    off = __builtin_offsetof(struct rstate, lr);
    off = __builtin_offsetof(struct rstate, sp);
    off = __builtin_offsetof(struct rstate, q[0]);
    off = sizeof(struct rstate);

    panic_with_state(state, "Test panic");
}
