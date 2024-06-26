# DynVec-artifact
Implementation and evaluation scripts for DynVec

## Pre-requests

- LLVM-16 (other version is not tested)
  - set LLVM\_BASE accordingly

## Code structure

- `DynVec-motivation` includes the `gather`/`scatter` micro-benchmarks for motivating performance evalutions;
- `scripts` includes the build/run tooling scripts to build/run DynVec;
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

## Running Motivation

To run motivation, we just need to change the dir to `DynVec-motivation` and run this commend:

```
make run
```

The results are stored in `DynVec-motivation/data`. 

And the visualization results is named motivation_exp.pdf.

## Tested platforms

- AArch64 machine with SVE (256-bits) enabled
