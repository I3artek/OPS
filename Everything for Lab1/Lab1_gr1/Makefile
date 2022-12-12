CC=gcc
C_FLAGS=-Wall -Werror -pedantic -g
L_FLAGS=-lm -fsanitize=address,undefined
XD=-lrt
TARGETS=main better_main better_main_without_globals
OBJECTS=main.o better_main.o better_main_without_globals.o

.PHONY: all clean

all: ${TARGETS}

main: main.o
	${CC} -o main main.o ${L_FLAGS}

main.o: main.c
	${CC} -o main.o -c main.c ${C_FLAGS}

better_main: better_main.o
	${CC} -o better_main better_main.o ${L_FLAGS}

better_main.o: better_main.c
	${CC} -o better_main.o -c better_main.c ${C_FLAGS}

better_main_without_globals: better_main_without_globals.o
	${CC} -o better_main_without_globals better_main_without_globals.o ${L_FLAGS}

better_main_without_globals.o: better_main_without_globals.c
	${CC} -o better_main_without_globals.o -c better_main_without_globals.c ${C_FLAGS}

clean:
	-rm -rf ${TARGETS} ${OBJECTS}
