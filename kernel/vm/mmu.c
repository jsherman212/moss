#include <stdbool.h>
#include <stdint.h>

#include <asm.h>
#include <caches.h>
#include <debug.h>
#include <kernel.h>
#include <libc/string.h>
#include <panic.h>
#include <vm/mmu.h>
#include <vm/vm.h>
#include <vm/vm_constants.h>
#include <vm/vm_prot.h>

/* All of level 1 and level 2 are mapped before jumping into the
 * kernel's _main from EL2 */
extern uint64_t kernel_level2_table[];

/* XXX: once I get a memory management thing going, once we run out of
 * space inside the L3 page we're left off on from EL2, I gotta get new
 * ones thru an api like alloc_ptpage or palloc or something that actually
 * keeps track of used virtual/physical memory */
static uint64_t next_free_phys_page = 0;

/* XXX: this will need to take in an EL param once userspace
 * exists */
static uint32_t vm_prot_to_tte_prot(vm_prot_t prot, uint8_t *xn_out){
    *xn_out = 1;

    uint32_t ap = ARM_PTE_AP_RONA;

    if(prot & VM_PROT_WRITE)
        ap = ARM_PTE_AP_RWNA;

    if(prot & VM_PROT_EXECUTE)
        *xn_out = 0;

    return ap;
}

static bool map_range_internal(uint64_t va, uint64_t pa, uint64_t sz,
        vm_prot_t prot, bool device, bool overwrite, bool blocks_allowed){
    /* XXX: this will not be here once I have a memory manager */
    if(next_free_phys_page == 0)
        next_free_phys_page = g_bootargsp->freephys;

    if(va & (PAGE_SIZE - 1))
        panic("%s: va %#llx is not page aligned", __func__, va);

    if(pa & (PAGE_SIZE - 1))
        panic("%s: pa %#llx is not page aligned", __func__, pa);

    if(sz & (PAGE_SIZE - 1))
        panic("%s: sz %#llx is not page aligned", __func__, sz);

    if(sz == 0)
        panic("%s: sz is zero");

    if(sz > (0x40000000uLL * 512))
        panic("%s: sz %#llx too large", __func__, sz);

    if(va < MIN_KERNEL_VA)
        panic("%s: va %#llx is not in TTBR1", __func__, va);

    /* TODO: when userspace is a thing, make sure the mapping is
     * actually for TTBR1 for this check */
    if((prot & VM_PROT_WRITE) && (prot & VM_PROT_EXECUTE))
        panic("%s: wx mappings are not allowed in TTBR1", __func__);

    if(!(prot & VM_PROT_READ))
        panic("%s: VM_PROT_READ is required", __func__);

    MOSSDBG("%s: va %#llx pa %#llx sz %#llx prot %#x device? %d"
            " overwrite? %d block mappings allowed? %d\r\n", __func__,
            va, pa, sz, prot, device, overwrite, blocks_allowed);

    /* Figure out if we can use some number of 2MB L2 block mappings
     * to save space. We can only do this if va and pa are both
     * 2MB aligned AND we are trying to map at least 2MB */
    uint64_t nblocks = 0;

    if(!(va & 0x1fffff) && !(pa & 0x1fffff) && sz >= 0x200000)
        nblocks = sz / 0x200000;

    MOSSDBG("%s: nblocks %lld\r\n", __func__, nblocks);

    uint64_t start = va;
    uint64_t end = start + sz;
    uint64_t curva = start;
    uint64_t curpa = pa;

    uint8_t xn_bit;
    uint32_t tte_prot = vm_prot_to_tte_prot(prot, &xn_bit);

    MOSSDBG("%s: TTE prot %d XN bit %d\r\n", __func__, tte_prot, xn_bit);

    /* Normal memory: 0, device memory: 1 */
    uint32_t attridx = device;

    /* XXX: will be whatever ttbr is passed in the future */
    uint64_t l1_tbl = linear_phystokv(read_ttbr1());
    
    while(curva < end){
        uint64_t l1_idx = (curva >> ARM_4K_L1_SHIFT) & ARM_4K_INDEX_MASK;
        uint64_t *l1_ttep = (uint64_t *)(l1_tbl + (l1_idx * 0x8));

        MOSSDBG("%s: curva %#llx: l1 idx %lld l1 ttep %#llx (%#llx)\r\n",
                __func__, curva, l1_idx, (uint64_t)l1_ttep, *l1_ttep);

        if(*l1_ttep == ARM_TTE_EMPTY){
            /* All of L2 is mapped, so it's easy to figure out the
             * address of the target L2 table */
            uint64_t l2_tbl_page = (l1_idx << PAGE_SHIFT) +
                (uintptr_t)kernel_level2_table;
            uint64_t l2_tbl_page_phys = kvtophys(l2_tbl_page);

            /* Set NSTable, type, and valid bits */
            *l1_ttep = l2_tbl_page_phys |
                (1uLL << ARM_TTE_NS_SHIFT) |
                (ARM_TTE_TYPE_ENTRY << ARM_TTE_TYPE_SHIFT) | ARM_TTE_VALID;

            dcache_clean_and_invalidate_PoC(l1_ttep, sizeof(*l1_ttep));

            asm volatile("dsb sy");
            asm volatile("isb sy");

            MOSSDBG("%s: created 1GB L1 entry for %#llx at %#llx:"
                    " %#llx\r\n", __func__, curva, (uint64_t)l1_ttep,
                    *l1_ttep);
        }

        uint64_t l2_tbl = linear_phystokv(*l1_ttep & ARM_4K_TABLE_MASK);
        uint64_t l2_idx = (curva >> ARM_4K_L2_SHIFT) & ARM_4K_INDEX_MASK;
        uint64_t *l2_ttep = (uint64_t *)(l2_tbl + (l2_idx * 0x8));

        MOSSDBG("%s: curva %#llx: l2 idx %lld l2 table %#llx l2 ttep %#llx"
                " (%#llx)\r\n", __func__, curva, l2_idx, l2_tbl,
                (uint64_t)l2_ttep, *l2_ttep);

        if(nblocks > 0 && blocks_allowed){
            if(*l2_ttep != ARM_TTE_EMPTY && !overwrite){
                panic("%s: a block mapping exists for %#llx at %#llx, but"
                        " overwriting isn't allowed (tte=%#llx)", __func__,
                        va, (uint64_t)l2_ttep, *l2_ttep);
            }

            /* XXX: check if these pages are for user or kernel
             * once userspace is a thing. But for now, we only
             * set PXN and always set UXN */
            *l2_ttep = curpa | (1uLL << ARM_PTE_UXN_SHIFT) |
                ((uint64_t)xn_bit << ARM_PTE_PXN_SHIFT) |
                (1uLL << ARM_PTE_AF_SHIFT) |
                (tte_prot << ARM_PTE_AP_SHIFT) |
                (attridx << ARM_PTE_ATTRIDX_SHIFT) |
                (ARM_TTE_TYPE_BLOCK << ARM_TTE_TYPE_SHIFT) |
                ARM_TTE_VALID;

            dcache_clean_and_invalidate_PoC(l2_ttep, sizeof(*l2_ttep));

            asm volatile("dsb sy");
            asm volatile("isb sy");

            MOSSDBG("%s: created 2MB L2 block mapping for %#llx at %#llx:"
                    " %#llx\r\n", __func__, curva, (uint64_t)l2_ttep,
                    *l2_ttep);

            nblocks--;
            curva += 0x200000;
            curpa += 0x200000;
            continue;
        }
        else if(*l2_ttep == ARM_TTE_EMPTY){
            /* XXX: will be palloc when I have a memory manager set
             * up instead of next_free_phys_page */

            /* Upper table attributes are res0 for level 2 */
            *l2_ttep = next_free_phys_page |
                (ARM_TTE_TYPE_ENTRY << ARM_TTE_TYPE_SHIFT) | ARM_TTE_VALID;

            dcache_clean_and_invalidate_PoC(l2_ttep, sizeof(*l2_ttep));

            asm volatile("dsb sy");
            asm volatile("isb sy");

            MOSSDBG("%s: created 2MB L2 entry for %#llx at %#llx:"
                    " %#llx\r\n", __func__, curva, (uint64_t)l2_ttep,
                    *l2_ttep);
        }

        uint64_t l3_tbl = linear_phystokv(*l2_ttep & ARM_4K_TABLE_MASK);

        MOSSDBG("%s: curva %#llx: l3 tbl %#llx\r\n", __func__, curva, l3_tbl);

        /* This could be the beginning of a new, unmapped level 3
         * page, so let's quickly test if it's here and map it if
         * it's not.
         *
         * XXX when I get a memory manager going next_free_phys_page
         * will be replaced with a call to palloc */
        if(at_s1e1r((uint64_t)l3_tbl) & 1){
            MOSSDBG("%s: unmapped L3 page at kv %#llx, mapping it\r\n",
                    __func__, l3_tbl);

            if(!map_range_internal(l3_tbl, next_free_phys_page, PAGE_SIZE,
                        VM_PROT_READ | VM_PROT_WRITE, false, false, false)){
                panic("%s: failed to map new page of L3", __func__);
            }

            MOSSDBG("%s: mapped new L3 page at %#llx\r\n", __func__, l3_tbl);

            bzero((void *)l3_tbl, PAGE_SIZE);

            next_free_phys_page += PAGE_SIZE;
        }

        uint64_t l3_idx = (curva >> ARM_4K_L3_SHIFT) & ARM_4K_INDEX_MASK;
        uint64_t *l3_ptep = (uint64_t *)(l3_tbl + (l3_idx * 0x8));

        MOSSDBG("%s: curva %#llx: l3 idx %lld l3 table %#llx l3 ptep %#llx"
                " (%#llx)\r\n", __func__, curva, l3_idx, l3_tbl,
                (uint64_t)l3_ptep, *l3_ptep);

        if(*l3_ptep != ARM_TTE_EMPTY && !overwrite){
            panic("%s: a page table entry exists for %#llx at %#llx, but"
                    " overwriting isn't allowed (pte=%#llx)", __func__, va,
                    (uint64_t)l3_ptep, *l3_ptep);
        }

        *l3_ptep = curpa | (1uLL << ARM_PTE_UXN_SHIFT) |
            ((uint64_t)xn_bit << ARM_PTE_PXN_SHIFT) |
            (1uLL << ARM_PTE_AF_SHIFT) |
            (tte_prot << ARM_PTE_AP_SHIFT) |
            (1uLL << ARM_PTE_NS_SHIFT) |
            (attridx << ARM_PTE_ATTRIDX_SHIFT) |
            (ARM_TTE_TYPE_ENTRY << ARM_TTE_TYPE_SHIFT) |
            ARM_TTE_VALID;

        dcache_clean_and_invalidate_PoC(l3_ptep, sizeof(*l3_ptep));

        asm volatile("dsb sy");
        asm volatile("isb sy");

        MOSSDBG("%s: created 4KB mapping for %#llx at %#llx:"
                " %#llx\r\n", __func__, curva, (uint64_t)l3_ptep,
                *l3_ptep);

        curva += PAGE_SIZE;
        curpa += PAGE_SIZE;
    }

    tlb_flush();

    return true;
}

/* TODO: when I get tasks set up these will take a task struct and
 * pass in its ttbr to map_range_internal */
bool map_range(uint64_t va, uint64_t pa, uint64_t sz, vm_prot_t prot,
        bool device){
    return map_range_internal(va, pa, sz, prot, device, false, false);
}

bool map_range_force(uint64_t va, uint64_t pa, uint64_t sz,
        vm_prot_t prot, bool device){
    return map_range_internal(va, pa, sz, prot, device, true, false);
}

bool map_range_block(uint64_t va, uint64_t pa, uint64_t sz,
        vm_prot_t prot, bool device){
    return map_range_internal(va, pa, sz, prot, device, false, true);
}

bool map_range_block_force(uint64_t va, uint64_t pa, uint64_t sz,
        vm_prot_t prot, bool device){
    return map_range_internal(va, pa, sz, prot, device, true, true);
}
