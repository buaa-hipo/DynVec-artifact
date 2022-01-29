CC=clang++
#CFLAGS=-O3 -ffast-math `llvm-config --cflags` -g 
#CC=g++
#CFLAGS=-g -ffast-math `llvm-config --cflags`
#LD=clang++ 
CFLAGS=-O3 -ffast-math `llvm-config --cxxflags --ldflags --libs core executionengine mcjit interpreter analysis native bitwriter --system-libs` -g
#LD=g++
LD=clang++
LDFLAGS= -ffast-math  `llvm-config --cxxflags --ldflags --libs core executionengine mcjit interpreter analysis native bitwriter --system-libs`  -rdynamic -g

#-lLLVMDebugTool 

all: sum
LKFILE= main.o llvm_common.o 
sum: $(LKFILE)
	$(LD) $^ $(LDFLAGS) -o $@

main.o: main.cpp llvm_module.h
	$(CC) $(CFLAGS) -c $<
#	$(CC) $< -S $(CFLAGS) -o sum.S
llvm_common.o:llvm_common.cpp
	$(CC) $(CFLAGS) -c $<
clean:
	-rm -f sum.o sum sum.bc sum.ll *.o
