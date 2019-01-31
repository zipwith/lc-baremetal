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
.SUFFIXES: .S .s .lc .llc .c .cc .h .o .a .iso .img .gz .cdepn .graph

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

OBJS   = boot.o hello.o portio.o

hello: ${OBJS} hello.ld
	$(LD) -T hello.ld -o hello ${OBJS} --print-map > hello.map
	objcopy --only-keep-debug hello hello.sym
	strip hello

boot.o: boot.s
	$(CC) -c -o boot.o boot.s

hello.o: hello.llc serial.llc
	milc hello.llc -lhello.ll --32 -pcsoros -i../libs-lc \
		--target=i386-pc-linux-gnu \
		--llvm-main=hello \
		--mil-main=hello
	clang -c -m32 ${CCOPTS} -o hello.o hello.ll

portio.o: portio.ll
	clang -c -m32 -o portio.o portio.ll

#Replace the above rule for hello.o with the following to build the
#original version using C:
#hello.o: hello.c
#	$(CC) ${CCOPTS} -o hello.o -c hello.c

#----------------------------------------------------------------------------
# tidy up after ourselves ...
clean:
	-rm -rf cdrom cdrom.iso hello *.o hello.ll *.lst *.map *.sym

#----------------------------------------------------------------------------
