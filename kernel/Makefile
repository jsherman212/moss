ASM_SOURCES = $(wildcard *.s)
C_SOURCES = $(wildcard *.c)

OBJECT_FILES = $(patsubst %.s, %.o, $(ASM_SOURCES))
OBJECT_FILES += $(patsubst %.c, %.o, $(C_SOURCES))

all : $(OBJECT_FILES)

%.o : %.c
	$(CC) $(CFLAGS) -c $< -o $@
