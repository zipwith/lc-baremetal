include ../../Makefile.common

all: kernel

#----------------------------------------------------------------------------
# kernel:  A trivial kernel that can be invoked by mimgload
KOBJS   = init.o kernel.o
kernel: ${KOBJS} kernel.ld
	$(LD) -T kernel.ld -o kernel ${KOBJS} ${LIBPATH} --print-map > kernel.map
	strip kernel

init.o: init.s
	$(CC) -c -o init.o init.s

kernel.o: kernel.llc
	milc kernel.llc -lkernel.ll --32 -pcsosrsos -i../../libs-lc \
		--target=i386-pc-linux-gnu \
		--llvm-main=kernel \
		--mil-main=kernel
	$(CC) ${CCOPTS} -c -o kernel.o kernel.ll

#----------------------------------------------------------------------------
# tidy up after ourselves ...
clean:
	-rm -r kernel *.o *.map *.ll

#----------------------------------------------------------------------------
