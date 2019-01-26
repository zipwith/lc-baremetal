include ../Makefile.common

# override setting in Makefile.common
CC = gcc -m32

all:	libio.a

libio.a: simpleio.o serial.o ia32-getc.o
	ar r libio.a simpleio.o serial.o ia32-getc.o
	ranlib libio.a

#----------------------------------------------------------------------------
# compile the simple IO library:
simpleio.o: simpleio.c simpleio.h
	$(CC) ${CCOPTS} -o simpleio.o -c simpleio.c

# compile the getc code:
ia32-getc.o: ia32-getc.c
	$(CC) ${CCOPTS} -o ia32-getc.o -c ia32-getc.c

# compile the serial output code:
serial.o: serial.s
	$(CC) -c -o serial.o serial.s

#----------------------------------------------------------------------------
# tidy up after ourselves ...
clean:
	-rm -rf simpleio.o serial.o ia32-getc.o libio.a

#----------------------------------------------------------------------------
