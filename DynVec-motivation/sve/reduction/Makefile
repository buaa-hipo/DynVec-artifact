CC=aarch64-linux-gnu-g++-10
CFLAGS=-O3 -g -march=armv8-a+sve

all: reduction reduction_merge

reduction: reduction.o pure.o main.o
	$(CC) $(CFLAGS) reduction.o pure.o main.o -o reduction
reduction_merge: reduction_merge.o pure.o main_merge.o
	$(CC) $(CFLAGS) reduction_merge.o pure.o main_merge.o -o reduction_merge

reduction_merge.o: reduction_merge.cpp eval.h common.h mode.h
	$(CC) $(CFLAGS) reduction_merge.cpp -c
main_merge.o: main_merge.cpp mode.h eval.h common.h
	$(CC) $(CFLAGS) main_merge.cpp -c
reduction.o: reduction.cpp eval.h common.h mode.h
	$(CC) $(CFLAGS) reduction.cpp -c
pure.o: pure.cpp mode.h eval.h common.h
	$(CC) $(CFLAGS) pure.cpp -c -fno-tree-vectorize
	$(CC) $(CFLAGS) pure.cpp -c
main.o: main.cpp mode.h eval.h common.h
	$(CC) $(CFLAGS) main.cpp -c
clean:
	rm *.o reduction reduction_merge

