#ifndef MMIO
#define MMIO

#include <stdint.h>

/* Currently for low peripheral mode */

#define MMIO_VA_BASE                (0xffffff80fc000000)

#define GPIO_BASE                   (MMIO_VA_BASE + 0x2200000)

#define rGPFSEL0                    *(volatile uint32_t *)(GPIO_BASE)
#define rGPFSEL1                    *(volatile uint32_t *)(GPIO_BASE + 0x4)
#define rGPFSEL2                    *(volatile uint32_t *)(GPIO_BASE + 0x8)
#define rGPFSEL3                    *(volatile uint32_t *)(GPIO_BASE + 0xc)
#define rGPFSEL4                    *(volatile uint32_t *)(GPIO_BASE + 0x10)
#define rGPFSEL5                    *(volatile uint32_t *)(GPIO_BASE + 0x14)

#define rGPSET0                     *(volatile uint32_t *)(GPIO_BASE + 0x1c)
#define rGPSET1                     *(volatile uint32_t *)(GPIO_BASE + 0x20)

#define rGPCLR0                     *(volatile uint32_t *)(GPIO_BASE + 0x28)
#define rGPCLR1                     *(volatile uint32_t *)(GPIO_BASE + 0x2c)

#define rGPIO_PUP_PDN_CNTRL_REG0    *(volatile uint32_t *)(GPIO_BASE + 0xe4)

#define AUX_BASE                    (MMIO_VA_BASE + 0x2215000)

#define rAUX_IRQ                    *(volatile uint32_t *)(AUX_BASE)
#define rAUX_ENABLES                *(volatile uint32_t *)(AUX_BASE + 0x4)
#define rAUX_MU_IO_REG              *(volatile uint32_t *)(AUX_BASE + 0x40)
#define rAUX_MU_IER_REG             *(volatile uint32_t *)(AUX_BASE + 0x44)
#define rAUX_MU_IIR_REG             *(volatile uint32_t *)(AUX_BASE + 0x48)
#define rAUX_MU_LCR_REG             *(volatile uint32_t *)(AUX_BASE + 0x4c)
#define rAUX_MU_MCR_REG             *(volatile uint32_t *)(AUX_BASE + 0x50)
#define rAUX_MU_LSR_REG             *(volatile uint32_t *)(AUX_BASE + 0x54)
#define rAUX_MU_CNTL_REG            *(volatile uint32_t *)(AUX_BASE + 0x60)
#define rAUX_MU_BAUD_REG            *(volatile uint32_t *)(AUX_BASE + 0x68)

/* https://github.com/raspberrypi/documentation/blob/JamesH65-mailbox_docs/configuration/mailboxes/accessing.md */
#define MBX_BASE                    (MMIO_VA_BASE + 0x200b880)

#define rMBX_READ                   *(volatile uint32_t *)(MBX_BASE)
#define rMBX_STATUS                 *(volatile uint32_t *)(MBX_BASE + 0x18)
#define rMBX_WRITE                  *(volatile uint32_t *)(MBX_BASE + 0x20)
#define rMBX_STATUS1                *(volatile uint32_t *)(MBX_BASE + 0x38)

#endif
