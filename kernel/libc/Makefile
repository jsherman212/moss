SOURCES = $(wildcard *.c)
OBJECT_FILES = $(patsubst %.c, %.o, $(SOURCES))

all : $(OBJECT_FILES)

%.o : %.c
	$(CC) $(CFLAGS) -c $< -o $@
