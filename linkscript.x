SECTIONS {
    /* Kernel is mapped from VA 0xffffff8000000000 upward once the MMU
        enable bit is set in SCTLR_EL1 */
    /* . = 0xffffffc000000000; */
    /* . = 0xffffff8000000000; */
/* Keep at 0 for testing */
    . = 0x0;

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

    /* Allocate space for stacks, page tables, etc */
    /*. = ALIGN(0x1000);*/

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

/* !!!!!!! PAGETABLES COLLIDE WITH VC SDRAM */
    pagetables_start = .;

    /* Each entry here represents 1gb, in total one level 1 table
        represents 512gb. But we don't have that much kernel VA space
        so one page for this is more than enough */
    kernel_level1_table = .;
    . += 0x1000;

    /* Each entry here represents 2mb */
    kernel_level2_table = .;
    /* . += ((0xffffffffffffffff - kernel_begin) / 0x200000) / 8; */
    /* . += ((0xffffffffffffffff - kernel_begin) / 0x40000000) * 0x1000; */
    /* Below for when we are not rebased to VA_KERNEL_BASE */
    . += ((0xffffffffffffffff - 0xffffff8000000000) / 0x40000000) * 0x1000;
    /*. += 0x1000;*/
    . = ALIGN(0x1000);

    /* Each entry here represents 4kb */
    kernel_level3_table = .;
    /* . += ((0xffffffffffffffff - kernel_begin) / 0x1000) / 8; */
    /* . += ((0xffffffffffffffff - kernel_begin) / 0x200000) * 0x1000; */
    /* Below for when we are not rebased to VA_KERNEL_BASE */
    . += ((0xffffffffffffffff - 0xffffff8000000000) / 0x200000) * 0x1000;
    /*. += 0x1000;*/
    . = ALIGN(0x1000);

/* !!!!!!! PAGETABLES COLLIDE WITH VC SDRAM */

    pagetables_end = .;

    kernel_end = .;
    kernel_heap_start = .;

    /DISCARD/ : {
        *(*.eh_frame*)
    }
}
