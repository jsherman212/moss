#ifndef EXCEPTION
#define EXCEPTION

/* Exception Classes */
#define ESR_EC_UNKNOWN                          (0x0)
#define ESR_EC_TRAPPED_WF                       (0x1)
#define ESR_EC_TRAPPED_SVE_SIMD_FP              (0x7)
#define ESR_EC_ILLEGAL_EXECUTION_STATE          (0xe)
#define ESR_EC_SVC                              (0x15)
#define ESR_EC_TRAPPED_MSR_MRS_SYS              (0x18)
#define ESR_EC_INSTR_FETCH_ABORT_LOWER_EL       (0x20)
#define ESR_EC_INSTR_FETCH_ABORT_SAME_EL        (0x21)
#define ESR_EC_UNALIGNED_PC                     (0x22)
#define ESR_EC_DATA_ABORT_LOWER_EL              (0x24)
#define ESR_EC_DATA_ABORT_SAME_EL               (0x25)
#define ESR_EC_UNALIGNED_SP                     (0x26)
#define ESR_EC_TRAPPED_FP                       (0x2c)
#define ESR_EC_SERROR                           (0x2f)
#define ESR_EC_BP_LOWER_EL                      (0x30)
#define ESR_EC_BP_SAME_EL                       (0x31)
#define ESR_EC_SS_LOWER_EL                      (0x32)
#define ESR_EC_SS_SAME_EL                       (0x33)
#define ESR_EC_WP_LOWER_EL                      (0x34)
#define ESR_EC_WP_SAME_EL                       (0x35)
#define ESR_EC_BRK                              (0x3c)

/* ISS encoding for ESR_EC_TRAPPED_WF */
#define WFI                                     (0x0)
#define WFE                                     (0x1)

/* ISS encoding for instruction fetch aborts/data aborts */
#define FSC_MASK                               (0x3f)

#define FSC_ADDRESS_SIZE_FAULT_L0              (0x0)
#define FSC_ADDRESS_SIZE_FAULT_L1              (0x1)
#define FSC_ADDRESS_SIZE_FAULT_L2              (0x2)
#define FSC_ADDRESS_SIZE_FAULT_L3              (0x3)
#define FSC_TRANSLATION_FAULT_L0               (0x4)
#define FSC_TRANSLATION_FAULT_L1               (0x5)
#define FSC_TRANSLATION_FAULT_L2               (0x6)
#define FSC_TRANSLATION_FAULT_L3               (0x7)
#define FSC_ACCESS_FLAG_FAULT_L1               (0x9)
#define FSC_ACCESS_FLAG_FAULT_L2               (0xa)
#define FSC_ACCESS_FLAG_FAULT_L3               (0xb)
#define FSC_PERMISSION_FAULT_L1                (0xd)
#define FSC_PERMISSION_FAULT_L2                (0xe)
#define FSC_PERMISSION_FAULT_L3                (0xf)
#define FSC_SYNC_EXTERNAL_ABORT                (0x10)
#define FSC_SYNC_EXTERNAL_ABORT_L0             (0x14)
#define FSC_SYNC_EXTERNAL_ABORT_L1             (0x15)
#define FSC_SYNC_EXTERNAL_ABORT_L2             (0x16)
#define FSC_SYNC_EXTERNAL_ABORT_L3             (0x17)
#define FSC_SYNC_PARITY_OR_ECC                 (0x18)
#define FSC_SYNC_PARITY_OR_ECC_L0              (0x1c)
#define FSC_SYNC_PARITY_OR_ECC_L1              (0x1d)
#define FSC_SYNC_PARITY_OR_ECC_L2              (0x1e)
#define FSC_SYNC_PARITY_OR_ECC_L3              (0x1f)
/* Alignment fault specific to data aborts */
#define FSC_ALIGNMENT_FAULT                    (0x21)
#define FSC_TLB_CONFLICT_ABORT                 (0x30)

#endif
