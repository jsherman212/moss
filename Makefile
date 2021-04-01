PREFIX = aarch64-linux-gnu

BUILD_DIR = build

# Please run make in the root of this project and not anywhere else
ROOT_DIR = $(shell pwd)

AS = $(PREFIX)-as
CC = $(PREFIX)-gcc
CFLAGS = -ffreestanding -nostdlib -nostartfiles -I$(ROOT_DIR)/include
LD = $(PREFIX)-ld
OBJCOPY = $(PREFIX)-objcopy

export AS
export CC
export CFLAGS

TARGET_DIRS = boot kernel

all : $(TARGET_DIRS) kernel8.img

.PHONY : target_dirs $(TARGET_DIRS)

$(TARGET_DIRS) :
	$(MAKE) -C $@

OBJECT_FILES = $(shell find $(TARGET_DIRS) -type f -name "*.o")

kernel8.img : $(OBJECT_FILES) linkscript.x
	$(LD) -T ./linkscript.x $(OBJECT_FILES) -o $(BUILD_DIR)/kernel8.elf
	$(OBJCOPY) $(BUILD_DIR)/kernel8.elf -O binary $(BUILD_DIR)/kernel8.img
	cp $(BUILD_DIR)/kernel8.img /media/psf/AllFiles/Volumes/boot/
