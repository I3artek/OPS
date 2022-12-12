CC=gcc
C_FLAGS=-Wall -Werror -pedantic -g
L_FLAGS=-lm -lrt -fsanitize=address,undefined
TARGETS=prog3 prog4 prog5 prog6 prog7 prog9 prog10
OBJECTS=prog3.o prog4.o prog5.o prog6.o prog7.o prog9.o prog10.o

all: ${TARGETS}

prog3: prog3.o
	${CC} -o prog3 prog3.o ${L_FLAGS}

prog3.o: prog3.c
	${CC} -o prog3.o -c prog3.c ${C_FLAGS}

prog4: prog4.o
	${CC} -o prog4 prog4.o ${L_FLAGS}

prog4.o: prog4.c
	${CC} -o prog4.o -c prog4.c ${C_FLAGS}

prog5: prog5.o
	${CC} -o prog5 prog5.o ${L_FLAGS}

prog5.o: prog5.c
	${CC} -o prog5.o -c prog5.c ${C_FLAGS}

prog6: prog6.o
	${CC} -o prog6 prog6.o ${L_FLAGS}

prog6.o: prog6.c
	${CC} -o prog6.o -c prog6.c ${C_FLAGS}

prog7: prog7.o
	${CC} -o prog7 prog7.o ${L_FLAGS}

prog7.o: prog7.c
	${CC} -o prog7.o -c prog7.c ${C_FLAGS}

prog9: prog9.o
	${CC} -o prog9 prog9.o ${L_FLAGS}

prog9.o: prog9.c
	${CC} -o prog9.o -c prog9.c ${C_FLAGS}

prog10: prog10.o
	${CC} -o prog10 prog10.o ${L_FLAGS}

prog10.o: prog10.c
	${CC} -o prog10.o -c prog10.c ${C_FLAGS}

.PHONY: clean all

clean:
	-rm -rf ${TARGETS} ${OBJECTS}

