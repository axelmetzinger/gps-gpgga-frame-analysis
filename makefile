SOURCES = $(wildcard *.c)
BINAIRES = $(patsubst %.c, %.o, ${SOURCES})
GCC = gcc

all: gps.o main

main: ${BINAIRES}
	${GCC} $^ -o $@

%.o: %.c %.h
	${GCC} -c $<

clean:
	rm *.o
	rm main
