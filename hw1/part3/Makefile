.PHONY: all hello main.o clean

# gcc is the current compiler
CC=gcc
# options being passed to the above comipler
CFLAGS=-Wall -g -fno-pic

all: hello

hello: main.o
	$(CC) $(CFLAGS) main.o -o hello

main.o: main.c
	$(CC) -c $(CFLAGS) main.c

clean:
	rm *.o hello