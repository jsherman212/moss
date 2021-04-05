#include <panic.h>
#include <uart.h>

#include "exception.h"

static const char *abort_fsc_str(uint32_t fsc){
    switch(fsc){
        case FSC_ADDRESS_SIZE_FAULT_L0:
            return "Address size fault, level 0";
        case FSC_ADDRESS_SIZE_FAULT_L1:
            return "Address size fault, level 1";
        case FSC_ADDRESS_SIZE_FAULT_L2:
            return "Address size fault, level 2";
        case FSC_ADDRESS_SIZE_FAULT_L3:
            return "Address size fault, level 3";
        case FSC_TRANSLATION_FAULT_L0:
            return "Translation fault, level 0";
        case FSC_TRANSLATION_FAULT_L1:
            return "Translation fault, level 1";
        case FSC_TRANSLATION_FAULT_L2:
            return "Translation fault, level 2";
        case FSC_TRANSLATION_FAULT_L3:
            return "Translation fault, level 3";
        case FSC_ACCESS_FLAG_FAULT_L1:
            return "Access flag fault, level 1";
        case FSC_ACCESS_FLAG_FAULT_L2:
            return "Access flag fault, level 2";
        case FSC_ACCESS_FLAG_FAULT_L3:
            return "Access flag fault, level 3";
        case FSC_PERMISSION_FAULT_L1:
            return "Permission fault, level 1";
        case FSC_PERMISSION_FAULT_L2:
            return "Permission fault, level 2";
        case FSC_PERMISSION_FAULT_L3:
            return "Permission fault, level 3";
        case FSC_SYNC_EXTERNAL_ABORT:
            return "Synchronous external abort, not on TT walk/update";
        case FSC_SYNC_EXTERNAL_ABORT_L0:
            return "Synchronous external abourt, on TT walk/update, level 0";
        case FSC_SYNC_EXTERNAL_ABORT_L1:
            return "Synchronous external abourt, on TT walk/update, level 1";
        case FSC_SYNC_EXTERNAL_ABORT_L2:
            return "Synchronous external abourt, on TT walk/update, level 2";
        case FSC_SYNC_EXTERNAL_ABORT_L3:
            return "Synchronous external abourt, on TT walk/update, level 3";
        case FSC_SYNC_PARITY_OR_ECC:
            return "Synchronous party or ECC error, not on TT walk";
        case FSC_SYNC_PARITY_OR_ECC_L0:
            return "Synchronous party or ECC error, level 0";
        case FSC_SYNC_PARITY_OR_ECC_L1:
            return "Synchronous party or ECC error, level 1";
        case FSC_SYNC_PARITY_OR_ECC_L2:
            return "Synchronous party or ECC error, level 2";
        case FSC_SYNC_PARITY_OR_ECC_L3:
            return "Synchronous party or ECC error, level 3";
        case FSC_ALIGNMENT_FAULT:
            return "Alignment fault";
        case FSC_TLB_CONFLICT_ABORT:
            return "TLB conflict abort";
        default:
            return "<unknown fsc>";
    };
}

static void handle_trapped_wf(struct rstate *state, uint32_t iss){
    /* XXX should I panic? */
    const char *instr = "wfi";

    if(iss & 0x1)
        instr = "wfe";

    panic_with_state(state, "trapped %s", instr);
}

static void handle_trapped_sve_simd_fp(struct rstate *state,
        uint32_t iss){
    panic_with_state(state, "trapped sve, simd, or fp instr");
}

static void handle_illegal_execution_state(struct rstate *state,
        uint32_t iss){
    /* XXX userspace(?): kill process */
    panic_with_state(state, "illegal execution state");
}

static void handle_svc(struct rstate *state, uint32_t iss){
    /* Check if the svc came from EL1 once I get userspace set up */
    /* panic_with_state(state, "svc"); */

    uart_printf("\r\n%s: trapped SVC instr\n", __func__);
}

static void handle_trapped_msr_mrs_sys(struct rstate *state,
        uint32_t iss){
    /* XXX userspace: kill process */
    panic_with_state(state, "trapped msr, mrs, or sys instr");
}

/* XXX gonna have to handle page faults here when I get userspace set up */
static void handle_abort(struct rstate *state, uint32_t ec, uint32_t iss){
    const char *what = "instruction fetch";

    if(ec == ESR_EC_DATA_ABORT_LOWER_EL ||
            ec == ESR_EC_DATA_ABORT_SAME_EL){
        what = "data";
    }

    const char *where = ec & 0x1 ? "kernel" : "userspace";
    const char *why = abort_fsc_str(iss & FSC_MASK);

    /* XXX won't actually panic for userspace stuff, but for
     * now that's fine */
    panic_with_state(state, "%s %s abort: %s", where, what, why);
}

static void handle_unaligned_pc(struct rstate *state, uint32_t iss){
    /* XXX userspace: kill process */
    panic_with_state(state, "unaligned pc");
}

static void handle_unaligned_sp(struct rstate *state, uint32_t iss){
    /* XXX userspace: kill process */
    panic_with_state(state, "unaligned sp");
}

static void handle_trapped_fp_instr(struct rstate *state,
        uint32_t iss){
    /* placeholder... will I need this functionality? */
    panic_with_state(state, "trapped fp instr");
}

static void handle_serror(struct rstate *state, uint32_t iss){
    panic_with_state(state, "SError interrupt");
}

static void handle_debug(struct rstate *state, uint32_t ec,
        uint32_t iss){
    const char *what = "breakpoint";
    const char *where = ec & 0x1 ? "kernel" : "userspace";

    if(ec == ESR_EC_SS_LOWER_EL || ec == ESR_EC_SS_SAME_EL)
        what = "software single step";
    else if(ec == ESR_EC_WP_LOWER_EL || ec == ESR_EC_WP_SAME_EL)
        what = "watchpoint";
    else if(ec == ESR_EC_BRK)
        what = "BRK instruction";

    /* XXX won't actually panic for userspace stuff, but for
     * now that's fine */
    panic_with_state(state, "%s %s exception", where, what);
}

void handle_sync_exc_1(struct rstate *state, uint32_t esr){
    uint32_t ec = (esr >> 26) & 0x3f;
    uint32_t iss = esr & 0xffffff;

    switch(ec){
        case ESR_EC_TRAPPED_WF:
            handle_trapped_wf(state, iss);
            break;
        case ESR_EC_TRAPPED_SVE_SIMD_FP:
            handle_trapped_sve_simd_fp(state, iss);
            break;
        case ESR_EC_ILLEGAL_EXECUTION_STATE:
            handle_illegal_execution_state(state, iss);
            break;
        case ESR_EC_SVC:
            handle_svc(state, iss);
            break;
        case ESR_EC_TRAPPED_MSR_MRS_SYS:
            handle_trapped_msr_mrs_sys(state, iss);
            break;
        case ESR_EC_INSTR_FETCH_ABORT_LOWER_EL:
        case ESR_EC_INSTR_FETCH_ABORT_SAME_EL:
            handle_abort(state, ec, iss);
            break;
        case ESR_EC_UNALIGNED_PC:
            handle_unaligned_pc(state, iss);
            break;
        case ESR_EC_DATA_ABORT_LOWER_EL:
        case ESR_EC_DATA_ABORT_SAME_EL:
            handle_abort(state, ec, iss);
            break;
        case ESR_EC_UNALIGNED_SP:
            handle_unaligned_sp(state, iss);
            break;
        case ESR_EC_TRAPPED_FP:
            handle_trapped_fp_instr(state, iss);
            break;
        case ESR_EC_SERROR:
            handle_serror(state, iss);
            break;
        case ESR_EC_BP_LOWER_EL:
        case ESR_EC_BP_SAME_EL:
        case ESR_EC_SS_LOWER_EL:
        case ESR_EC_SS_SAME_EL:
        case ESR_EC_WP_LOWER_EL:
        case ESR_EC_WP_SAME_EL:
        case ESR_EC_BRK:
            handle_debug(state, ec, iss);
            break;
        case ESR_EC_UNKNOWN:
        default:
            panic_with_state(state, "unknown exception syndrome %#x", ec);
    };


    uint64_t off;

    /* off = __builtin_offsetof(struct rstate, x[7]); */
    /* off = __builtin_offsetof(struct rstate, x[15]); */
    /* off = __builtin_offsetof(struct rstate, x[29]); */
    off = __builtin_offsetof(struct rstate, lr);
    off = __builtin_offsetof(struct rstate, sp);
    off = __builtin_offsetof(struct rstate, pc);
    off = __builtin_offsetof(struct rstate, cpsr);
    off = __builtin_offsetof(struct rstate, q[0]);
    off = sizeof(struct rstate);

    /* panic_with_state(state, "Test panic"); */
}
