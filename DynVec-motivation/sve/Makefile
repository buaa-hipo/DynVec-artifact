CC=aarch64-linux-gnu-gcc-10
CFLAGS=-O3 -g -march=armv8-a+sve

all: gather_sve_512 gather_sve_512_double gather_sve_256 gather_sve_256_double scatter_sve_512 scatter_sve_512_double scatter_sve_256 scatter_sve_256_double reduce_sve_512 reduce_sve_256 reduce_sve_512_double reduce_sve_256_double

gather_sve_512: gather_sve.cc eval.h common.h
	$(CC) $(CFLAGS) -DUSE_512 gather_sve.cc -o gather_sve_512 -msve-vector-bits=512

gather_sve_256: gather_sve.cc eval.h common.h
	$(CC) $(CFLAGS) -DUSE_256 gather_sve.cc -o gather_sve_256 -msve-vector-bits=256

gather_sve_256_double: gather_sve.cc eval.h common.h
	$(CC) $(CFLAGS) -DUSE_256 -DUSE_DOUBLE gather_sve.cc -o gather_sve_256_double -msve-vector-bits=256

gather_sve_512_double: gather_sve.cc eval.h common.h
	$(CC) $(CFLAGS) -DUSE_512 -DUSE_DOUBLE gather_sve.cc -o gather_sve_512_double -msve-vector-bits=512

scatter_sve_512: scatter_sve.cc eval.h common.h
	$(CC) $(CFLAGS) -DUSE_512 scatter_sve.cc -o scatter_sve_512 -msve-vector-bits=512

scatter_sve_256: scatter_sve.cc eval.h common.h
	$(CC) $(CFLAGS) -DUSE_256 scatter_sve.cc -o scatter_sve_256 -msve-vector-bits=256

scatter_sve_256_double: scatter_sve.cc eval.h common.h
	$(CC) $(CFLAGS) -DUSE_256 -DUSE_DOUBLE scatter_sve.cc -o scatter_sve_256_double -msve-vector-bits=256

scatter_sve_512_double: scatter_sve.cc eval.h common.h
	$(CC) $(CFLAGS) -DUSE_512 -DUSE_DOUBLE scatter_sve.cc -o scatter_sve_512_double -msve-vector-bits=512


reduce_sve_512: reduce_sve.cc eval.h common.h
	$(CC) $(CFLAGS) -DUSE_512 -DUSE_SINGLE reduce_sve.cc -o reduce_sve_512 -msve-vector-bits=512

reduce_sve_256: reduce_sve.cc eval.h common.h
	$(CC) $(CFLAGS) -DUSE_256 -DUSE_SINGLE reduce_sve.cc -o reduce_sve_256 -msve-vector-bits=256

reduce_sve_512_double: reduce_sve.cc eval.h common.h
	$(CC) $(CFLAGS) -DUSE_512 -DUSE_DOUBLE reduce_sve.cc -o reduce_sve_512_double -msve-vector-bits=512

reduce_sve_256_double: reduce_sve.cc eval.h common.h
	$(CC) $(CFLAGS) -DUSE_256 -DUSE_DOUBLE reduce_sve.cc -o reduce_sve_256_double -msve-vector-bits=256

clean:
	rm -f gather_sve gather_sve_128 gather_sve_256* gather_sve_512* reduce_sve_512 reduce_sve_256 reduce_sve_512_double reduce_sve_256_double scatter_sve scatter_sve_128 scatter_sve_256* scatter_sve_512*
