include ../Makefile.common

# override setting in Makefile.common
CC = gcc -m32

all:	mimgload mimgmake

#----------------------------------------------------------------------------
# mimgload:  A memory image loader that runs at boot time
LOBJS   = bootload.o mimgload.o
mimgload: ${LOBJS} mimgload.ld
	$(LD) -T mimgload.ld -o mimgload ${LOBJS} \
		-L ../simpleio -lio --print-map > mimgload.map
	strip mimgload

bootload.o: bootload.s
	$(CC) -c -o bootload.o bootload.s

mimgload.o: mimgload.c
	$(CC) ${CCOPTS} -I ../simpleio -o mimgload.o -c mimgload.c

#----------------------------------------------------------------------------
# mimgmake:  A tool for constructing memory images
mimgmake: mimgmake.c
	$(CC) -o mimgmake mimgmake.c

#----------------------------------------------------------------------------
# tidy up after ourselves ...
clean:
	-rm -r mimgload mimgmake *.o *.lst *.map

#----------------------------------------------------------------------------
