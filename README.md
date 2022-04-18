# DynVec-artifact
Implementation and evaluation scripts for DynVec

## Pre-requests

- Intel oneAPI 2021.1 (other version is not tested)

## Code structure

- `DynVec-motivation` includes the `gather`/`scatter` micro-benchmarks for motivating performance evalutions;
- `scripts` includes the build/run tooling scripts to build/run all artifacts including ICC, DynVec, MKL, CSR5, and DynVec motivtions;
- `spmv/Benchmark_SpMV_using_CSR5` is the implementations for CSR5 (as a submodule);
- `spmv/CVR` is the implementations for CVR on AVX512 platforms (as a submodule);
- `spmv/spmv_mkl` contains the source codes to implement SpMV with MKL;
- `spmv/DynVec` contains the DynVec source codes.
- `data.tar.gz` can be downloaded from [here](https://1drv.ms/u/s!AsGTYrgSALbmo-cjP_ukT2rtoAp2Iw?e=7ufdpw) and it will be used in `run.sh` for SpMV evaluation.

## Build
First, we need to clone the entire repo with compared CSR5 submodule:

```
git clone --recursive https://github.com/buaa-hipo/DynVec-artifact.git
```

After cloning the repo, use the build script to build the dependency, DynVec, CSR5 and motivation benchmarks:

```
./build.sh <knl,avx2,avx512>
```

Note that the build script will download and compiles llvm-8.0.0, which may take some time to finish.

## Running SpMV

To run spmv with prepared dataset (`data.tar.gz` in release), we just need to use the run script:

```
./run.sh
```

The results are stored in `log/run_log/spmv/log_spmv_*`.

## Tested platforms

- Intel Xeon E5-2620 v3 CPU (Haswell, build with `avx2`)
- Intel Xeon E5-2680 v4 CPU (Broadwell, build with `avx2`)
- Intel Xeon Gold 6126 CPU (Skylake, build with `avx512`)
- Intel Xeon Platinum 9242 CPU (Cascade, build with `avx512`)
- Intel Phi 7210 CPU (KNL, build with `knl`)
