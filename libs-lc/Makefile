#----------------------------------------------------------------------------
# Build rules:

.SUFFIXES:
.SUFFIXES: .S .s .lc .llc .ll .c .cc .h .bc .o .a .iso .img .gz .cdepn .graph

.PHONY: all clean

BCS	= ia32.bc

all:	$(BCS)

.ll.bc:
	llvm-as -o=$*.bc $*.ll

#----------------------------------------------------------------------------
# tidy up after ourselves ...
clean:
	-rm -rf ${BCS}

#----------------------------------------------------------------------------
