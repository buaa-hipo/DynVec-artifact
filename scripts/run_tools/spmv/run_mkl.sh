export MKL_DYNAMIC=false
export MKL_NUM_THREADS=1
export OMP_NUM_THREADS=1
DATA_ROOT=$1
LOG_ROOT=$2
LOG_FILE=$LOG_ROOT/log_spmv_mkl


ls $DATA_ROOT|xargs -n 1  echo $DATA_ROOT|sed "s/ //g" > data.txt
DATA=`cat data.txt`
echo "" > $LOG_FILE

cd spmv/spmv_mkl

for x in $DATA
do
    { ./app $x; } >> $LOG_FILE 2>> ${LOG_FILE}.err
done

cd ../..
