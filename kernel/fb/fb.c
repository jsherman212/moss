#include <stdbool.h>
#include <stdint.h>

/* #include <caches.h> */
#include <debug.h>
#include <doprnt.h>
#include <fb/font8x8_basic.h>
#include <locks/spinlock.h>
#include <mailbox.h>
#include <panic.h>
#include <uart.h>
#include <vm/mmu.h>
#include <vm/vm.h>

#define FB_WIDTH 1920
#define FB_HEIGHT 1080

#define FONT_HEIGHT 8
#define FONT_WIDTH 8

/* Array of line endings that are indexed by row. This exists
 * so I put the cursor on the right place when the user backspaces
 * to the previous row */
static int32_t g_fb_lineendings[FB_HEIGHT / FONT_HEIGHT];

static splck_t fb_spinlock = SPLCK_INITIALIZER;

static volatile uint32_t *g_framebuffer = NULL;

static uint32_t width = 0;
static uint32_t height = 0;
static uint32_t pitch = 0;
static uint32_t size = 0;
static bool rgb = false;

void fb_init(void){
    FBDBG("%s: starting\r\n", __func__);

    uint32_t msg[35];

    msg[0] = sizeof(msg);
    msg[1] = MBX_PROCESS_REQUEST;

    msg[2] = MBX_TAG_Framebuffer_Set_Phys_Width_Height;
    msg[3] = msg[4] = 8;
    msg[5] = FB_WIDTH;
    msg[6] = FB_HEIGHT;

    msg[7] = MBX_TAG_Framebuffer_Set_Virt_Width_Height;
    msg[8] = msg[9] = 8;
    msg[10] = FB_WIDTH;
    msg[11] = FB_HEIGHT;

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

    FBDBG("%s: framebuffer lives at physical address %#x\r\n",
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

    FBDBG("%s: mapped framebuffer at %#llx\r\n"
            "\twidth:       %d\r\n"
            "\theight:      %d\r\n"
            "\tpitch:       %d\r\n"
            "\tsize:        %d\r\n", __func__, (uint64_t)g_framebuffer,
            width, height, pitch, size);

    /* XXX make screen green */
    for(uint32_t i=0; i<size/sizeof(uint32_t); i++){
        g_framebuffer[i] = 0x00ff00;
    }

    for(uint32_t i=0; i<sizeof(g_fb_lineendings)/sizeof(uint32_t); i++)
        g_fb_lineendings[i] = 0;
}

static int32_t gx = 0;
static int32_t gy = 0;

static void clear_current_row(void){
    volatile uint8_t *fb = (volatile uint8_t *)g_framebuffer;

    for(int i=0; i<FB_WIDTH/FONT_WIDTH; i++){
        uint32_t offsetx = i * sizeof(uint32_t);
        uint32_t offsety = (gy + i) * pitch;

        *((volatile uint32_t *)(fb + offsetx + offsety)) = 0x0;
    }
}

/* TODO: scrolling, making sure we aren't at the end of a row, etc */
/* We are really just drawing individual pixels */
static void fb_putc_internal(char c){
    char *chp = font8x8_basic[c];
    volatile uint8_t *fb = (volatile uint8_t *)g_framebuffer;

    for(int i=0; i<FONT_HEIGHT; i++){
        for(int k=0; k<FONT_WIDTH; k++){
            /* Choose black or white for the current pixel */
            uint32_t color = (chp[i] & (1 << k)) ? 0xffffffff : 0x0;

            uint32_t offsetx = (gx + k) * sizeof(uint32_t);
            uint32_t offsety = (gy + i) * pitch;

            *((volatile uint32_t *)(fb + offsetx + offsety)) = color;
        }
    }
}

void fb_putc(char c){
    FBDBG("%s: gx %d gy %d putc '%c' %#x\r\n", __func__, gx, gy, c, c);

    /* TODO: scrolling, making sure we aren't at the end of a row, etc */
    switch(c){
        case '\n':
            g_fb_lineendings[gy / FONT_HEIGHT] = gx - FONT_WIDTH;
            FBDBG("%s: newline: set row %d's line end to %d\r\n", __func__,
                    gy / FONT_HEIGHT, g_fb_lineendings[gy / FONT_HEIGHT]);
            gx = 0;
            gy += FONT_HEIGHT;
            break;
        case '\r':
            gx = 0;
            /* TODO: set line ending */
            break;
        case '\t':
            for(int i=0; i<4; i++)
                fb_putc_internal(' ');

            gx += FONT_WIDTH * 4;
            break;
        /* Backspace */
        case 0x7f:
            /* Go back one character or to the end of the row above */
            if(gx == 0 && gy > 0){
                gy -= FONT_HEIGHT;
                gx = g_fb_lineendings[gy / FONT_HEIGHT];
                FBDBG("%s: backspace: set gx to %d (row %d's line end)\r\n",
                        __func__, g_fb_lineendings[gy / FONT_HEIGHT],
                        gy / FONT_HEIGHT);
            }
            else if((gx - FONT_WIDTH) >= 0){
                gx -= FONT_WIDTH;
            }
            else{
                gx = 0;
            }

            fb_putc_internal(' ');
            break;
        default:
            fb_putc_internal(c);
            gx += FONT_WIDTH;
            break;
    };

    if(gx >= width){
        g_fb_lineendings[gy / FONT_HEIGHT] = gx - FONT_WIDTH;
        FBDBG("%s: new row: set row %d's line end to %d\r\n", __func__,
                gy / FONT_HEIGHT, g_fb_lineendings[gy / FONT_HEIGHT]);
        gx = 0;
        gy += FONT_HEIGHT;
    }

    /* Move current framebuffer contents one row up, freeing a bottom row */
    if(gy >= height){

        /* TODO: finish */
        clear_current_row();
        gx = 0;
        gy -= FONT_HEIGHT;
    }
}

static void fb_gets_internal(char *buf, size_t buflen, bool echo){
    char *bufstart = buf;
    char *bufend = buf + buflen;

    while(buf < bufend){
        char got = uart_getc_unlocked();

        /* Make sure deleted characters aren't included */
        if(got != 0x7f)
            *buf++ = got;
        else{
            if(buf > bufstart)
                buf--;

            *buf = '\0';
        }

        if(got != '\r' && got != '\n'){
            if(echo)
                fb_putc(got);
        }
        else{
            *buf = '\0';
            return;
        }
    }
}

void fb_gets_with_echo(char *buf, size_t buflen){
    fb_gets_internal(buf, buflen, true);
}

void fb_puts(const char *s){
    char *ss = (char *)s;

    while(*ss)
        fb_putc(*ss++);

    fb_putc('\n');
}

static void fb_printf_putc(char c, struct doprnt_info *di){
    di->written++;
    fb_putc(c);
}

int fb_printf(const char *fmt, ...){
    va_list args;
    va_start(args, fmt);

    struct doprnt_info di;
    di.buf = NULL;
    di.remaining = 99999999;
    di.written = 0;

    int w = doprnt(fmt, fb_printf_putc, &di, args);

    va_end(args);

    return w;
}
