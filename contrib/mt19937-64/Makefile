CC = gcc
CCFLAGS = -g -O3

all: mt19937-64test

mt19937-64:
	${CC} mt19937-64.c -c ${CCFLAGS}

mt19937-64test: mt19937-64
	${CC} mt19937-64.o mt19937-64test.c -o mt19937-64test ${CCFLAGS}

clean:
	rm -rf *.o mt19937-64test
