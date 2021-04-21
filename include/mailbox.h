#ifndef MAILBOX
#define MAILBOX

#include <stdbool.h>
#include <stdint.h>

void mbxinit(void);

/* Copy out the contents of the mailbox buffer to the first param */
void mbxread(uint32_t *);

/* Send a mailbox message through the first parameter to the
 * channel specified by the second parameter, with the response
 * to be received through the third parameter */
bool mbxsend(uint32_t *, uint32_t *, uint8_t);

/* Write to the contents of the internal mailbox buffer, but
 * do not send */
void mbxwrite(uint32_t *, size_t);

#define MBX_PROCESS_REQUEST                                     (0x0)
#define MBX_REQUEST_SUCCESSFUL                                  (0x80000000)
#define MBX_REQUEST_ERROR                                       (0x80000001)

#define MBX_FULL                                                (0x80000000)
#define MBX_EMPTY                                               (0x40000000)

enum {
    MBX_CHANNEL_PROP =                                          0x8,
};

#define MBX_TAG_Last                                            (0x0)

enum {
    MBX_TAG_VideoCore_Get_Firmware_Revision =                   0x1,
    MBX_TAG_VideoCore_Get_Board_Model =                         0x10001,
    MBX_TAG_VideoCore_Get_Board_Revision,
    MBX_TAG_VideoCore_Get_Board_MAC_Address,
    MBX_TAG_VideoCore_Get_Board_Serial,
    MBX_TAG_VideoCore_Get_ARM_Memory,
    MBX_TAG_VideoCore_Get_VC_Memory,
    MBX_TAG_VideoCore_Get_Clocks,
};

enum {
    MBX_TAG_Framebuffer_Get =                                   0x40001,
    MBX_TAG_Framebuffer_Get_Pitch =                             0x40008,
    MBX_TAG_Framebuffer_Set_Phys_Width_Height =                 0x48003,
    MBX_TAG_Framebuffer_Set_Virt_Width_Height,
    MBX_TAG_Framebuffer_Set_Depth,
    MBX_TAG_Framebuffer_Set_Pixel_Order,
    MBX_TAG_Framebuffer_Set_Virt_Offset =                       0x48009,
};

#endif
