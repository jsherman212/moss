.section .text
.align 2

.global getel
getel:
    mrs x0, CurrentEL
    lsr x0, x0, #0x2
    ret
