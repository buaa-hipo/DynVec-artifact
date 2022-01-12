export OMP_NUM_THREADS=1
DATA_ROOT=$1
LOG_ROOT=$2
LOG_FILE=$LOG_ROOT/log_spmv_csr5


ls $DATA_ROOT|xargs -n 1  echo $DATA_ROOT|sed "s/ //g" > data.txt
DATA=`cat data.txt`
echo "" > $LOG_FILE

cd spmv/Benchmark_SpMV_using_CSR5
# the binary is already copied out and we do not handle arch info here.
for x in $DATA
do
    ./spmv $x >> $LOG_FILE
done

cd ../..
