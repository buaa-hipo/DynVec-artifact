# DynVec-artifact
Implementation and evaluation scripts for DynVec

## Pre-requests

- LLVM-16 (other version is not tested)

## Code structure

- `DynVec-motivation` includes the `gather`/`scatter` micro-benchmarks for motivating performance evalutions;
- `scripts` includes the build/run tooling scripts to build/run all artifacts including ICC, DynVec, MKL, CSR5, and DynVec motivtions;
- `spmv/DynVec` contains the DynVec source codes.
- `data.tar.gz` can be downloaded from [here](https://1drv.ms/u/s!AsGTYrgSALbmo-cjP_ukT2rtoAp2Iw?e=7ufdpw) and it will be used in `run.sh` for SpMV evaluation.

## Build
First, we need to clone the repo:

```
git clone --recursive https://github.com/buaa-hipo/DynVec-artifact.git
```

After cloning the repo, use the build script to install DynVec:

```
./build.sh
```

## Running SpMV

To run spmv with prepared dataset (`data.tar.gz` in release), we just need to use the run script:

```
./run.sh
```

The results are stored in `log/run_log/spmv/log_spmv_*`.

## Tested platforms

- Huawei Kunpeng cloud server kC2 with SVE (256-bits) enabled
