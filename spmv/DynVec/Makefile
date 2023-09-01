VPATH=build.x86
CC=g++
#CC=clang++
#CC=icpc
#CFLAGS=-O3 -ffast-math `llvm-config --cflags` 
#ARCH=core-avx2
PAPI_ROOT?=../papi-6.0.0/install/
#ARCH=knl
#LLVM_BASE?=../llvm-8.0.0/install/
LLVM_BASE?=/root/llvm-16.0.0-debug/
LLVM_INCLUDE=${LLVM_BASE}/include/
LLVM_LIB=${LLVM_BASE}/lib/
INCLUDE=-Iparse/ -Inode/ -Ilog/ -Itype/ -Ihash/ -Itools_set/ -Istatement/ -Ipass/ -Ibit2addr/ -Iutil/ -Iio_matrix/ -ITimer/ -Itransform_data/ -Iintelligent_unroll/ -Iinclude/ -I./ -I${LLVM_INCLUDE} -I$(PAPI_ROOT)/include
#CFLAGS=-march=$(ARCH) -std=c++11 -O3 -ffast-math  -frtti -Wall  -fPIC
CFLAGS= -std=c++17 -O0 -ffast-math  -frtti -Wall  -fPIC -mavx2 -g -D __AVX2__
#CFLAGS= -std=c++17 -O0 -ffast-math  -frtti -Wall  -fPIC -mavx512cd
#CFLAGS= -std=c++17 -O0 -ffast-math  -frtti -Wall  -fPIC -g -D __SVE__
CFLAGSPermitWarning=$(CFLAGS)
#-Werror

INCLUDE_LLVM_DIR=`llvm-config --cxxflags` 
#CFLAGS=-g -ffast-math `llvm-config --cflags`
LD=g++ 
#LD=clang++
#LD=icpc
#LDFLAGS= -L${LLVM_LIB} -ffast-math  `llvm-config --cxxflags --ldflags --libs core executionengine  mcjit interpreter analysis native bitwriter --system-libs`   -rdynamic -llzma -std=c++14 -L$(PAPI_ROOT)/lib -lpapi
LDFLAGS= -L${LLVM_LIB} -ffast-math  `llvm-config --cxxflags --ldflags --libs core executionengine  mcjit interpreter analysis native bitwriter --system-libs`   -rdynamic -llzma -std=c++14 -L$(PAPI_ROOT)/lib 
LINK_DIR=build/ 

AR=ar
#AR=xiar
#AR=llvm-ar
BUILD_PATH=.

include Makefile.in
