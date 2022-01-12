CC=icpc
#CFLAGS=-O3 -ffast-math `llvm-config --cflags`  

CFLAGS=-std=c++11 -O3 -xHost 
LD=icpc 
LDFLAGS=-O3 -xHost -std=c++11 

all: sum
LKFILE= sum.o Timers.o csr_matrix.o util.o pagerank.o
sum: $(LKFILE)
	$(LD) $^ $(LDFLAGS) -o $@

sum.o: sum.cpp
	$(CC) $(CFLAGS) -c $<
#	$(CC) $< -S $(CFLAGS) -o sum.S
Timers.o:Timers.cpp
	$(CC) $(CFLAGS) $< -c -o $@

csr_matrix.o:csr_matrix.cpp
	$(CC) $(CFLAGS) $< -c -o $@
util.o:util.cpp
	$(CC) $(CFLAGS) $< -c -o $@
pagerank.o:pagerank.cpp
	$(CC)  $(CFLAGS) $< -c -o $@
clean:
	rm *.o sum
