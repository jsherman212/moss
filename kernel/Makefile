TARGET_DIRS = libc

ASM_SOURCES = $(wildcard *.s)
C_SOURCES = $(wildcard *.c)

OBJECT_FILES = $(patsubst %.s, %.o, $(ASM_SOURCES))
OBJECT_FILES += $(patsubst %.c, %.o, $(C_SOURCES))

TARGET_DIR_OBJECT_FILES = $(shell find $(TARGET_DIRS) -type f -name "*.o")

all : $(TARGET_DIRS) $(OBJECT_FILES)

.PHONY : $(TARGET_DIRS) $(TARGET_DIR_OBJECT_FILES)

$(TARGET_DIRS) :
	$(MAKE) -C $@

%.o : %.c
	$(CC) $(CFLAGS) -c $< -o $@
