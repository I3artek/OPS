CC=gcc
C_FLAGS=-Wall -Werror -pedantic -g
L_FLAGS=-lm -lrt -fsanitize=address,undefined
TARGETS=main
OBJECTS=main.o

.PHONY: all clean

all: ${TARGETS}

main: main.o
	${CC} -o main main.o ${L_FLAGS}

main.o: main.c
	${CC} -o main.o -c main.c ${C_FLAGS}

clean:
	-rm -rf ${TARGETS} ${OBJECTS}