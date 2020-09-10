.PHONY: all clean

CC=gcc
CFLAGS=-O3
SRC=radix.c

all: test radix.o

radix.o: $(SRC)
	$(CC) $(CFLAGS) -c radix.c

test: $(SRC)
	$(CC) $(CFLAGS) -D _UNIT_TEST -o test radix.c

clean:
	rm -rf radix.o test
