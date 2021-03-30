SOURCES = $(wildcard *.s)
OBJECT_FILES = $(patsubst %.s, %.o, $(SOURCES))
# XXX hdrs patsubst .s with .h from $(SOURCES) if needed

all : $(OBJECT_FILES)

%.o : %.s
	$(CC) -c $< -o $@
