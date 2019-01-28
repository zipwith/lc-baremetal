#----------------------------------------------------------------------------
include ../Makefile.common

all:	cdrom.iso

run:	cdrom.iso
	$(QEMU) -m 32 -serial stdio -cdrom cdrom.iso

include ../Makefile.cdrom

image:
	make -C kernel
	make -C user
	../mimg/mimgmake image \
		noload:../mimg/mimgload \
		bootdata:0x1000-0x3fff \
		kernel/kernel \
		user/user

#----------------------------------------------------------------------------
# tidy up after ourselves ...
clean:
	make -C kernel clean
	make -C user   clean
	-rm -rf grub.cmds cdrom cdrom.iso image image.gz

#----------------------------------------------------------------------------
