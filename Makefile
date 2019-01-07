#----------------------------------------------------------------------------
include Makefile.common

.phony: all run libs clean

#BOOT = hello-lc
#BOOT = winhello
BOOT = example-mimg-lc
#BOOT = calc-untyped

all:	libs
	make -C ${BOOT}

run:	libs
	make -C ${BOOT} run

libs:
	make -C simpleio
	make -C mimg

clean:
	-make -C simpleio         clean
	-make -C mimg             clean
	-make -C hello-lc         clean
	-make -C winhello	  clean
	-make -C example-mimg-lc  clean
	-make -C calc-untyped     clean

#----------------------------------------------------------------------------
