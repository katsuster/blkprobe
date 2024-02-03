
TARGET  = blkprobe
OBJS    = blkprobe.o

CC      = gcc
RM      = rm -f

CFLAGS  = -Wall -O2 -D_FILE_OFFSET_BITS=64 -I/usr/include/fuse3
LDFLAGS =
LDLIBS  = -lfuse3 -lpthread

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) $(LDFLAGS) -o $@ $< $(LDLIBS)

install: all

clean: 
	$(RM) $(TARGET) $(OBJS)

dist-clean: clean

.PHONY: all install clean dist-clean

