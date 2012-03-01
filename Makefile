
TARGET  = blkprobe
OBJS    = blkprobe.o

CC      = gcc
RM      = rm -f

CFLAGS  = -Wall -O2 -D_FILE_OFFSET_BITS=64
LDFLAGS = -lfuse -lpthread

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) $(LDFLAGS) -o $@ $<

install: all

clean: 
	$(RM) $(TARGET) $(OBJS)

dist-clean: clean

.PHONY: all install clean dist-clean

