SECTIONS {
    /* Kernel is mapped from VA 0xffffffc000000000 upward once the MMU
        enable bit is set in SCTLR_EL1 */
    /* . = 0xffffffc000000000; */
    /* . = 0xffffff8000000000; */
/* Keep at 0 for testing */
    . = 0x0;

    kernel_begin = .;

    .text :  { *(.text) }
    .rodata : { *(.rodata) }
    .data : { *(.data) }

    bss_start = .;
    .bss : { *(.bss*) }
    bss_end = .;

    /* Allocate space for stacks, page tables, etc */
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

    /* Each entry here represents 1gb, in total one level 1 table
        represents 512gb. But we don't have that much kernel VA space
        so one page for this is more than enough */
    kernel_level1_table = .;
    . += 0x1000;

    /* Each entry here represents 2mb */
    kernel_level2_table = .;
    /* . += ((0xffffffffffffffff - kernel_begin) / 0x200000) / 8; */
    . += ((0xffffffffffffffff - kernel_begin) / 0x40000000) * 0x1000;
    . += 0x1000;
    . = ALIGN(0x1000);

    /* Each entry here represents 4kb */
    kernel_level3_table = .;
    /* . += ((0xffffffffffffffff - kernel_begin) / 0x1000) / 8; */
    . += ((0xffffffffffffffff - kernel_begin) / 0x200000) * 0x1000;
    . += 0x1000;
    . = ALIGN(0x1000);

    translation_tables_end = .;

    /* This does not collide with VC SDRAM */
    kernel_end = .;

    /DISCARD/ : {
        *(*.eh_frame*)
    }
}
