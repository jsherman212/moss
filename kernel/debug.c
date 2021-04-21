#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>

#include <asm.h>
#include <debug.h>
#include <libc/string.h>
#include <uart.h>
#include <vm/vm.h>
#include <vm/vm_constants.h>
#include <vm/vm_prot.h>

void dump_bootargs(struct bootargs *args){
    MOSSDBG("bootargs:\r\n"
            "\tdtb32:           0x%-16.16llx\r\n"
            "\tKernel base:     0x%-16.16llx\r\n"
            "\tKernel slide:    0x%-16.16llx\r\n"
            "\tStart of L3:     0x%-16.16llx\r\n"
            "\tFirst free phys: 0x%-16.16llx\r\n",
            args->dtb32, args->kernel_base, args->kernel_slide,
            args->start_of_L3, args->freephys);
}

static void apstr(uint64_t pte, char *s){
    uint32_t ap = (pte & ARM_PTE_AP_MASK) >> ARM_PTE_AP_SHIFT;

    strcpy(s, "---/---");

    /* First take care of execute bits */
    if((ap & ~1) == ARM_PTE_AP_RONA){
        s[0] = 'r';

        if(ap & 1)
            s[4] = 'r';
    }
    else if((ap & ~1) == ARM_PTE_AP_RWNA){
        s[0] = 'r';
        s[1] = 'w';

        if(ap & 1){
            s[4] = 'r';
            s[5] = 'w';
        }
    }

    if(!(pte & ARM_PTE_PXN_MASK))
        s[2] = 'x';

    if(!(pte & ARM_PTE_UXN_MASK))
        s[6] = 'x';
}

void dump_kva_space(void){
    uint64_t l1_tbl = linear_phystokv(read_ttbr1());
    uint64_t curva = MIN_KERNEL_VA;

    /* Each L1 table entry handles 1GB */
    while(curva != (MAX_KERNEL_VA + 1)){
        uint64_t l1_idx = (curva >> ARM_4K_L1_SHIFT) & ARM_4K_INDEX_MASK;
        uint64_t *l1_ttep = (uint64_t *)(l1_tbl + (l1_idx * 0x8));

        MOSSDBG("[%#llx-%#llx): ", curva, curva + 0x40000000);

        if(*l1_ttep == ARM_TTE_EMPTY){
            MOSSDBG("1GB unmapped (empty L1 entry)\r\n");
            curva += 0x40000000;
            continue;
        }

        /* Each L2 entry handles 2MB */
        uint64_t curva_l2_tbl = linear_phystokv(*l1_ttep & ARM_4K_TABLE_MASK);
        uint64_t l2_end = curva + 0x40000000;

        MOSSDBG("\r\n");

        while(curva < l2_end){
            MOSSDBG("\t[%#llx-%#llx): ", curva, curva + 0x200000);

            uint64_t l2_idx = (curva >> ARM_4K_L2_SHIFT) & ARM_4K_INDEX_MASK;
            uint64_t *l2_ttep = (uint64_t *)(curva_l2_tbl + (l2_idx * 0x8));

            if(*l2_ttep == ARM_TTE_EMPTY){
                MOSSDBG("2MB unmapped (empty L2 entry)\r\n");
                curva += 0x200000;
                continue;
            }

            uint32_t l2_type = (*l2_ttep & ARM_TTE_TYPE_MASK) >> ARM_TTE_TYPE_SHIFT;

            if(l2_type == ARM_TTE_TYPE_BLOCK){
                char perms[8];
                apstr(*l2_ttep, perms);

                uint64_t physpage = *l2_ttep & ARM_TTE_BLOCK_OA_MASK;

                MOSSDBG("tte=%#llx phys=%#llx perms=%s\r\n",
                        *l2_ttep, physpage, perms);

                curva += 0x200000;
                continue;
            }

            uint64_t curva_l3_tbl = linear_phystokv(*l2_ttep & ARM_4K_TABLE_MASK);
            uint64_t l3_end = curva + 0x200000;

            MOSSDBG("\r\n");

            while(curva < l3_end){
                MOSSDBG("\t\t[%#llx-%#llx): ", curva, curva + PAGE_SIZE);

                uint64_t l3_idx = (curva >> ARM_4K_L3_SHIFT) & ARM_4K_INDEX_MASK;
                uint64_t *l3_ptep = (uint64_t *)(curva_l3_tbl + (l3_idx * 0x8));

                if(*l3_ptep == ARM_TTE_EMPTY){
                    MOSSDBG("4K unmapped (empty L3 entry)\r\n");
                    curva += PAGE_SIZE;
                    continue;
                }

                char perms[8];
                apstr(*l3_ptep, perms);

                uint64_t physpage = *l3_ptep & ARM_PTE_OA_MASK;

                MOSSDBG("pte=%#llx phys=%#llx perms=%s\r\n",
                        *l3_ptep, physpage, perms);

                curva += PAGE_SIZE;
            }

            /* Round up to next 2MB boundry if not already a multiple
             * of 2MB */
            if(curva & 0x1fffff)
                curva = (curva + 0x200000) & ~(0x200000 - 1);
        }
    }
}

void hexdump(void *data, size_t size){
    char ascii[17];
    size_t i, j;
    ascii[16] = '\0';
    int putloc = 0;
    uint64_t curaddr = (uint64_t)data;
    for (i = 0; i < size; ++i) {
        if(!putloc){
            MOSSDBG("%#llx: ", curaddr);
            curaddr += 0x10;
            putloc = 1;
        }

        MOSSDBG("%2x ", ((unsigned char*)data)[i]);
        if (((unsigned char*)data)[i] >= ' ' && ((unsigned char*)data)[i] <= '~') {
            ascii[i % 16] = ((unsigned char*)data)[i];
        } else {
            ascii[i % 16] = '.';
        }
        if ((i+1) % 8 == 0 || i+1 == size) {
            MOSSDBG(" ");
            if ((i+1) % 16 == 0) {
                MOSSDBG("|  %s \r\n", ascii);
                putloc = 0;
            } else if (i+1 == size) {
                ascii[(i+1) % 16] = '\0';
                if ((i+1) % 16 <= 8) {
                    MOSSDBG(" ");
                }
                for (j = (i+1) % 16; j < 16; ++j) {
                    MOSSDBG("   ");
                }
                MOSSDBG("|  %s \r\n", ascii);
                putloc = 0;
            }
        }
    }
}
