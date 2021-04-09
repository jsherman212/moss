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

    /* Early kvtophys before page tables are set up. Params:
        va:    virtual address
        outpa: returned physical address
    */
.macro early_kvtophys va, outpa
    ldr \outpa, =VA_KERNEL_BASE
    sub \outpa, \va, \outpa
    add \outpa, \outpa, PA_KERNEL_BASE
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

    /*
    adr x2, kernel_begin
    adrp x3, kernel_end
    adr x4, kernel_level1_table
    adr x5, kernel_level2_table
    adrp x6, kernel_level3_table
    ldr x7, =VA_KERNEL_BASE
    */

    /*
    adr x5, kernel_level1_table
    adr x6, kernel_level2_table
    ldr x7, =VA_KERNEL_BASE
    */

    /* Map first page of kernel for testing */
    /*add_l1_entry x4, x5, x7, x19, x20*/
    /*add_l1_entry x5 x6 x7 x19 x20 x21*/

    adr x19, kernel_level1_table
    adrp x20, kernel_level2_table
    adrp x21, kernel_level3_table

    /* Save boot parameters */
    mov x25, x0
    mov x26, x1
    mov x27, x2
    mov x28, x3

    adr x0, kernel_level1_table
    adrp x1, translation_tables_end
    sub x1, x1, x0
    bl _bzero_16

    ldr x0, =VA_KERNEL_BASE
    mov x1, PA_KERNEL_BASE
    /* r-x */
    mov w2, #(VM_PROT_READ | VM_PROT_EXECUTE)
    adrp x3, kernel_level3_table
    bl _early_map_page

    /* Restore boot parameters */
    mov x0, x25
    mov x1, x26
    mov x2, x27
    mov x3, x28

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

    ldr x0, =VA_KERNEL_BASE
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

    /* Given a physical page, map it to the specified virtual page.
    The version here does not work when the MMU is enabled since
    we deal exclusively with physical memory.

    Parameters:
        x0: virtual address
        x1: physical address
        w2: virtual protection
        x3: current L3 table
    */
_early_map_page:
    mov x29, x30

    /* Makes no sense to have a mapping that can't be read from
    this early on */
    tbz w2, #VM_PROT_READ_BIT, _spin_forever

    /* Prep: from this point forward:
        x3: physical address of start of level 1 table
        x4: virtual address of start of level 1 table
        x5: physical address of start of level 2 table
        x6: virtual address of start of level 2 table
        x7: physical address of start of level 3 table
        x8: virtual address of start of level 3 table

        x4: physical address of start of level 1 table
        x5: physical address of start of level 2 table
    TODO when we are rebased to 0xffffffc000000000 we need to do
    early_kvtophys on the above two registers

        x6: execute permission flag
        x7 - x24: scratch registers
    */

    /* This will change once we rebase to 0xffffffc000000000 in linker script */
    /*adr x4, kernel_level1_table*/
    /* early_kvtophys on x4 for x3 once we're rebased */
    /*mov x3, x4*/

    /*adr x6, kernel_level2_table*/
    /* early_kvtophys on x6 for x5 once we're rebased */
    /*mov x5, x6*/

    /*adr x8, kernel_level3_table*/
    /* early_kvtophys on x8 for x7 once we're rebased */
    /*mov x7, x8*/

    adr x4, kernel_level1_table
    adrp x5, kernel_level2_table

    /* Turn w2 into the TTE-specific protection value. At this point
    we are only mapping for kernel so userspace doesn't matter */

    /* PXN, so let's assume non-executable */
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

Lget_l1_entry:
    /* First, get the index into the level 1 table. We do not have more
    than 512 GB mapped from VA_KERNEL_BASE upward so this table is only
    one page */
    lsr x7, x0, L1_SHIFT
    and x7, x7, INDEX_MASK
    add x4, x4, x7, lsl #0x3
    ldr x8, [x4]
    /* Already a valid L1 entry? */
    tbnz x8, #0, Lget_l2_entry

Lmake_l1_entry:
    /* First, figure out the correct L2 table to use. This is easy
    because we have exactly 512 GB of virtual kernel address space.
    The level 1 index for VA_KERNEL_BASE is just 0x0, and we only
    map upwards. So, to figure out the L2 table we need to use,
    we do (l1_idx << 12) + kernel_level1_table.

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

Lget_l2_entry:
    /* Second, get the index into the level 2 table. x8 is the L1
    table entry */
    lsr x7, x0, L2_SHIFT
    and x7, x7, INDEX_MASK
    and x8, x8, TABLE_MASK
    add x8, x8, x7, lsl #0x3
    ldr x9, [x8]
    /* Already a valid L2 entry? */
    tbnz x9, #0, Lget_l3_entry

Lmake_l2_entry:
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

Lget_l3_entry:
    /* Finally, get the index into the L3 table. x9 is the L2 table entry */
    lsr x7, x0, L3_SHIFT
    and x7, x7, INDEX_MASK
    and x9, x9, TABLE_MASK
    add x9, x9, x7, lsl #0x3
    ldr x10, [x9]
    /* Already a valid L3 entry? */
    tbnz x10, #0, Learly_map_done

Lmake_l3_entry:
    /* Since this is the last level, and we already have the physical
    address we want to map, we can start to build the L3 entry. */
    mov x10, x1

    /* x10 == physical address */

    /* Set PXN bit */
    lsl x11, x6, #53
    orr x10, x10, x11

    /* Set access flag, otherwise we'd fault the first time we do
    address translation for a given page */
    orr x10, x10, #(1 << 10)

    /* SH bits are zero, intentionally */

    /* Set AP bits */
    lsl x11, x2, #6
    orr x10, x10, x11

    /* Set NS bit */
    orr x10, x10, #(1 << 5)

    /* AttrIdx is zero, intentionally
    TODO: need a parameter for mapping device memory */
    
    /* Set table entry type bit/valid bit */
    orr x10, x10, #0x3

    /* Write the entry to the L3 table */
    str x10, [x9]

Learly_map_done:
    dsb sy
    isb sy

    mov x30, x29

    ret

_transtest_read:
    at s1e1r, x0
    isb sy
    mrs x0, par_el1
    ret
