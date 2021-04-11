#ifndef VM_CONSTANTS
#define VM_CONSTANTS

#define KERNEL_BASE_PA              (0x80000)

#define MIN_KERNEL_VA               (0xffffff8000000000)
#define MAX_KERNEL_VA               (0xffffffffffffffff)

#define PAGE_SIZE                   (0x1000)

#define ARM_4K_L1_SHIFT             (30)
#define ARM_4K_L2_SHIFT             (21)
#define ARM_4K_L3_SHIFT             (12)

#define ARM_4K_INDEX_MASK           (0x1ff)
#define ARM_4K_TABLE_MASK           (0xfffffffff000)

#define ARM_TTE_EMPTY               (0)

#define ARM_TTE_TYPE_BLOCK          (0)
#define ARM_TTE_TYPE_ENTRY          (1)

#define ARM_TTE_TYPE_SHIFT          (1)
#define ARM_TTE_TYPE_MASK           (1uLL << ARM_TTE_TYPE_SHIFT)

#define ARM_TTE_BLOCK_OA_MASK       (0xffffffe00000uLL)
#define ARM_PTE_OA_MASK             (0x7ffffffff000uLL)

#define ARM_PTE_UXN_SHIFT           (54)
#define ARM_PTE_UXN_MASK            (1uLL << ARM_PTE_UXN_SHIFT)

#define ARM_PTE_PXN_SHIFT           (53)
#define ARM_PTE_PXN_MASK            (1uLL << ARM_PTE_PXN_SHIFT)

#define ARM_PTE_AP_SHIFT            (6)
#define ARM_PTE_AP_MASK             (0xc0uLL)

#define ARM_PTE_AP_RWNA             (0x0) /* priv=read-write, user=no-access */
#define ARM_PTE_AP_RWRW             (0x1) /* priv=read-write, user=read-write */
#define ARM_PTE_AP_RONA             (0x2) /* priv=read-only, user=no-access */
#define ARM_PTE_AP_RORO             (0x3) /* priv=read-only, user=read-only */

#endif
