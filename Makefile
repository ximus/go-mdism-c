CC ?= gcc
CFLAGS += -std=gnu99
CFLAGS += -g -DDEBUG=1

all:
	$(CC) $(CFLAGS) main.c -o bin/main.o

debug:
	gdb bin/main.o

clean:
	rm -rf bin/*