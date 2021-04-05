.include "exception_asm.h"

.section .text

.align 12
.global ExceptionVectorsBase
ExceptionVectorsBase:
/* Current EL with SP0 */
/* Synchronous */
    msr SPSel, #1
    sub sp, sp, #0x310
    str x0, [sp]
    msr SPSel, #0
    mov x0, sp
    msr SPSel, #1
    b _handle_sync_exc_0
.balign 0x80
b .     /* IRQ/vIRQ */
.balign 0x80
b .     /* FIQ/vFIQ */
.balign 0x80
b .     /* SError/vSError */
.balign 0x80

/* Current EL with SPx */
/* Synchronous */
    sub sp, sp, #0x310
    str x0, [sp]
    add x0, sp, #0x310
    b _handle_sync_exc_0
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
    dsb sy
    isb sy
    mov x0, sp
    /* Get back on SP_EL0 now that we've saved everything, but
    not before we set the first parameter to a pointer to the rstate
    structure */
    msr SPSel, #0
    mrs x1, esr_el1
    bl handle_sync_exc_1
    /* We're done handling this exception, get back on the exception
    stack to restore register state */
    msr SPSel, #1
    RESTORE_REGISTERS sp
    /* Save a pointer to the current rstate structure... */
    mov x0, sp
    /* ...and then free the space we used for it */
    add sp, sp, #0x310
    /* w1 = saved spsr_el1 */
    ldr w1, [x0, #0x108]
    tbnz w1, #0, Lsync_exc_exit

Lneeds_sp_el0:
    msr SPSel, #0

Lsync_exc_exit:
    /* Finally, restore sp, x0, and x1 */
    ldr x1, [x0, #0xf8]
    mov sp, x1
    ldp x0, x1, [x0]
    dsb sy
    isb sy
    eret
