SECTIONS {
    /* Kernel is mapped from VA 0xffffff8000000000 upward once the MMU
        enable bit is set in SCTLR_EL1 */
    . = 0xffffff8000000000;

    kernel_begin = .;

    text_start = .;
    .text :  { *(.text) }
    . = ALIGN(0x1000);
    text_end = .;

    rodata_start = .;
    .rodata : { *(.rodata) }
    . = ALIGN(0x1000);
    rodata_end = .;

    data_start = .;
    .data : { *(.data) }
    . = ALIGN(0x1000);
    data_end = .;

    bss_start = .;
    .bss : { *(.bss*) }
    . = ALIGN(0x1000);
    bss_end = .;

    stacks_start = .;

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

    stacks_end = .;

    static_pagetables_start = .;

    /* Each entry here represents 1gb, in total one level 1 table
        represents 512gb, which is exactly how much kernel VA space
        we have */
    kernel_level1_table = .;
    . += 0x1000;

    /* Each entry here represents 2mb. Mapping the L2 tables that represent
        kernel VA base to 0xffffffffffffffff only takes a couple of MB,
        so to keep things simple, we'll keep doing that. */
    kernel_level2_table = .;
    . += ((0xffffffffffffffff - kernel_begin) / 0x40000000) * 0x1000;
    . = ALIGN(0x1000);

    /* Each entry here represents 4kb. However, if we map the pagetables
        that map kernel VA base to 0xffffffffffffffff, that takes up 1GB,
        which is such a huge waste of memory. Instead, I'm only mapping
        the pagetables that map static regions of the kernel
        (e.g. text,rodata,stacks,static ptes etc). If more L3 tables are
        needed after kernel entrypoint is jumped to, the kernel handles it.

        This intentionally only takes into account the contiguous region
        text - end of stacks represents */
    kernel_level3_table = .;
    /* +1 is to include the page when the divide evaluates to zero */
    . += (((stacks_end - kernel_begin) / 0x200000) + 1) * 0x1000;
    . = ALIGN(0x1000);

    static_pagetables_end = .;

    /DISCARD/ : {
        *(*.eh_frame*)
    }
}
