PREFIX = aarch64-linux-gnu

BUILD_DIR = build

# Please run make in the root of this project and not anywhere else
ROOT_DIR = $(shell pwd)

AS = $(PREFIX)-as
CC = $(PREFIX)-gcc

CFLAGS = -ffreestanding -nostdlib -nostartfiles -I$(ROOT_DIR)/include

ifeq ($(MOSS_DEBUG), 1)
	CFLAGS += -DMOSS_DEBUG
endif

ifeq ($(VM_DEBUG), 1)
	CFLAGS += -DVM_DEBUG
endif

ifeq ($(FB_DEBUG), 1)
	CFLAGS += -DFB_DEBUG
endif

LD = $(PREFIX)-ld
LDFLAGS = -z max-page-size=4096 -z common-page-size=4096
LDFLAGS += -T ./linkscript.x
OBJCOPY = $(PREFIX)-objcopy

export AS
export CC
export CFLAGS

TARGET_DIRS = boot kernel
OBJECT_FILES = $(shell find $(TARGET_DIRS) -type f -name "*.o")

all : $(TARGET_DIRS) kernel8.img

.PHONY : $(OBJECT_FILES) $(TARGET_DIRS)

$(TARGET_DIRS) :
	$(MAKE) -C $@

kernel8.img : $(OBJECT_FILES) linkscript.x
	$(LD) $(LDFLAGS) $(OBJECT_FILES) -o $(BUILD_DIR)/kernel8.elf
	$(OBJCOPY) $(BUILD_DIR)/kernel8.elf -O binary $(BUILD_DIR)/kernel8.img
	cp $(BUILD_DIR)/kernel8.img /media/psf/AllFiles/Volumes/boot/
