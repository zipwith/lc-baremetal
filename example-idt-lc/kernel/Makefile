include ../../Makefile.common

all: kernel

#----------------------------------------------------------------------------
# A simple protected mode kernel that switches back and forth
# between user and kernel mode when a system call is invoked.

KOBJS   = init.o kernel.o
kernel: ${KOBJS} kernel.ld
	$(LD) -T kernel.ld -o kernel ${KOBJS} ${LIBPATH} --print-map > kernel.map
	strip kernel

init.o: init.s
	$(CC) -g -c -o init.o init.s

kernel.o: kernel.llc
	milc $(MILOPTS) kernel.llc -lkernel.ll -i../../libs-lc \
		--llvm-main=kernel --mil-main=kernel
	$(CC) -g ${CCOPTS} -o kernel.o -c kernel.ll

#----------------------------------------------------------------------------
# tidy up after ourselves ...
clean:
	-rm -r kernel *.o *.map *.ll

#----------------------------------------------------------------------------
