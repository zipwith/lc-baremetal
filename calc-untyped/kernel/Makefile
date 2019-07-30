include ../../Makefile.common

all: kernel

#----------------------------------------------------------------------------
# kernel:  A trivial kernel that can be invoked by mimgload
KOBJS   = init.o opt-combined.o
kernel: ${KOBJS} kernel.ld
	$(LD) -T kernel.ld -o kernel ${KOBJS} ${LIBPATH} --print-map > kernel.map
	strip kernel

init.o: init.s
	$(CC) -c -o init.o init.s

kernel.ll: kernel.llc
	milc $(MILOPTS) kernel.llc -lkernel.ll -i../../libs-lc \
		--llvm-main=kernel --mil-main=kernel

kernel.bc: kernel.ll
	llvm-as -o=kernel.bc kernel.ll

combined.bc: kernel.bc
	llvm-link -o=combined.bc kernel.bc ../../libs-lc/ia32.bc

opt-combined.bc: combined.bc
	opt -always-inline -o=opt-combined.bc combined.bc

opt-combined.o: opt-combined.bc
	clang -c -m32 ${CCOPTS} -o opt-combined.o opt-combined.bc
	llc -O2 -march=x86 opt-combined.bc  # for debugging/inspection of .s

#----------------------------------------------------------------------------
# tidy up after ourselves ...
clean:
	-rm -r kernel opt-combined.s *.bc *.o *.map *.ll

#----------------------------------------------------------------------------
