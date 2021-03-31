# moss

moss is a bare-metal arm64 OS for the Raspberry Pi 4 model B.

1. Get an arm64 toolchain: `sudo apt-get install gcc-aarch64-linux-gnu`
2. Add to config.txt:

```
# Probably remove this so we can have contiguous mmio
arm_peri_high=1

# VC SDRAM - give it 256 MB. It will be mapped from 0x40000000 downward
gpu_mem=256

# armv8 kernel
arm_64bit=1

# load kernel at 0x0
kernel_address=0

enable_uart=1
```

### Resources Used
#### UART
https://s-matyukevich.github.io/raspberry-pi-os/docs/lesson01/rpi-os.html
https://isometimes.github.io/rpi4-osdev/part3-helloworld/
