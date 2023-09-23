CC=clang
CFLAGS=-c -Wall -pedan3c -std=c99
LIBS=-lm # note: the l means library, m means math
all: main
main: main.o cross.o
$(CC) main.o cross.o -o main $(LIBS)
main.o: main.c
$(CC) $(CFLAGS) main.c
cross.o: cross.c cross.h
$(CC) $(CFLAGS) cross.c
clean:
rm -f *.o main
