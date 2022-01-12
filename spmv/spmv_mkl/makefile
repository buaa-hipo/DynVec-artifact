CC=icc
CPP=icpc
COMPILER=$(CC) -O3 -std=c++11 -o $@ -c $^
COMPILER_PP=$(CPP) -O3 -std=c++11 -o $@ -c $^
LINK=$(CC) -O3 -o $@ $^ -mkl -qopenmp -lpapi
#LINK=gcc -o $@ $^ #-mkl=parallel -qopenmp

app:main.o csr_matrix.o util.o Timers.o csr_spmv.o csr_analyse.o csr_spmv_simd.o
	$(LINK)
main.o:main.cpp
	$(COMPILER)
util.o:util.c
	$(COMPILER)
csr_matrix.o:csr_matrix.cpp
	$(COMPILER)
csr_spmv.o:csr_spmv.cpp
	$(COMPILER)
Timers.o:Timers.cpp
	$(COMPILER_PP)
csr_analyse.o:csr_analyse.cpp
	$(COMPILER)
csr_spmv_simd.o:csr_spmv_simd.cpp
	$(COMPILER)
clean:
	rm app *.o


