SECTIONS {
    /* Kernel code always starts at 0 */
    . = 0x0;
    .text :  { *(.text) }
    .rodata : { *(.rodata) }
    .data : { *(.data) }

    /* Allocate space for stacks, page tables, etc. bss goes after
        everything so we don't have to worry about colliding with
        important data */
    . = ALIGN(0x1000);

    cpu0_stack = .;
    . += 0x1000;
    cpu0_exception_stack = .;
    . += 0x1000;
    
    /* Gonna be multicore eventually, may as well future-proof */
    cpu1_stack = .;
    . += 0x1000;
    cpu1_exception_stack = .;
    . += 0x1000;

    cpu2_stack = .;
    . += 0x1000;
    cpu2_exception_stack = .;
    . += 0x1000;

    cpu3_stack = .;
    . += 0x1000;
    cpu3_exception_stack = .;
    . += 0x1000;

    /* TODO pagetables here */

    .bss : { *(.bss*) }

    /DISCARD/ : {
        *(*.eh_frame*)
    }
}
