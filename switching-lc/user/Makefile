include ../../Makefile.common

INCPATH = -I ../../simpleio
LIBPATH = -L ../../simpleio -lio

CC = gcc -m32

all:	user user2

#----------------------------------------------------------------------------
# A simple user program:
UOBJS	= user.o userlib.o
user:	${UOBJS} user.ld
	$(LD) -T user.ld -o user ${UOBJS} ${LIBPATH} --print-map > user.map
	strip user

user.o:	user.c
	$(CC) ${CCOPTS} ${INCPATH} -o user.o -c user.c

#----------------------------------------------------------------------------
# A second simple user program:
UOBJS2	= user2.o userlib.o
user2:	${UOBJS2} user.ld
	$(LD) -T user2.ld -o user2 ${UOBJS2} ${LIBPATH} --print-map > user2.map
	strip user2

user2.o:user2.c
	$(CC) ${CCOPTS} ${INCPATH} -o user2.o -c user2.c

#----------------------------------------------------------------------------
# An assembly code library for both of the user programs:
userlib.o: userlib.s
	$(CC) -Wa,-alsm=userlib.lst -c -o userlib.o userlib.s

#----------------------------------------------------------------------------
# tidy up after ourselves ...
clean:
	-rm -r user user2 *.o *.lst *.map

#----------------------------------------------------------------------------
