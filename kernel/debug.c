#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>

#include <libc/string.h>
#include <uart.h>
#include <vm/vm_constants.h>
#include <vm/vm_prot.h>
#include <vm/vm.h>

#include "asm.h"

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

void hexdump(void *, size_t);

void dump_kva_space(void){
    uint64_t static_pte_start = 0xFFFFFF800000F000;
    /* uint64_t static_pte_end = static_pte_start + 0x201000; */
    uint64_t static_pte_end = 0xffffff8000209000;

    /* while(static_pte_start < static_pte_end){ */
    /*     uint64_t phys = kvtophys(static_pte_start); */
    /*     uart_printf("%s: %#llx: phys=%#llx\r\n", __func__, static_pte_start, */
    /*             phys); */
    /*     static_pte_start += PAGE_SIZE; */
    /* } */
    /* return; */

    uint64_t l1_tbl = linear_phystokv(read_ttbr1());
    /* uart_printf("%s: ttbr1: %#llx\r\n", __func__, read_ttbr1()); */
    /* uart_printf("%s: l1 table kva %#llx\r\n", __func__, l1_tbl); */
    /* hexdump((void*)l1_tbl, PAGE_SIZE); */
    /* hexdump((void*)(MIN_KERNEL_VA+0x80000), 0x100); */
    /* return; */
    uint64_t curva = MIN_KERNEL_VA;

    /* Each L1 table entry handles 1GB */
    while(curva != (MAX_KERNEL_VA + 1)){
        uint64_t l1_idx = (curva >> ARM_4K_L1_SHIFT) & ARM_4K_INDEX_MASK;
        uint64_t *l1_ttep = (uint64_t *)(l1_tbl + (l1_idx * 0x8));

        uart_printf("[%#llx-%#llx): ", curva, curva + 0x40000000);

        if(*l1_ttep == ARM_TTE_EMPTY){
            uart_printf("1GB unmapped (empty L1 entry)\r\n");
            curva += 0x40000000;
            continue;
        }

        /* Each L2 entry handles 2MB */
        uint64_t curva_l2_tbl = linear_phystokv(*l1_ttep & ARM_4K_TABLE_MASK);
        /* uart_printf("%s: cur l2 table %#llx\r\n", __func__, curva_l2_tbl); */
        /* hexdump((void*)curva_l2_tbl, PAGE_SIZE); */
        uint64_t l2_end = curva + 0x40000000;

        uart_printf("\r\n");

        while(curva < l2_end){
            uart_printf("\t[%#llx-%#llx): ", curva, curva + 0x200000);

            uint64_t l2_idx = (curva >> ARM_4K_L2_SHIFT) & ARM_4K_INDEX_MASK;
            uint64_t *l2_ttep = (uint64_t *)(curva_l2_tbl + (l2_idx * 0x8));

            if(*l2_ttep == ARM_TTE_EMPTY){
                uart_printf("2MB unmapped (empty L2 entry)\r\n");
                curva += 0x200000;
                continue;
            }

            uint32_t l2_type = (*l2_ttep & ARM_TTE_TYPE_MASK) >> ARM_TTE_TYPE_SHIFT;

            if(l2_type == ARM_TTE_TYPE_BLOCK){
                char perms[8];
                apstr(*l2_ttep, perms);

                uint64_t physpage = *l2_ttep & ARM_TTE_BLOCK_OA_MASK;

                uart_printf("tte=%#llx phys=%#llx perms=%s\r\n",
                        *l2_ttep, physpage, perms);

                curva += 0x200000;
                continue;
            }

            /* uart_printf("%s: l3 tbl phys %#llx\r\n", __func__, */
            /*         *l2_ttep & ARM_4K_TABLE_MASK); */
            uint64_t curva_l3_tbl = linear_phystokv(*l2_ttep & ARM_4K_TABLE_MASK);
            /* curva_l3_tbl = 0xFFFFFF8000011000LL; */
            /* curva_l3_tbl += 0x80000; */
            /* uart_printf("%s: cur l3 table %#llx\r\n", __func__, curva_l3_tbl); */
            /* hexdump((void *)curva_l3_tbl, PAGE_SIZE); */
            uint64_t l3_end = curva + 0x200000;

            /* return; */

            uart_printf("\r\n");

            while(curva < l3_end){
                uart_printf("\t\t[%#llx-%#llx): ", curva, curva + PAGE_SIZE);

                uint64_t l3_idx = (curva >> ARM_4K_L3_SHIFT) & ARM_4K_INDEX_MASK;
                uint64_t *l3_ptep = (uint64_t *)(curva_l3_tbl + (l3_idx * 0x8));

                if(*l3_ptep == ARM_TTE_EMPTY){
                    uart_printf("4K unmapped (empty L3 entry)\r\n");
                    curva += PAGE_SIZE;
                    continue;
                }

                char perms[8];
                apstr(*l3_ptep, perms);

                uint64_t physpage = *l3_ptep & ARM_PTE_OA_MASK;

                uart_printf("pte=%#llx phys=%#llx perms=%s\r\n",
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
            uart_printf("%#llx: ", curaddr);
            curaddr += 0x10;
            putloc = 1;
        }

        uart_printf("%2x ", ((unsigned char*)data)[i]);
        if (((unsigned char*)data)[i] >= ' ' && ((unsigned char*)data)[i] <= '~') {
            ascii[i % 16] = ((unsigned char*)data)[i];
        } else {
            ascii[i % 16] = '.';
        }
        if ((i+1) % 8 == 0 || i+1 == size) {
            uart_printf(" ");
            if ((i+1) % 16 == 0) {
                uart_printf("|  %s \r\n", ascii);
                putloc = 0;
            } else if (i+1 == size) {
                ascii[(i+1) % 16] = '\0';
                if ((i+1) % 16 <= 8) {
                    uart_printf(" ");
                }
                for (j = (i+1) % 16; j < 16; ++j) {
                    uart_printf("   ");
                }
                uart_printf("|  %s \r\n", ascii);
                putloc = 0;
            }
        }
    }
}
