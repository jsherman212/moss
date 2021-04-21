#include <stdbool.h>
#include <stdint.h>

#include <caches.h>
#include <debug.h>
#include <locks/spinlock.h>
#include <mailbox.h>
#include <panic.h>
#include <vm/mmu.h>
#include <vm/vm.h>

static splck_t fb_spinlock = SPLCK_INITIALIZER;

static volatile uint32_t *g_framebuffer = NULL;

static uint32_t width = 0;
static uint32_t height = 0;
static uint32_t pitch = 0;
static uint32_t size = 0;
static bool rgb = false;

void fb_init(void){
    MOSSDBG("%s: starting\r\n", __func__);

    uint32_t msg[35];

    msg[0] = sizeof(msg);
    msg[1] = MBX_PROCESS_REQUEST;

    msg[2] = MBX_TAG_Framebuffer_Set_Phys_Width_Height;
    msg[3] = msg[4] = 8;
    msg[5] = 1920;
    msg[6] = 1080;

    msg[7] = MBX_TAG_Framebuffer_Set_Virt_Width_Height;
    msg[8] = msg[9] = 8;
    msg[10] = 1920;
    msg[11] = 1080;

    msg[12] = MBX_TAG_Framebuffer_Set_Virt_Offset;
    msg[13] = msg[14] = 8;
    msg[15] = msg[16] = 0;

    msg[17] = MBX_TAG_Framebuffer_Set_Depth;
    msg[18] = msg[19] = 4;
    /* Bits per pixel */
    msg[20] = 32;

    msg[21] = MBX_TAG_Framebuffer_Set_Pixel_Order;
    msg[22] = msg[23] = 4;
    /* RGB */
    msg[24] = 1;

    msg[25] = MBX_TAG_Framebuffer_Get;
    msg[26] = msg[27] = 8;
    /* Ask for a 2MB-aligned address so we can map this using 2MB L2
     * block entries */
    msg[28] = 0x200000;
    msg[29] = 0;

    msg[30] = MBX_TAG_Framebuffer_Get_Pitch;
    msg[31] = msg[32] = 4;
    msg[33] = 0;

    msg[34] = MBX_TAG_Last;

    uint32_t reply[sizeof(msg)/sizeof(uint32_t)];

    if(!mbxsend(msg, reply, MBX_CHANNEL_PROP))
        panic("%s: framebuffer init failed", __func__);

    uint32_t fb_phys = reply[28] & 0x3fffffff;

    MOSSDBG("%s: framebuffer lives at physical address %#x\r\n",
            __func__, fb_phys);

    width = reply[10];
    height = reply[11];
    pitch = reply[33];
    rgb = reply[24];
    size = reply[29];

    /* XXX: hardcoded for now, since I have no memory manager,
     * will change once I get mem manager set up */
    g_framebuffer = (volatile uint32_t *)0xffffffb000000000;

    if(!map_range_block((uint64_t)g_framebuffer, fb_phys, size,
                VM_PROT_READ | VM_PROT_WRITE, true)){
        panic("%s: failed to map framebuffer", __func__);
    }

    MOSSDBG("%s: mapped framebuffer at %#llx\r\n"
            "\twidth:       %#x\r\n"
            "\theight:      %#x\r\n"
            "\tpitch:       %#x\r\n"
            "\tsize:        %#x\r\n", __func__, (uint64_t)g_framebuffer,
            width, height, pitch, size);

    /* XXX make screen green */
    for(uint32_t i=0; i<size/sizeof(uint32_t); i++){
        g_framebuffer[i] = 0x00ff00;
    }
}
