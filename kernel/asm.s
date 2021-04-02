.section .text
.align 2

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

.global spin_forever
spin_forever:
    b .
