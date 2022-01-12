#!/bin/bash
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
make -f Makefile.avx2 run > $RUNLOG_ROOT/motivation.log 2>&1
cd ${CUR_DIR}
# prepare data for evaluation run
sh scripts/run_tools/get_data.sh
SPMV_DATA=`pwd`/data/mx_data_less_than_1G/
PAGERANK_DATA=`pwd`/data/pagerank_data/
SPMV_LOG_ROOT=$RUNLOG_ROOT/spmv
PAGERANK_LOG_ROOT=$RUNLOG_ROOT/pagerank
mkdir -p $SPMV_LOG_ROOT
mkdir -p $PAGERANK_LOG_ROOT
# running SpMV evaluations
echo "--------- Evaluating SpMV ---------"
echo "Evaluating DynVec..."
sh scripts/run_tools/spmv/run_dynvec.sh $SPMV_DATA $SPMV_LOG_ROOT
echo "Evaluating CSR5..."
sh scripts/run_tools/spmv/run_csr5.sh $SPMV_DATA $SPMV_LOG_ROOT
echo "Evaluating MKL..."
sh scripts/run_tools/spmv/run_mkl.sh $SPMV_DATA $SPMV_LOG_ROOT

echo "--------- Evaluating Page Rank ---------"
echo "Evaluating Baseline..."
sh scripts/run_tools/pagerank/run_baseline.sh $PAGERANK_DATA $PAGERANK_LOG_ROOT
echo "Evaluating DynVec..."
sh scripts/run_tools/pagerank/run_dynvec.sh $PAGERANK_DATA $PAGERANK_LOG_ROOT
echo "Evaluating Conflict-Free..."
sh scripts/run_tools/pagerank/run_conflice_free.sh $PAGERANK_DATA $PAGERANK_LOG_ROOT
