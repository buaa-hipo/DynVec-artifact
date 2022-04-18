#!/bin/bash
set -e
CUR_DIR=`pwd`
RUNLOG_ROOT="`pwd`/log/run_log"
mkdir -p $RUNLOG_ROOT
# load environments
echo "Setting environments ..."
source scripts/env.sh
# running motivation
echo "-------- Evaluating motivation -------"
echo "Log file: $RUNLOG_ROOT/motivation.log"
cd DynVec-motivation
make -f Makefile.avx2 run > $RUNLOG_ROOT/motivation.log 2>&1 && \
	echo -e "\033[32m Success! \033[0m" || \
		(echo -e "\033[31m Failed! \033[0m"; exit -1)
cd ${CUR_DIR}
# prepare data for evaluation run
echo "-------- Preparing data --------"
echo "Log file: $RUNLOG_ROOT/get_data.log"
sh scripts/run_tools/get_data.sh > $RUNLOG_ROOT/get_data.log 2>&1 && \
	echo -e "\033[32m Success! \033[0m" || \
		(echo -e "\033[31m Failed! \033[0m"; exit -1)

SPMV_DATA=`pwd`/data/mx_data_less_than_1G/
SPMV_LOG_ROOT=$RUNLOG_ROOT/spmv
mkdir -p $SPMV_LOG_ROOT
# running SpMV evaluations
echo "--------- Evaluating SpMV ---------"
echo "Evaluating DynVec..."
{ set -e; sh scripts/run_tools/spmv/run_dynvec.sh $SPMV_DATA $SPMV_LOG_ROOT; } > $SPMV_LOG_ROOT/run.log 2>&1 && \
	echo -e "\033[32m Success! \033[0m" || \
		(echo -e "\033[31m Failed! \033[0m"; exit -1)
echo "Evaluating CSR5..."
{ set -e; sh scripts/run_tools/spmv/run_csr5.sh $SPMV_DATA $SPMV_LOG_ROOT; } > $SPMV_LOG_ROOT/run.log 2>&1 && \
	echo -e "\033[32m Success! \033[0m" || \
		(echo -e "\033[31m Failed! \033[0m"; exit -1)
echo "Evaluating MKL..."
{ set -e; sh scripts/run_tools/spmv/run_mkl.sh $SPMV_DATA $SPMV_LOG_ROOT; } > $SPMV_LOG_ROOT/run.log 2>&1 && \
	echo -e "\033[32m Success! \033[0m" || \
		(echo -e "\033[31m Failed! \033[0m"; exit -1)

echo "Evaluating CVR..."
result=`lscpu | grep -i -o avx512`
if [ "$result" != "" ]; then
	{ set -e; sh scripts/run_tools/spmv/run_cvr.sh $SPMV_DATA $SPMV_LOG_ROOT; } > $SPMV_LOG_ROOT/run.log 2>&1 && \
		echo -e "\033[32m Success! \033[0m" || \
			(echo -e "\033[31m Failed! \033[0m"; exit -1)
else
    echo "CVR must be run on machines with AVX512 support, skipping..."
fi
