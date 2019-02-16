#----------------------------------------------------------------------------
# Makefile for a simple bare metal program
#----------------------------------------------------------------------------
# Basic settings:

CC	  = clang -m32
CCOPTS    = -g -std=gnu99 -O -Wall -nostdlib -nostdinc -Winline \
	    -nodefaultlibs -fno-builtin -fomit-frame-pointer \
	    -fno-stack-protector -freg-struct-return
LD        = ld -melf_i386
QEMU      = qemu-system-i386

#----------------------------------------------------------------------------
# Build rules:

.SUFFIXES:
.SUFFIXES: .S .s .lc .llc .ll .c .cc .h .bc .o .a .iso .img .gz .cdepn .graph

.PHONY: all clean

all:	cdrom.iso

run:	cdrom.iso
	$(QEMU) -m 32 -serial stdio -cdrom cdrom.iso

cdrom.iso: cdrom
	grub-mkrescue -o cdrom.iso cdrom

# make a basic cdrom image
cdrom: grub.cfg hello
	mkdir -p cdrom/boot/grub
	cp grub.cfg cdrom/boot/grub
	cp hello cdrom
	touch cdrom

OBJS   = boot.o opt-combined.o

hello: ${OBJS} hello.ld
	$(LD) -T hello.ld -o hello ${OBJS} --print-map > hello.map
	objcopy --only-keep-debug hello hello.sym
	strip hello

boot.o: boot.s
	$(CC) -c -o boot.o boot.s

hello.ll: hello.llc
	milc hello.llc -lhello.ll --32 -pcsoros -i../libs-lc \
		--target=i386-pc-linux-gnu \
		--llvm-main=hello \
		--mil-main=hello

hello.bc: hello.ll
	llvm-as -o=hello.bc hello.ll

combined.bc: hello.bc
	llvm-link -o=combined.bc hello.bc ../libs-lc/ia32.bc

opt-combined.bc: combined.bc
	opt -always-inline -o=opt-combined.bc combined.bc

opt-combined.o: opt-combined.bc
	clang -c -m32 ${CCOPTS} -o opt-combined.o opt-combined.bc
	llc -O2 -march=x86 opt-combined.bc  # for debugging/inspection of .s

#----------------------------------------------------------------------------
# tidy up after ourselves ...
clean:
	-rm -rf cdrom cdrom.iso hello hello.ll opt-combined.s \
		*.o *.bc *.lst *.map *.sym

#----------------------------------------------------------------------------
