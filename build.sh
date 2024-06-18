#!/bin/bash
set -e
echo "Building for ARCH=AArch64"

CUR_DIR=`pwd`

echo "Setting environments ..."
source scripts/env.sh

LOG_DIR=$CUR_DIR/log/build_log
mkdir -p $LOG_DIR
LOG_FILE=$LOG_DIR/build-dynvec.log
echo "Building DynVec (log file $LOG_FILE) ..."
cd spmv/DynVec
{ set -e; make -j4 > $LOG_FILE 2>&1; } && \
        echo -e "\033[32m DynVec build successfully! \033[0m" || \
            (echo -e "\033[31m DynVec build fail! \033[0m"; exit -1)
cd $CUR_DIR
