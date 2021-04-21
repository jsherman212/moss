#include <stdint.h>

#include <caches.h>
#include <debug.h>
#include <libc/string.h>
#include <mailbox.h>
#include <mmio.h>
#include <panic.h>
#include <vm/vm.h>
#include <vm/vm_constants.h>

extern uint64_t mailbox_page[];

static volatile uint32_t *mbx = (volatile uint32_t *)mailbox_page;
static const size_t mbxlen = PAGE_SIZE;

void mbxread(uint32_t *buf){
    if(!buf)
        return;

    memcpy(buf, (const void *)mbx, mbxlen);
}

bool mbxsend(uint32_t *msg, uint32_t *reply, uint8_t channel){
    uint32_t msgsz = msg[0];

    if(msgsz > mbxlen)
        return false;

    memcpy((void *)mbx, msg, msgsz);

    uint32_t desc = kvtophys((uint64_t)mbx) | channel;

    /* Wait until we can write */
    while(rMBX_STATUS & MBX_FULL);

    rMBX_WRITE = desc;

    for(;;){
        /* Wait for a reply */
        while(rMBX_STATUS & MBX_EMPTY);

        if(rMBX_READ == desc){
            if(mbx[1] != MBX_REQUEST_SUCCESSFUL)
                return false;

            memcpy(reply, (const void *)mbx, msgsz);

            return true;
        }
    }
}

void mbxwrite(uint32_t *buf, size_t len){
    if(len > mbxlen)
        return;

    memcpy((void *)mbx, buf, len);
}
