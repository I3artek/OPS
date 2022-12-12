CC=gcc
C_FLAGS=-Wall -Werror -pedantic -g
L_FLAGS=-lm -fsanitize=address,undefined
XD=-lrt

.PHONY: all clean

all: main

main: main.o
	${CC} -o main main.o ${L_FLAGS}

main.o: main.c
	${CC} -o main.o -c main.c ${C_FLAGS}

clean:
	-rm -rf main main.o
