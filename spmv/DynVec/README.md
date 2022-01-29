# Prerequiement
- install llvm 8.0.0
    + Please follow instructions in https://github.com/Changxi-Liu/llvm-8.0.0

- add llvm install path ${LLVM_PROJ}/install/bin into PATH env variable

- modify Makefile LLVM_BASE to your llvm8.0.0 install path

## For Architectures that support AVX512 : KNL, Skylake
- modify ARCH in Makefile to ARCH=knl

## For Haswell

- modify ARCH in Makefile to ARCH=core-avx2


# Make this project
make -j32

# run
./spmv ${data}
