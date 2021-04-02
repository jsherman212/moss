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
    add x0, sp, #0x300
    stp x30, x0, [\ss, #0xf0]
    stp q0, q1, [\ss, #0x100]
    stp q2, q3, [\ss, #0x120]
    stp q4, q5, [\ss, #0x140]
    stp q6, q7, [\ss, #0x160]
    stp q8, q9, [\ss, #0x180]
    stp q10, q11, [\ss, #0x1a0]
    stp q12, q13, [\ss, #0x1c0]
    stp q14, q15, [\ss, #0x1e0]
    stp q16, q17, [\ss, #0x200]
    stp q18, q19, [\ss, #0x220]
    stp q20, q21, [\ss, #0x240]
    stp q22, q23, [\ss, #0x260]
    stp q24, q25, [\ss, #0x280]
    stp q26, q27, [\ss, #0x2a0]
    stp q28, q29, [\ss, #0x2c0]
    stp q30, q31, [\ss, #0x2e0]
.endm

.section .text

.align 12
.global ExceptionVectorsBase
ExceptionVectorsBase:
/* Current EL with SP0 */
/* Synchronous */
    msr SPSel, #0
    sub sp, sp, #0x300
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
    /* eret if svc */
    ret
