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

    mov x29, xzr
    bl _main
    /* Not reached */

    /* Not cpu0? */
Lnot_cpu0:
    b .

    /* Not booted into EL2? */
Lnot_el2:
    b .
