.set VA_KERNEL_BASE,  0xffffff8000000000
.set PA_KERNEL_BASE,  0x80000
.set L1_SHIFT,        30
.set L2_SHIFT,        21
.set L3_SHIFT,        12
.set INDEX_MASK,      0x1ff
.set TABLE_MASK,      0xfffffffff000

.set VM_PROT_READ_BIT,    0
.set VM_PROT_WRITE_BIT,   1
.set VM_PROT_EXECUTE_BIT, 2

.set VM_PROT_READ,    1 << VM_PROT_READ_BIT
.set VM_PROT_WRITE,   1 << VM_PROT_WRITE_BIT
.set VM_PROT_EXECUTE, 1 << VM_PROT_EXECUTE_BIT

.set L3_ENTRIES_LIMIT,  511

.set PAGE_SIZE,     0x1000

.set DEVICE_MEMORY_SIZE,        0x4000000

.set PA_DEVICE_MEMORY_BASE,     0xfc000000
.set PA_DEVICE_MEMORY_END,      PA_DEVICE_MEMORY_BASE + DEVICE_MEMORY_SIZE
.set VA_DEVICE_MEMORY_BASE,     VA_KERNEL_BASE + PA_DEVICE_MEMORY_BASE
.set VA_DEVICE_MEMORY_END,      VA_DEVICE_MEMORY_BASE + DEVICE_MEMORY_SIZE

.set GPIO_BASE_PA, 0xfe200000
.set AUX_BASE_PA,  0xfe215000

    /* Early kvtophys before page tables are set up. Params:
        va:    virtual address
        outpa: returned physical address
    */
.macro early_kvtophys va, outpa
    ldr \outpa, =VA_KERNEL_BASE
    sub \outpa, \va, \outpa
    add \outpa, \outpa, PA_KERNEL_BASE
.endm

    /* TODO Parameter setup will change once we are rebased to
    VA_KERNEL_BASE */
    /* Will have to call early_kvtophys once rebased to VA_KERNEL_BASE */
.macro early_map_range start, end, prot, block, device
    adrp x0, \start
    bl _early_phystokv
    adrp x1, \start
    mov w2, \prot
    mov w3, \block
    adrp x4, \end
    sub x4, x4, x1
    mov w5, \device
    mov x6, sp
    bl _early_map_range 
    ldr x19, [sp]
.endm

.macro early_map_range2 start, end, prot, block, device
    ldr x0, =\start
    bl _early_phystokv
    ldr x1, =\start
    mov w2, \prot
    mov w3, \block
    ldr x4, =\end
    sub x4, x4, x1
    mov w5, \device
    mov x6, sp
    bl _early_map_range 
    ldr x19, [sp]
.endm

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

    /* Set exception vector before attempting any loads/stores */
    adr x5, ExceptionVectorsBase
    msr vbar_el1, x5

    /* AttrIdx 0 is normal memory, AttrIdx 1 is device memory */
    mov x5, #0xff
    msr mair_el1, x5

    /* Set up page tables for EL1 inside EL2 so we don't have to deal with
    creating an identity mapping. We are using a 4k page size. T1SZ/T0SZ
    are both 25 so we start at level 1 for address translation.

    TCR_EL1:
        - IPS: 0
        - TG1: 2
        - TG0: 0
        - T1SZ: 25
        - SH0: 0
        - T0SZ: 25
    */
    mov x5, #0x19
    movk x5, #0x8019, lsl #16
    msr tcr_el1, x5

    /* Give EL2 cpu0's stack and then bzero it before we eret */
    adrp x5, cpu0_stack
    add x5, x5, #0x1000
    msr SPSel, #0
    mov sp, x5

    isb sy

    /* Save boot parameters */
    stp x0, x1, [sp, #-0x10]!
    stp x2, x3, [sp, #-0x10]!

    /*b Leret_to_el1*/

    adr x19, kernel_level1_table
    adrp x20, kernel_level2_table
    adrp x21, kernel_level3_table
    adrp x22, stacks_end

    adrp x0, static_pagetables_start
    adrp x1, static_pagetables_end
    sub x1, x1, x0
    bl _bzero_64

    /* TODO will need to be a phys address when rebased to VA_KERNEL_BASE */
    /* Pointer to current L3 table */
    adrp x19, kernel_level3_table
    str x19, [sp, #-0x10]!
    
    /* TODO: gonna have to somehow save the current L3 table for
    kernel's use after everything is mapped, stash it in tpidr_el1? */

    early_map_range text_start text_end VM_PROT_READ|VM_PROT_EXECUTE 0 0
    early_map_range rodata_start rodata_end VM_PROT_READ 0 0
    early_map_range data_start data_end VM_PROT_READ|VM_PROT_WRITE 0 0
    early_map_range bss_start bss_end VM_PROT_READ|VM_PROT_WRITE 0 0
    early_map_range stacks_start stacks_end VM_PROT_READ|VM_PROT_WRITE 0 0

    /* Everything mapped before this point was contiguous. Now we're
    jumping like 3GB forward to map device memory, so re-initialize
    the L3 table pointer. TODO: need to save whereever L3 table pointer
    left off before this as well as after this so the kernel can
    figure out which phys pages are free */

    /* Beginning of device memory L3 table will be at 0x40000000 */
    /* TODO: wont be needed once I get this working with L2 block mappings */
    /*
    mov x19, #0x1
    lsl x19, x19, #30
    str x19, [sp]

    mov x0, x19
    mov x1, #0x4000000
    bl _bzero_64
    */

    early_map_range2 PA_DEVICE_MEMORY_BASE PA_DEVICE_MEMORY_END VM_PROT_READ|VM_PROT_WRITE 1 1

    dsb sy
    isb sy

    /* We do not map page tables. Doesn't really make sense to, since
    they don't change all the time, and TTEs output physical addresses
    to next level tables. If a PTE needs to be changed we can always
    just map the page its on, change it, then unmap */

    adr x5, kernel_level1_table
    /* early_kvtophys only when we start at 0xffffff8000000000 in linkscript */
    /*early_kvtophys x5, x5*/
    msr ttbr1_el1, x5

    dsb sy
    isb sy
    tlbi vmalle1
    isb sy

Leret_to_el1:
    /* Enable caches, instruction cache, SP alignment checking,
    and WXN, but not the MMU yet */
    /*mov x5, #0x100c*/
    mov x5, #0x100d
    orr x5, x5, #(1 << 19)
    msr sctlr_el1, x5

    dsb sy
    isb sy
    tlbi vmalle1
    isb sy

    /*ldr x0, =VA_KERNEL_BASE*/
    /*adrp x0, text_start*/
    /*add x0, x0, #0x2000*/
    /*adrp x0, rodata_start*/
    /*adrp x0, pagetables_end*/
    /*sub x0, x0, #0x1000*/
    /*adrp x0, bss_start*/
    /*adrp x0, device_memory_start*/
    /*ldr x0, =PA_DEVICE_MEMORY_BASE*/
    /*ldr x0, =GPIO_BASE_PA*/
    /*ldr x0, =AUX_BASE_PA*/
    mov x0, #0x80000
    movk x0, #0x5000
    bl _early_phystokv
    msr tpidr_el0, x0
    /*add x0, x0, #0x1000*/
    bl _transtest_read
    msr tpidr_el1, x0

    isb sy

    mrs x5, sctlr_el1
    and x5, x5, #(~1)
    msr sctlr_el1, x5

    dsb sy
    isb sy
    tlbi vmalle1
    isb sy

    /* Free the space used for current L3 table pointer */
    add sp, sp, #0x10

    /* Restore boot parameters */
    ldp x0, x1, [sp], #0x10
    ldp x2, x3, [sp], #0x10

    /* TODO Need to bzero unused phys (start from static pagetables end
    and go to the start of VC SDRAM, then the region after VC SDRAM */

    /* TODO this eret will cause a panic since el1_entry is still
    a physical address */
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

    stp x0, x1, [sp, #-0x10]!
    stp x2, x3, [sp, #-0x10]!

    adr x0, bss_start
    adr x1, bss_end
    sub x1, x1, x0
    bl _bzero

    ldp x0, x1, [sp], #0x10
    ldp x2, x3, [sp], #0x10

    mov x5, xzr
    mov x6, xzr
    mov x7, xzr
    mov x8, xzr
    mov x9, xzr
    mov x10, xzr
    mov x11, xzr
    mov x12, xzr
    mov x13, xzr
    mov x14, xzr
    mov x15, xzr
    mov x16, xzr
    mov x17, xzr
    mov x18, xzr
    mov x19, xzr
    mov x20, xzr
    mov x21, xzr
    mov x22, xzr
    mov x23, xzr
    mov x24, xzr
    mov x25, xzr
    mov x26, xzr
    mov x27, xzr
    mov x28, xzr
    mov x29, xzr
    mov x30, xzr

    bl _main
    /* Not reached */

    /* Not cpu0? */
Lnot_cpu0:
    b _spin_forever

    /* Not booted into EL2? */
Lnot_el2:
    b _spin_forever

_spin_forever:
    b .

_bzero:
    add x2, x0, x1
Lzero_loop:
    strb wzr, [x0], #0x1
    cmp x0, x2
    b.ne Lzero_loop
    ret

_bzero_16:
    add x2, x0, x1
Lzero_loop_16:
    stp xzr, xzr, [x0], #0x10
    cmp x0, x2
    b.ne Lzero_loop_16
    ret

_bzero_64:
    add x2, x0, x1
Lzero_loop_64:
    stp xzr, xzr, [x0], #0x10
    stp xzr, xzr, [x0], #0x10
    stp xzr, xzr, [x0], #0x10
    stp xzr, xzr, [x0], #0x10
    cmp x0, x2
    b.ne Lzero_loop_64
    ret

    /* Given a mask of VM protections, return the equivalent TTE protections.
        Parameters:
            w0: VM permissions
            x1: pointer to PXN bit, will get set according to x0
    */
_vm_prot_to_tte_prot:
    stp x19, x20, [sp, #-0x10]!
    stp x21, x22, [sp, #-0x10]!
    stp x29, x30, [sp, #-0x10]!

    mov w19, w0
    mov x20, x1

    /* Assume PXN bit has to be set */
    mov w21, #0x1

    /* Assume AP_RWNA */
    mov w22, wzr

    tbz w19, #VM_PROT_EXECUTE_BIT, Lset_rw_perms

Lx:
    mov w21, wzr

Lset_rw_perms:
    tbnz w19, #VM_PROT_WRITE_BIT, Lvm_prot_to_tte_prot_done

Lro:
    mov w22, #0x2

Lvm_prot_to_tte_prot_done:
    str w21, [x20]

    mov x0, x22

    ldp x29, x30, [sp], #0x10
    ldp x21, x22, [sp], #0x10
    ldp x19, x20, [sp], #0x10
    ret

    /* Given a physical page, map it to the specified virtual page.
    The version here does not work when the MMU is enabled since
    we deal exclusively with physical memory in EL2.

    Parameters:
        x0: virtual address
        x1: physical address
        w2: virtual protection
        x3: current L3 table
        w4: set to one if we are mapping device memory
    */
_early_map_page:
    /* Makes no sense to have a mapping that can't be read from
    this early on */
    tbz w2, #VM_PROT_READ_BIT, _spin_forever

    stp x19, x20, [sp, #-0x10]!
    stp x21, x22, [sp, #-0x10]!
    stp x23, x24, [sp, #-0x10]!
    stp x25, x26, [sp, #-0x10]!
    stp x27, x28, [sp, #-0x10]!
    stp x29, x30, [sp, #-0x10]!
    add x29, sp, #0x10

    mov x19, x0
    mov x20, x1
    mov w21, w2
    mov x23, x3
    mov w24, w4
    
    /* Return value */
    mov w25, wzr

    /* Prep: from this point forward:
        x4: physical address of start of level 1 table
        x5: physical address of start of level 2 table
    TODO when we are rebased to 0xffffff8000000000 we need to do
    early_kvtophys on the above two registers
        x6: execute permission flag
        x7 - x23: scratch registers
    */

    adr x4, kernel_level1_table
    adrp x5, kernel_level2_table

    /* Turn w2 into the TTE-specific protection value. At this point
    we are only mapping for kernel so userspace doesn't matter */

    sub sp, sp, #0x10
    mov w0, w21
    mov x1, sp
    bl _vm_prot_to_tte_prot
    mov w2, w0
    ldr w6, [sp], #0x10

    /* w2 == TTE protections
       w6 == PXN bit
    */

    /*
    mov w6, #0x1
    tbz w2, #VM_PROT_EXECUTE_BIT, Lset_rw_perms

Lx:
    mov w6, wzr

Lset_rw_perms:
    tbz w2, #VM_PROT_WRITE_BIT, Lro

Lrw:
    mov w2, wzr
    b Lget_l1_entry

Lro:
    mov w2, #0x2
    */

Learly_map_page_get_l1_entry:
    /* First, get the index into the level 1 table. We do not have more
    than 512 GB mapped from VA_KERNEL_BASE upward so this table is only
    one page */
    lsr x7, x19, L1_SHIFT
    and x7, x7, INDEX_MASK
    add x4, x4, x7, lsl #0x3
    ldr x8, [x4]
    /* Already a valid L1 entry? */
    tbnz x8, #0, Learly_map_page_get_l2_entry

Learly_map_page_make_l1_entry:
    /* First, figure out the correct L2 table to use. This is easy
    because we have exactly 512 GB of virtual kernel address space.
    The level 1 index for VA_KERNEL_BASE is just 0x0, and we only
    map upwards. So, to figure out the L2 table we need to use,
    we do (l1_idx << 12) + kernel_level2_table.

    x4 still points to the L1 table entry we need to fill and x7
    is still the L1 table index. */
    lsl x7, x7, #12
    add x7, x7, x5

    /* x7 == base of L2 table, now we can form the L1 table entry */

    /* Set NSTable bit */
    orr x7, x7, #(1 << 63)

    /* Keep all other bits zero. APTable stays zero so subsequent levels
    of lookup are read/write, PXNTable stays zero so we can do
    instruction fetches for executable memory */

    /* Set table entry type bit/valid bit */
    orr x7, x7, #0x3

    /* Write the entry to the L1 table */
    str x7, [x4]

    mov x8, x7

Learly_map_page_get_l2_entry:
    /* Second, get the index into the level 2 table. x8 is the L1
    table entry */
    lsr x7, x19, L2_SHIFT
    and x7, x7, INDEX_MASK
    and x8, x8, TABLE_MASK
    add x8, x8, x7, lsl #0x3
    ldr x9, [x8]
    /* Already a valid L2 entry? */
    tbnz x9, #0, Learly_map_page_get_l3_entry

Learly_map_page_make_l2_entry:
    /* It's harder to figure out the address of the correct L3 table
    to use because the kernel virtual address space covers more than
    1 GB. To solve this problem, I pass a pointer to the current L3
    table to this function as an argument so we can go off of that. x8
    points to the L2 table index. */
    mov x7, x3

    /* Upper table attributes are res0 for stage 2 */

    /* x7 == base of current L3 table, now we can form the L2 table entry */

    /* Set table entry type bit/valid bit */
    orr x7, x7, #0x3

    /* Write the entry to the L2 table */
    str x7, [x8]

    mov x9, x7

Learly_map_page_get_l3_entry:
    /* Finally, get the index into the L3 table. x9 is the L2 table entry */
    lsr x7, x19, L3_SHIFT
    and x7, x7, INDEX_MASK
    and x9, x9, TABLE_MASK
    add x9, x9, x7, lsl #0x3
    ldr x10, [x9]
    /* Already a valid L3 entry? */
    tbnz x10, #0, Learly_map_done

Learly_map_page_make_l3_entry:
    /* Since this is the last level, and we already have the physical
    address we want to map, we can start to build the L3 entry. */
    mov x10, x20

    /* x10 == physical address */

    /* Set UXN bit to one since these are kernel pages */
    mov x11, #0x1
    lsl x11, x11, #54
    orr x10, x10, x11

    /* Set PXN bit */
    lsl x11, x6, #53
    orr x10, x10, x11

    /* Set access flag, otherwise we'd fault the first time we do
    address translation for a some page */
    orr x10, x10, #(1 << 10)

    /* SH bits are zero, intentionally */

    /* Set AP bits */
    lsl x11, x2, #6
    orr x10, x10, x11

    /* Set NS bit */
    orr x10, x10, #(1 << 5)

    /* Set AttrIdx */
    lsl w24, w24, #0x2
    orr x10, x10, x24
    
    /* Set table entry type bit/valid bit */
    orr x10, x10, #0x3

    /* Write the entry to the L3 table */
    str x10, [x9]

    mov w25, #0x1

Learly_map_done:
    dsb sy
    isb sy
    
    mov w0, w25

    ldp x29, x30, [sp], #0x10
    ldp x27, x28, [sp], #0x10
    ldp x25, x26, [sp], #0x10
    ldp x23, x24, [sp], #0x10
    ldp x21, x22, [sp], #0x10
    ldp x19, x20, [sp], #0x10
    ret

    /* Given a physical 2MB block, map it to the specified virtual
    address. The version here does not work when the MMU is enabled since
    we deal exclusively with physical memory in EL2.

    Parameters:
        x0: virtual address
        x1: physical address
        w2: virtual protection
        w3: set to one if we are mapping device memory
    */
_early_map_block:
    /* Makes no sense to have a mapping that can't be read from
    this early on */
    tbz w2, #VM_PROT_READ_BIT, _spin_forever

    stp x19, x20, [sp, #-0x10]!
    stp x21, x22, [sp, #-0x10]!
    stp x23, x24, [sp, #-0x10]!
    stp x25, x26, [sp, #-0x10]!
    stp x27, x28, [sp, #-0x10]!
    stp x29, x30, [sp, #-0x10]!
    add x29, sp, #0x10

    mov x19, x0
    mov x20, x1
    mov w21, w2
    mov w22, w3

    sub sp, sp, #0x10
    mov w0, w21
    mov x1, sp
    bl _vm_prot_to_tte_prot
    mov w8, w0
    ldr w9, [sp], #0x10

    adr x10, kernel_level1_table
    adrp x11, kernel_level2_table

Learly_map_block_get_l1_entry:
    lsr x12, x19, L1_SHIFT
    and x12, x12, INDEX_MASK
    add x13, x10, x12, lsl #0x3
    ldr x14, [x13]
    /* Already a valid L1 entry? */
    tbnz x14, #0, Learly_map_block_get_l2_block_entry

Learly_map_block_make_l1_entry:
    lsl x12, x12, #12

    /* Physical address of a L2 table */
    add x14, x11, x12

    /* Set NSTable bit */
    orr x14, x14, #(1 << 63)

    /* Set table entry type bit/valid bit */
    orr x14, x14, #0x3

    /* Write the entry to the L1 table */
    str x14, [x13]

Learly_map_block_get_l2_block_entry:
    lsr x12, x19, L2_SHIFT
    and x12, x12, INDEX_MASK
    and x14, x14, TABLE_MASK
    add x13, x14, x12, lsl #0x3
    ldr x14, [x13]
    /* Already a valid L2 block entry? */
    tbnz x14, #0, Learly_map_block_done

Learly_map_block_make_l2_block_entry:
    /* Output address */
    mov x14, x20

    /* Set UXN bit to one since these are kernel pages */
    orr x14, x14, #(1 << 54)

    /* Set PXN bit */
    lsl x15, x9, #53
    orr x14, x14, x15

    /* Set access flag, otherwise we'd fault the first time we do
    address translation for a some page */
    orr x14, x14, #(1 << 10)

    /* SH bits are zero, intentionally */

    /* Set AP bits */
    lsl w8, w8, #6
    orr x14, x14, x8

    /* Set NS bit */
    orr x14, x14, #(1 << 5)

    /* Set AttrIdx */
    lsl w22, w22, #0x2
    orr x14, x14, x22

    /* Set block entry type/valid bit */
    orr x14, x14, #0x1

    str x14, [x13]

Learly_map_block_done:
    dsb sy
    isb sy

    ldp x29, x30, [sp], #0x10
    ldp x27, x28, [sp], #0x10
    ldp x25, x26, [sp], #0x10
    ldp x23, x24, [sp], #0x10
    ldp x21, x22, [sp], #0x10
    ldp x19, x20, [sp], #0x10
    ret

    /* Map a physically-contiguous range onto contiguous virtual kernel
    address space, but in 2MB blocks rather than 4KB pages.

    Parameters:
        x0: virtual address to map
        x1: physical address
        w2: virtual protection
        w3: set to one if we're creating 2MB L2 block mappings
        x4: the size of the region
        w5: set to one if we're mapping device memory

    !!! This function is meant to be called only from _early_map_range !!!
    */
_early_map_block_range:
    mov x19, x0
    mov x20, x1
    mov w21, w2
    /* w3 ignored */
    add x22, x1, x4
    mov w23, w5
    
Lnextblock:
    mov x0, x19
    mov x1, x20
    mov w2, w21
    mov w3, w23
    bl _early_map_block

Lnextblockprep:
    add x19, x19, #0x200000
    add x20, x20, #0x200000
    cmp x20, x22
    b.ne Lnextblock

    ldp x29, x30, [sp], #0x10
    ldp x25, x26, [sp], #0x10
    ldp x23, x24, [sp], #0x10
    ldp x21, x22, [sp], #0x10
    ldp x19, x20, [sp], #0x10
    ret

    /* Map a physically-contiguous range onto contiguous virtual kernel
    address space.

    Parameters:
        x0: virtual address to map
        x1: physical address
        w2: virtual protection
        w3: set to one if we're creating 2MB L2 block mappings
        x4: the size of the region
        w5: set to one if we're mapping device memory
        x6: pointer to the pointer to the current page of the L3 table
    */
_early_map_range:
    stp x19, x20, [sp, #-0x10]!
    stp x21, x22, [sp, #-0x10]!
    stp x23, x24, [sp, #-0x10]!
    stp x25, x26, [sp, #-0x10]!
    stp x29, x30, [sp, #-0x10]!
    add x29, sp, #0x10

    cbz x4, Learly_map_range_done
    cbnz w3, _early_map_block_range

    mov x19, x0
    mov x20, x1
    mov w21, w2
    mov w22, w3
    mov x23, x4
    add x24, x1, x4
    mov w25, w5
    mov x26, x6

    /* Figure out how many entries are currently in this L3 table */
    ldr x8, [x26]

Lfind_empty_l3_entry:
    ldr x9, [x8], #0x8
    cmp x9, xzr
    b.ne Lfind_empty_l3_entry

    /*and x8, x8, #0xff*/
    /* pageoff/8 will give us the index of the first empty entry */
    and x8, x8, #0xfff
    lsr x8, x8, #3
    str x8, [sp, #-0x10]!

Lnextpage:
    mov x0, x19
    mov x1, x20
    mov w2, w21
    ldr x3, [x26]
    mov w4, w25
    bl _early_map_page
    cmp w0, wzr
    b.eq Lnextpageprep

    /* Just added another entry */
    ldr x8, [sp]
    add x8, x8, #0x1
    str x8, [sp]

    cmp x8, L3_ENTRIES_LIMIT
    b.eq Ladjust_l3_table_ptr
    b Lnextpageprep

Ladjust_l3_table_ptr:
    ldr x8, [x26]
    add x8, x8, #0x1000
    str x8, [x26]

    /* We are starting on a new page of L3, so reset the counter */
    str xzr, [sp]

Lnextpageprep:
    add x19, x19, PAGE_SIZE
    add x20, x20, PAGE_SIZE
    cmp x20, x24
    b.ne Lnextpage

    add sp, sp, #0x10

Learly_map_range_done:
    ldp x29, x30, [sp], #0x10
    ldp x25, x26, [sp], #0x10
    ldp x23, x24, [sp], #0x10
    ldp x21, x22, [sp], #0x10
    ldp x19, x20, [sp], #0x10
    ret

    /* This can only clobber x1!!! */
_early_phystokv:
    ldr x1, =VA_KERNEL_BASE
    add x0, x1, x0
    ret

_transtest_read:
    at s1e1r, x0
    isb sy
    mrs x0, par_el1
    ret
