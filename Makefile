#----------------------------------------------------------------------------
include Makefile.common

.phony: all run libs runall clean

#BOOT = serial-lc
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
	make -C libs-lc

runall:	libs
	make -C serial-lc        run
	make -C hello-lc         run
	make -C winhello	  run
	make -C example-mimg-lc  run
	make -C example-idt-lc   run
	make -C switching-lc     run
	make -C calc-untyped     run

clean:
	-make -C simpleio         clean
	-make -C mimg             clean
	-make -C libs-lc          clean
	-make -C serial-lc        clean
	-make -C hello-lc         clean
	-make -C winhello	  clean
	-make -C example-mimg-lc  clean
	-make -C example-idt-lc   clean
	-make -C switching-lc     clean
	-make -C calc-untyped     clean

#----------------------------------------------------------------------------
