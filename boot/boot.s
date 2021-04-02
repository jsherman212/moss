.section .text
.align 2

.global _start

    /* x0 - x4 are already used, we are booted into EL2 */
_start:
    mrs x5, mpidr_el1
    and x5, x5, #0xff
    cbnz x5, Lnot_cpu0

    adr x5, Lel1_entry

    /* SPSR: mask all interrupts when we return to EL1 */
    mov x6, #0x3c4

    mrs x7, CurrentEL
    lsr x7, x7, #0x2
    cmp x7, #0x2
    b.eq Lel2_entry
    b Lnot_el2

Lel2_entry:
    /* Set exception vector before attempting any loads/stores */
    adr x7, ExceptionVectorsBase
    msr vbar_el1, x7

    msr elr_el2, x5
    msr spsr_el2, x6

    /* EL1 is AArch64 */
    mov x5, #(1 << 31)
    msr hcr_el2, x5

    /* Enable floating point */
    mrs x5, cpacr_el1
    orr x5, x5, #0x300000
    msr cpacr_el1, x5
    
    /* Enable caches, instruction cache, SP alignment checking,
    and WXN, but not the MMU yet */
    mov x5, #0x100c
    orr x5, x5, #(1 << 19)
    msr sctlr_el1, x5

    isb sy
    eret

Lel1_entry:
    adr x5, cpu0_exception_stack
    add x5, x5, #0x1000
    msr SPSel, #1
    mov sp, x5

    adr x5, cpu0_stack
    add x5, x5, #0x1000
    msr SPSel, #0
    mov sp, x5

    isb sy

    mov x5, xzr
    mov x6, xzr
    mov x7, xzr
    mov x8, xzr
    mov x9, xzr
    mov x10, xzr
    mov x11, xzr
    mov x12, xzr
    mov x13, xzr
    mov x14, xzr
    mov x15, xzr
    mov x16, xzr
    mov x17, xzr
    mov x18, xzr
    mov x19, xzr
    mov x20, xzr
    mov x21, xzr
    mov x22, xzr
    mov x23, xzr
    mov x24, xzr
    mov x25, xzr
    mov x26, xzr
    mov x27, xzr
    mov x28, xzr
    mov x29, xzr
    mov x30, xzr

    bl _main
    /* Not reached */

    /* Not cpu0? */
Lnot_cpu0:
    b .

    /* Not booted into EL2? */
Lnot_el2:
    b .
