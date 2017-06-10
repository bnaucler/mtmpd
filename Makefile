CC = cc
TARGET = mtmpd
SOURCE = *.c
DESTDIR = /usr/bin
CFLAGS ?= -Wall -g -ljansson -lcurl

all:
	$(CC) $(CFLAGS) -o $(TARGET) $(SOURCE)

clean:
	rm -f $(TARGET)

install:
	cp $(TARGET) $(DESTDIR)
