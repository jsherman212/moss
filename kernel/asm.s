.section .text
.align 2

    /* TODO: disable interrupts for the at routines */
.global at_s1e1r
at_s1e1r:
    at s1e1r, x0
    isb sy
    mrs x0, par_el1
    ret

.global at_s1e1w
at_s1e1w:
    at s1e1w, x0
    isb sy
    mrs x0, par_el1
    ret

.global at_s1e0r
at_s1e0r:
    at s1e0r, x0
    isb sy
    mrs x0, par_el1
    ret

.global at_s1e0w
at_s1e0w:
    at s1e0w, x0
    isb sy
    mrs x0, par_el1
    ret

.global curcpu
curcpu:
    mrs x0, mpidr_el1
    and x0, x0, #0xff
    ret

.global getel
getel:
    mrs x0, CurrentEL
    lsr x0, x0, #0x2
    ret

.global read_ttbr1
read_ttbr1:
    mrs x0, ttbr1_el1
    ret

.global spin_forever
spin_forever:
    b .

.global tlb_flush
tlb_flush:
    isb sy
    dsb sy
    tlbi vmalle1
    dsb sy
    isb sy
    ret
