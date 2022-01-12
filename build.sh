#!/bin/bash
set -e

ARCH=${1}
[ -z $ARCH ] && (echo "Usage: ./build.sh <arch=knl,avx512,avx2>"; exit -1)

if ([ "$ARCH" == "avx512" ] || [ "$ARCH" == "knl" ] || [ "$ARCH" == "avx2" ]); then
   echo "Building for ARCH=$ARCH"
else
  (echo "Unknown ARCH: $ARCH. Should be one of: avx512, knl, avx2"; exit -1)
fi

CUR_DIR=`pwd`

echo "Checking environment ..."
sh scripts/build_tools/env_check.sh && \
        echo -e "\033[32m environtment check successfully! \033[0m" || \
            (echo -e "\033[31m environment check fail! \033[0m"; exit -1)

mkdir -p log/build_log/
LOG_FILE=`pwd`/log/build_log/build-depancy.log
echo "Building DynVec Dependancies (log file $LOG_FILE) ..."
sh scripts/build_tools/build-dependancy.sh $LOG_FILE && \
        echo -e "\033[32m Dependency build successfully! \033[0m" || \
            (echo -e "\033[31m Dependancy build fail! \033[0m"; exit -1)

LOG_FILE=`pwd`/log/build_log/build-dynvec.log
echo "Building DynVec (log file $LOG_FILE) ..."
cd intelligent-unroll
{ set -e; make -j > $LOG_FILE 2>&1; } && \
        echo -e "\033[32m DynVec build successfully! \033[0m" || \
            (echo -e "\033[31m DynVec build fail! \033[0m"; exit -1)
cd $CUR_DIR

echo "Preparing artifacts for SpMV ..."

LOG_FILE=`pwd`/log/build_log/build-csr5.log
echo "Building CSR5 (log file $LOG_FILE) ..."
cd spmv/Benchmark_SpMV_using_CSR5

if [ "$ARCH" == "avx512" ]; then
  cd CSR5_avx512
  echo "Patching for skylake ..."
  patch main.cpp < $CUR_DIR/scripts/build_tools/patches/CSR5_skylake_avx512.patch
  patch Makefile < $CUR_DIR/scripts/build_tools/patches/CSR5_skylake_avx512_make.patch
elif [ "$ARCH" == "knl" ]; then
  cd CSR5_avx512
elif [ "$ARCH" == "avx2" ]; then
  cd CSR5_avx2
fi

{ set -e; make -j > $LOG_FILE 2>&1; } && \
        echo -e "\033[32m CSR5 build successfully! \033[0m" || \
            (echo -e "\033[31m CSR5 build fail! \033[0m"; exit -1)
cd $CUR_DIR

LOG_FILE=`pwd`/log/build_log/build-mkl.log
echo "Building MKL SpMV benchmarks (log file $LOG_FILE)"
cd spmv/spmv_mkl
{ set -e; make -j > $LOG_FILE 2>&1; } && \
        echo -e "\033[32m MKL SpMV benchmarks build successfully! \033[0m" || \
            (echo -e "\033[31m MKL SpMV benchmarks build fail! \033[0m"; exit -1)
cd $CUR_DIR

echo "Preparing artifacts for Page Rank ..."

LOG_FILE=`pwd`/log/build_log/build-pagerank-icc.log
echo "Building Page Rank basic implementation (log file $LOG_FILE)"
cd page_rank/page_rank_base
{ set -e; make -j > $LOG_FILE 2>&1; } && \
        echo -e "\033[32m Page Rank basic implementation build successfully! \033[0m" || \
            (echo -e "\033[31m Page Rank basic implementation build fail! \033[0m"; exit -1)
cd $CUR_DIR

echo "Building Conflict-Free Page Rank implemenation (log file $LOG_FILE)"
cd page_rank/conflict_free/graphs/page_rank
if [ "$ARCH" == "avx512" ]; then
  echo "+ Add Patching for makefile ..."
  patch Makefile < ${CUR_DIR}/scripts/build_tools/patches/conflict_free_make.patch
fi
echo "+ Building ..."
{ set -e; make -j > $LOG_FILE 2>&1; } && \
        echo -e "\033[32m Conflict-Free Page Rank implementation build successfully! \033[0m" || \
            (echo -e "\033[31m Conflict-Free Page Rank implementation build fail! \033[0m"; exit -1)
cd ${CUR_DIR}

LOG_FILE=`pwd`/log/build_log/build-motivation.log
echo "Building DynVec Motivation benchmarks (log file $LOG_FILE) ..."
cd DynVec-motivation
if [ "$ARCH" == "avx2" ]; then
  { set -e; make -f Makefile.avx2 > $LOG_FILE 2>&1; } && \
        echo -e "\033[32m DynVec Motivation benchmarks build successfully! \033[0m" || \
            (echo -e "\033[31m DynVec Motivation benchmarks build fail! \033[0m"; exit -1)
else
  { set -e; make -f Makefile.avx512 > $LOG_FILE 2>&1; } && \
        echo -e "\033[32m DynVec Motivation benchmarks build successfully! \033[0m" || \
            (echo -e "\033[31m DynVec Motivation benchmarks build fail! \033[0m"; exit -1)
fi
cd ${CUR_DIR}
