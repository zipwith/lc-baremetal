#----------------------------------------------------------------------------
include ../Makefile.common

all:	cdrom.iso

run:	cdrom.iso
	$(QEMU) -m 32 -serial stdio -cdrom cdrom.iso

include ../Makefile.cdrom

image:
	make -C kernel
	../mimg/mimgmake image \
		noload:../mimg/mimgload \
		bootdata:0x1000-0x3fff \
		kernel/kernel.llc@0x400000 \
		Makefile@0x440000 \
		Makefile@0x460000 \
		kernel/kernel.llc@0x500000 \
		kernel/kernel

#----------------------------------------------------------------------------
# tidy up after ourselves ...
clean:
	make -C kernel clean
	-rm -rf grub.cmds cdrom cdrom.iso image image.gz

#----------------------------------------------------------------------------
