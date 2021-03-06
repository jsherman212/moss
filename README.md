# moss

moss is a bare-metal arm64 OS for the Raspberry Pi 4 model B.

1. Get an arm64 toolchain: `sudo apt-get install gcc-aarch64-linux-gnu`
2. Add to config.txt:

```
# VC SDRAM - give it 256 MB. It will be mapped from 0x40000000 downward
gpu_mem=256

# armv8 kernel
arm_64bit=1

enable_uart=1

core_freq_min=500

hdmi_group=1

hdmi_mode=16
```

### Features
- UART
- Kernel virtual address space mapped from `0xffffff8000000000` upwards
    - MMIO mapped from `0xffffff80fc000000`
- Framebuffer
- Lock types:
    - simple spinlock

### TODO
- Kernel ASLR
- Multicore
- Userspace

### Resources Used
#### UART
https://s-matyukevich.github.io/raspberry-pi-os/docs/lesson01/rpi-os.html

https://isometimes.github.io/rpi4-osdev/part3-helloworld/

#### Framebuffer
https://github.com/raspberrypi/firmware/wiki/Mailbox-property-interface

https://isometimes.github.io/rpi4-osdev/part5-framebuffer/

http://www.abbeycatuk.info/2017/03/06/the-raspberry-pis-videocore-iv/

https://github.com/raspberrypi/documentation/blob/JamesH65-mailbox_docs/configuration/mailboxes/accessing.md
