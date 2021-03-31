.section .text
.align 2

.global _start

    /* x0 - x4 are already used */
_start:
    mrs x5, mpidr_el1
    and x5, x5, #0xff
    cbz x5, Ljump_to_kernel

    /* Not cpu0? */
    b .

    /* XXX what el are we booted into... */
Ljump_to_kernel:
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
    b .
