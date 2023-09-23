# Makefile

CC = gcc
CFLAGS = -std=c99 -Wall -pedantic -fpic -shared
SRC = mxarr.c
OBJ = mxarr.o
LIB = libmxarr.so

all: $(LIB)

mxarr.o: $(SRC) mxarr.h
    $(CC) $(CFLAGS) -c -o $(OBJ) $(SRC)

$(LIB): $(OBJ)
    $(CC) $(CFLAGS) -o $(LIB) $(OBJ)

clean:
    rm -f $(OBJ) $(LIB)
