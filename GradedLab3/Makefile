CC=gcc
C_FLAGS= -Wall -fsanitize=address,undefined -g #-Werror -pedantic
L_FLAGS=-lm -lpthread -fsanitize=address,undefined
TARGETS=main
OBJECTS=main.o

.PHONY: all clean

all: main

main: main.o
	${CC} -o main main.o ${L_FLAGS}

main.o: main.c
	${CC} -o main.o -c main.c ${C_FLAGS}

clean:
	-rm -rf ${TARGETS} ${OBJECTS}