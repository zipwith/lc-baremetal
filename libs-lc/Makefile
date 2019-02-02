#----------------------------------------------------------------------------
# Build rules:

.SUFFIXES:
.SUFFIXES: .S .s .lc .llc .ll .c .cc .h .o .a .iso .img .gz .cdepn .graph

.PHONY: all clean

all:	portio.bc

portio.bc: portio.ll
	llvm-as -o=portio.bc portio.ll

#----------------------------------------------------------------------------
# tidy up after ourselves ...
clean:
	-rm -rf portio.bc

#----------------------------------------------------------------------------
