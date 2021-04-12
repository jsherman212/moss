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

.set static_pagetables_end_TEST,    0xffffff8000209000
/*.set bad_pte_addr,                  0xffffff8000200000*/
/*.set bad_pte_addr,                  0xffffff80001ff000*/
.set bad_pte_addr,                  0xffffff8000200000

.macro early_map_range start, end, prot, block, device
    ldr x0, =\start
    bl _early_kvtophys
    mov x1, x0
    ldr x0, =\start
    mov w2, \prot
    mov w3, \block
    ldr x4, =\end
    sub x4, x4, x0
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

    mrs x5, CurrentEL
    lsr x5, x5, #0x2
    cmp x5, #0x2
    b.eq Lel2_entry
    /*
    cmp x5, #0x3
    b.eq Lel3_entry
    */
    b Lnot_el2

    /*
Lel3_entry:
    adr x5, Lel2_entry
    msr elr_el3, x5
    mov x5, #0x3c8
    msr spsr_el3, x5
    isb sy
    eret
    */

Lel2_entry:
    mrs x5, CurrentEL
    /* Give EL2 cpu0's stack and then bzero it before we eret */
    /*adrp x5, cpu0_stack*/
    adr x5, cpu0_stack
    /*ldr x5, =cpu0_stack*/
    add x5, x5, #0x1000
    msr SPSel, #0
    mov sp, x5
    
    mov x2, #0x4141

    isb sy

    /* Save boot parameters */
    stp x0, x1, [sp, #-0x10]!
    stp x2, x3, [sp, #-0x10]!

    dmb sy
    dsb sy
    isb sy

    /* TODO: elr has to be virtual? */
    /*
    mov x29, x0
    mov x0, x5
    bl _early_kvtophys
    mov x5, x0
    mov x0, x29
    */

    /*adr x0, Lel1_entry*/
    ldr x0, =Lel1_entry
    /*bl _early_kvtophys*/
    msr elr_el2, x0
    /*msr elr_el3, x0*/

    /* SPSR: mask all interrupts when we return to EL1 */
    mov x0, #0x3c4
    msr spsr_el2, x0
    /*msr spsr_el3, x0*/

    /* EL1 is AArch64 */
    mov x0, #(1 << 31)
    msr hcr_el2, x0

    /* Enable floating point for EL1 */
    mrs x0, cpacr_el1
    orr x0, x0, #0x300000
    msr cpacr_el1, x0

    /* Enable floating point for EL2 */
    /*
    mrs x0, cptr_el2
    orr x0, x0, #0x300000
    msr cptr_el2, x0
    */

    isb sy

    /*adr x0, ExceptionVectorsBase*/
    ldr x0, =ExceptionVectorsBase
    msr vbar_el1, x0

    /* AttrIdx 0 is normal memory, AttrIdx 1 is device memory */
    mov x0, #0xff
    msr mair_el1, x0

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
    mov x0, #0x19
    movk x0, #0x8019, lsl #16
    msr tcr_el1, x0

    /*b Leret_to_el1*/

    /* We zero out all L1 and L2, but only some of L3. We don't need to zero
    out all of L3 because we should not be trying to map over existing
    L3 entries this early on */
    adrp x0, static_pagetables_start
    adrp x1, static_pagetables_end
    sub x1, x1, x0
    /*
    adrp x2, static_pagetables_start
    ldr x0, =static_pagetables_end_TEST
    bl _early_kvtophys
    mov x1, x0
    mov x0, x2
    sub x1, x1, x0
    */

    /* TODO Too slow on qemu */
    bl _bzero_64
    /*bl _bzero_8*/
    /*bl _bzero*/

    /*
    dsb sy
    isb sy
    */

    /* Pointer to current L3 table, must be x19 for the
    early_map_range macro */
    adrp x19, kernel_level3_table
    str x19, [sp, #-0x10]!
    
    /* TODO: gonna have to somehow save the current L3 table for
    kernel's use after everything is mapped, stash it in tpidr_el1? */

    early_map_range text_start text_end VM_PROT_READ|VM_PROT_EXECUTE 0 0
    early_map_range rodata_start rodata_end VM_PROT_READ 0 0
    early_map_range data_start data_end VM_PROT_READ|VM_PROT_WRITE 0 0
    early_map_range bss_start bss_end VM_PROT_READ|VM_PROT_WRITE 0 0
    early_map_range stacks_start stacks_end VM_PROT_READ|VM_PROT_WRITE 0 0

    /* Now we gotta map the pagetables that map text to the end
    of stacks so we can create new virtual mappings after the MMU has
    been enabled. We need the first L3 table for these pagetables to
    bzero the region */
    /*
    adrp x0, static_pagetables_start
    adr x1, kernel_level1_table
    adr x2, kernel_level2_table


    lsr x1, x0, L1_SHIFT
    and x1, x1, INDEX_MASK
    add x2, x1, 
    */

    /* TODO: update l3 pointer on the stack */


    /* Map the pagetables for static kernel memory so we can create
    new mappings after the MMU is enabled. The pagetables end up being
    some ways away from the ones that map text, rodata, etc, so we
    need to calculate where they'll be so we can bzero that region */

    /*
    ldr x0, =static_pagetables_start
    bl _early_kvtophys
    mov x1, x0
    ldr x0, =static_pagetables_start
    mov w2, #(VM_PROT_READ | VM_PROT_WRITE)
    mov w3, wzr
    ldr x4, =static_pagetables_end
    sub x4, x4, x0
    lsr x4, x4, #0x3
    add x4, x4, PAGE_SIZE
    and x4, x4, #~(PAGE_SIZE - 1)
    mov w5, wzr
    mov x6, sp
    bl _early_map_range 
    ldr x19, [sp]
    */

    /*
    ldr x19, [sp]
    add x19, x19, PAGE_SIZE
    and x19, x19, #~(PAGE_SIZE - 1)
    str x19, [sp]
    */

    /*early_map_range kernel_level1_table kernel_level2_table VM_PROT_READ|VM_PROT_WRITE 0 0*/
    early_map_range kernel_level1_table kernel_level2_table VM_PROT_READ 0 0

    /*
    ldr x19, [sp]
    add x19, x19, PAGE_SIZE
    and x19, x19, #~(PAGE_SIZE - 1)
    str x19, [sp]
    */

    /*early_map_range kernel_level2_table kernel_level3_table VM_PROT_READ|VM_PROT_WRITE 0 0*/
    early_map_range kernel_level2_table kernel_level3_table VM_PROT_READ|VM_PROT_WRITE 0 0

    ldr x0, =kernel_level3_table
    bl _early_kvtophys
    mov x1, x0
    ldr x0, =kernel_level3_table
    mov w2, #(VM_PROT_READ | VM_PROT_WRITE | VM_PROT_EXECUTE)
    mov w3, wzr
    /* Add an extra page to the current L3 table so we take into account
    PTEs that are on the current L3 page. Otherwise, those would stay
    unmapped */
    add x19, x19, PAGE_SIZE
    and x19, x19, #~(PAGE_SIZE - 1)
    sub x4, x19, x1
    mov w5, wzr
    mov x6, sp
    bl _early_map_range 
    ldr x19, [sp]

    /*early_map_range static_pagetables_start static_pagetables_end VM_PROT_READ|VM_PROT_WRITE 0 0*/

    /*early_map_range static_pagetables_start static_pagetables_end_TEST VM_PROT_READ|VM_PROT_WRITE 0 0*/

    /* We map device memory as 2MB L2 blocks to save space */
    early_map_range VA_DEVICE_MEMORY_BASE VA_DEVICE_MEMORY_END VM_PROT_READ|VM_PROT_WRITE 1 1

    dsb sy
    isb sy

    adr x0, kernel_level1_table
    msr ttbr1_el1, x0

    /*
    adrp x0, kernel_level3_table
    msr tpidr_el1, x0
    bl _early_phystokv
    msr tpidr_el0, x0
    */

    dsb sy
    isb sy
    tlbi vmalle1
    isb sy

Leret_to_el1:
    /* Enable caches, instruction cache, SP alignment checking,
    WXN, and MMU for address translation inside EL1 */
    /*mov x5, #0x100c*/
    mov x5, #0x100d
    orr x5, x5, #(1 << 19)
    msr sctlr_el1, x5

    dsb sy
    isb sy
    tlbi vmalle1
    isb sy

    /*
    mov x0, #0x4000
    msr vbar_el3, x0
    */

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
    /*
    mov x0, #0x80000
    movk x0, #0x5000
    bl _early_phystokv
    msr tpidr_el0, x0
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
    */

    /*ldr x0, =GPIO_BASE_PA*/
    /*ldr x0, =stacks_start*/
    /*ldr x0, =VA_DEVICE_MEMORY_BASE*/
    /*
    ldr x0, =bad_pte_addr
    bl _early_kvtophys
    msr tpidr_el0, x0
    ldr x0, =bad_pte_addr
    bl _transtest_read
    msr tpidr_el1, x0
    */

    /* Free the space used for current L3 table pointer */
    add sp, sp, #0x10

    /* Restore boot parameters */
    ldp x2, x3, [sp], #0x10
    ldp x0, x1, [sp], #0x10

    /* TODO Need to bzero unused phys (start from static pagetables end
    and go to the start of VC SDRAM, then the region after VC SDRAM) */

    eret

Lel1_entry:
    mrs x5, CurrentEL
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

_bzero_8:
    add x1, x0, x1
Lzero_loop_8:
    str xzr, [x0], #0x8
    cmp x0, x1
    b.ne Lzero_loop_8
    ret

_bzero_16:
    add x1, x0, x1
Lzero_loop_16:
    stp xzr, xzr, [x0], #0x10
    cmp x0, x1
    b.ne Lzero_loop_16
    ret

_bzero_32:
    movi v0.2d, #0
    add x2, x0, x1
Lzero_loop_32:
    stp q0, q0, [x0], #0x20
    cmp x0, x2
    b.ne Lzero_loop_32
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

    This doesn't check for duplicate L3 entries since we're mapping
    static kernel memory.

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

    adr x4, kernel_level1_table
    adrp x5, kernel_level2_table

    /* Turn w2 into the TTE-specific protection value. At this point
    we are only mapping for kernel so userspace doesn't matter */

    sub sp, sp, #0x10
    mov w0, w21
    mov x1, sp
    bl _vm_prot_to_tte_prot
    mov x21, x0
    ldr x26, [sp], #0x10

    /* w2 == TTE protections
       w6 == PXN bit
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

    dsb sy
    isb sy

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
    table to this function as an argument so we can go off of that
    inside x3. */
    mov x7, x3

    /* Upper table attributes are res0 for level 2 */

    /* x7 == base of current L3 table, now we can form the L2 table entry */

    /* Set table entry type bit/valid bit */
    orr x7, x7, #0x3

    /* Write the entry to the L2 table */
    str x7, [x8]

    dsb sy
    isb sy

    mov x9, x7

Learly_map_page_get_l3_entry:
    /* Finally, get the index into the L3 table. x9 is the L2 table entry */
    lsr x7, x19, L3_SHIFT
    and x7, x7, INDEX_MASK
    and x9, x9, TABLE_MASK
    add x9, x9, x7, lsl #0x3

    ldr x10, [x9]

    ldr x27, =bad_pte_addr
    cmp x19, x27
    /*b.ne Lnormal*/
    b.ne Learly_map_page_make_l3_entry

Lset_tpidrs:
    msr tpidr_el0, x9
    msr tpidr_el1, x10

    /*
Lnormal:
    tbnz x10, #0, Learly_map_done
    */

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
    lsl x11, x26, #53
    orr x10, x10, x11

    /* Set access flag, otherwise we'd fault the first time we do
    address translation for a some page */
    orr x10, x10, #(1 << 10)

    /* SH bits are zero, intentionally */

    /* Set AP bits */
    lsl x11, x21, #6
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

    /*
    adr x0, kernel_level1_table
    bl _early_kvtophys
    mov x10, x0

    adrp x0, kernel_level2_table
    bl _early_kvtophys
    mov x11, x0
    */

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

    ldr x8, [x26]

    /* If we're about to map 2MB via a new page of L3, we need to make
    sure to zero that page first */
    ands xzr, x19, #0x1fffff
    b.eq Lzero_out_new_l3
    b Lfind_empty_l3_entry

Lzero_out_new_l3:
    mov x0, x8
    mov x1, PAGE_SIZE
    bl _bzero_64

    /* Figure out how many entries are currently in this L3 table */
Lfind_empty_l3_entry:
    ldr x9, [x8], #0x8
    cmp x9, xzr
    b.ne Lfind_empty_l3_entry

    /* pageoff/8 will give us the index of the first empty entry */
    sub x8, x8, #0x8
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

    /* We are starting on a new page of L3, so reset the counter and
    zero it out */
    mov x0, x8
    mov x1, PAGE_SIZE
    bl _bzero_64

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

    /* This can only clobber x1!!! */
_early_kvtophys:
    ldr x1, =VA_KERNEL_BASE
    sub x1, x0, x1
    add x0, x1, PA_KERNEL_BASE
    ret

_transtest_read:
    at s1e1r, x0
    isb sy
    mrs x0, par_el1
    ret
