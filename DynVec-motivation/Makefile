CC=aarch64-linux-gnu-gcc-10
all: reduce_sve_512 reduce_sve_256 reduce_sve_512_double reduce_sve_256_double
	${CC} -O3 -g -march=armv8-a+sve -DUSE_128 gather_sve.cc -o gather_sve_128 -msve-vector-bits=128
	${CC} -O3 -g -march=armv8-a+sve -DUSE_256 gather_sve.cc -o gather_sve_256 -msve-vector-bits=256 
	${CC} -O3 -g -march=armv8-a+sve -DUSE_512 gather_sve.cc -o gather_sve_512 -msve-vector-bits=512
	${CC} -O3 -g -march=armv8-a+sve -DUSE_256 -DUSE_DOUBLE gather_sve.cc -o gather_sve_256_double -msve-vector-bits=256
	${CC} -O3 -g -march=armv8-a+sve -DUSE_512 -DUSE_DOUBLE gather_sve.cc -o gather_sve_512_double -msve-vector-bits=512
	${CC} -O3 -g -march=armv8-a+sve -DUSE_128 scatter_sve.cc -o scatter_sve_128 -msve-vector-bits=128
	${CC} -O3 -g -march=armv8-a+sve -DUSE_256 scatter_sve.cc -o scatter_sve_256 -msve-vector-bits=256 
	${CC} -O3 -g -march=armv8-a+sve -DUSE_512 scatter_sve.cc -o scatter_sve_512 -msve-vector-bits=512
	${CC} -O3 -g -march=armv8-a+sve -DUSE_256 -DUSE_DOUBLE scatter_sve.cc -o scatter_sve_256_double -msve-vector-bits=256
	${CC} -O3 -g -march=armv8-a+sve -DUSE_512 -DUSE_DOUBLE scatter_sve.cc -o scatter_sve_512_double -msve-vector-bits=512

reduce_sve_512: reduce_sve.cc eval.h common.h
	${CC} -O3 -g -march=armv8-a+sve -DUSE_512 -DUSE_SINGLE reduce_sve.cc -o reduce_sve_512 -msve-vector-bits=512

reduce_sve_256: reduce_sve.cc eval.h common.h
	${CC} -O3 -g -march=armv8-a+sve -DUSE_256 -DUSE_SINGLE reduce_sve.cc -o reduce_sve_256 -msve-vector-bits=256

reduce_sve_512_double: reduce_sve.cc eval.h common.h
	${CC} -O3 -g -march=armv8-a+sve -DUSE_512 -DUSE_DOUBLE reduce_sve.cc -o reduce_sve_512_double -msve-vector-bits=512

reduce_sve_256_double: reduce_sve.cc eval.h common.h
	${CC} -O3 -g -march=armv8-a+sve -DUSE_256 -DUSE_DOUBLE reduce_sve.cc -o reduce_sve_256_double -msve-vector-bits=256

clean:
	rm -f gather_sve gather_sve_128 gather_sve_256* gather_sve_512* reduce_sve_512 reduce_sve_256 reduce_sve_512_double reduce_sve_256_double scatter_sve scatter_sve_128 scatter_sve_256* scatter_sve_512*

define run_eval
        mkdir data
        ./$(1)
        mv data $(2)
endef

run:
	$(call run_eval,gather_sve_256,data-gather-float-256)
	$(call run_eval,gather_sve_256_double,data-gather-double-256)
	$(call run_eval,scatter_sve_256,data-scatter-float-256)
	$(call run_eval,scatter_sve_256_double,data-scatter-double-256)
