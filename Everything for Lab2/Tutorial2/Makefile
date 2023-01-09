CC=gcc
C_FLAGS=-Wall -Werror -pedantic -g
L_FLAGS=-lm -fsanitize=address,undefined
XD=-lrt

.PHONY: all clean

all: main1 main2 main3 main4

main1: main1.o
	${CC} -o main1 main1.o ${L_FLAGS}

main1.o: main1.c
	${CC} -o main1.o -c main1.c ${C_FLAGS}

main2: main2.o
	${CC} -o main2 main2.o ${L_FLAGS}

main2.o: main2.c
	${CC} -o main2.o -c main2.c ${C_FLAGS}

main3: main3.o
	${CC} -o main3 main3.o ${L_FLAGS}

main3.o: main3.c
	${CC} -o main3.o -c main3.c ${C_FLAGS}

main4: main4.o
	${CC} -o main4 main4.o ${L_FLAGS}

main4.o: main4.c
	${CC} -o main4.o -c main4.c ${C_FLAGS}

clean:
	-rm -rf main main.o
