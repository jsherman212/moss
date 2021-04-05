/* ss: pointer to free space */
.macro SAVE_REGISTERS ss
    stp x0, x1, [\ss, #0x0]
    stp x2, x3, [\ss, #0x10]
    stp x4, x5, [\ss, #0x20]
    stp x6, x7, [\ss, #0x30]
    stp x8, x9, [\ss, #0x40]
    stp x10, x11, [\ss, #0x50]
    stp x12, x13, [\ss, #0x60]
    stp x14, x15, [\ss, #0x70]
    stp x16, x17, [\ss, #0x80]
    stp x18, x19, [\ss, #0x90]
    stp x20, x21, [\ss, #0xa0]
    stp x22, x23, [\ss, #0xb0]
    stp x24, x25, [\ss, #0xc0]
    stp x26, x27, [\ss, #0xd0]
    stp x28, x29, [\ss, #0xe0]
    /* sizeof(struct rstate) */
    add x0, sp, #0x310
    stp x30, x0, [\ss, #0xf0]
    mrs x0, elr_el1
    str x0, [\ss, #0x100]
    stp q0, q1, [\ss, #0x110]
    stp q2, q3, [\ss, #0x130]
    stp q4, q5, [\ss, #0x150]
    stp q6, q7, [\ss, #0x170]
    stp q8, q9, [\ss, #0x190]
    stp q10, q11, [\ss, #0x1b0]
    stp q12, q13, [\ss, #0x1d0]
    stp q14, q15, [\ss, #0x1f0]
    stp q16, q17, [\ss, #0x210]
    stp q18, q19, [\ss, #0x230]
    stp q20, q21, [\ss, #0x250]
    stp q22, q23, [\ss, #0x270]
    stp q24, q25, [\ss, #0x290]
    stp q26, q27, [\ss, #0x2b0]
    stp q28, q29, [\ss, #0x2d0]
    stp q30, q31, [\ss, #0x2f0]
.endm

    /* ss: pointer to rstate structure, does not restore x0 so
    we can use that to restore sp right before eret */
.macro RESTORE_REGISTERS ss
    ldr x1, [\ss, #0x8]
    ldp x2, x3, [\ss, #0x10]
    ldp x4, x5, [\ss, #0x20]
    ldp x6, x7, [\ss, #0x30]
    ldp x8, x9, [\ss, #0x40]
    ldp x10, x11, [\ss, #0x50]
    ldp x12, x13, [\ss, #0x60]
    ldp x14, x15, [\ss, #0x70]
    ldp x16, x17, [\ss, #0x80]
    ldp x18, x19, [\ss, #0x90]
    ldp x20, x21, [\ss, #0xa0]
    ldp x22, x23, [\ss, #0xb0]
    ldp x24, x25, [\ss, #0xc0]
    ldp x26, x27, [\ss, #0xd0]
    ldp x28, x29, [\ss, #0xe0]
    ldr x30, [\ss, #0xf0]
    ldp q0, q1, [\ss, #0x110]
    ldp q2, q3, [\ss, #0x130]
    ldp q4, q5, [\ss, #0x150]
    ldp q6, q7, [\ss, #0x170]
    ldp q8, q9, [\ss, #0x190]
    ldp q10, q11, [\ss, #0x1b0]
    ldp q12, q13, [\ss, #0x1d0]
    ldp q14, q15, [\ss, #0x1f0]
    ldp q16, q17, [\ss, #0x210]
    ldp q18, q19, [\ss, #0x230]
    ldp q20, q21, [\ss, #0x250]
    ldp q22, q23, [\ss, #0x270]
    ldp q24, q25, [\ss, #0x290]
    ldp q26, q27, [\ss, #0x2b0]
    ldp q28, q29, [\ss, #0x2d0]
    ldp q30, q31, [\ss, #0x2f0]
.endm

.section .text

.align 12
.global ExceptionVectorsBase
ExceptionVectorsBase:
/* Current EL with SP0 */
/* Synchronous */
    msr SPSel, #0
    sub sp, sp, #0x310
    b _handle_sync_exc_0
.balign 0x80
b .     /* IRQ/vIRQ */
.balign 0x80
b .     /* FIQ/vFIQ */
.balign 0x80
b .     /* SError/vSError */
.balign 0x80

/* Current EL with SPx */
b .     /* Synchronous */
.balign 0x80
b .     /* IRQ/vIRQ */
.balign 0x80
b .     /* FIQ/vFIQ */
.balign 0x80
b .     /* SError/vSError */
.balign 0x80

/* Lower EL using AArch64 */
b ./* Synchronous */
.balign 0x80
b .     /* IRQ/vIRQ */
.balign 0x80
b .     /* FIQ/vFIQ */
.balign 0x80
b .     /* SError/vSError */
.balign 0x80

/* Lower EL using AArch32 (unused) */
b .     /* Synchronous */
.balign 0x80
b .     /* IRQ/vIRQ */
.balign 0x80
b .     /* FIQ/vFIQ */
.balign 0x80
b .     /* SError/vSError */
.balign 0x80

_handle_sync_exc_0:
    SAVE_REGISTERS sp
    mov x0, sp
    mrs x1, esr_el1
    bl handle_sync_exc_1
    /* TODO when userspace is a thing, make sure to differentiate
    between EL1 and EL0 eret */
    mov x0, #0x3c4
    /* TODO keep track of far and spsr in state structure instead of
    just masking all interrupts */
    msr spsr_el1, x0
    RESTORE_REGISTERS sp
    ldr x0, [sp, #0xf8]
    /* TODO make sure I select the right stack pointer!! */
    mov sp, x0
    eret
