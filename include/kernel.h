#ifndef KERNEL
#define KERNEL

struct bootargs {
    /* Original first argument */
    void *dtb32;

    /* First static page of kernel, virtual */
    uint64_t kernel_base;

    /* TODO: KASLR */
    uint64_t kernel_slide;

    /* Starting page of level 3 page table entries for future
     * mappings, virtual */
    uint64_t start_of_L3;

    /* First page of free physical memory until we hit VC SDRAM. Then
     * free physical memory is the same as the address map for low
     * peripheral mode in chapter 1.2 Address Map in the BCM2711 ARM
     * peripherals manual */
    uint64_t freephys;
};

extern struct bootargs *g_bootargs;

#endif
