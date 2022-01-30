export OMP_NUM_THREADS=1
DATA_ROOT=$1
LOG_ROOT=$2
LOG_FILE=$LOG_ROOT/log_spmv_dynvec


ls $DATA_ROOT|xargs -n 1  echo $DATA_ROOT|sed "s/ //g" > data.txt
DATA=`cat data.txt`
echo "" > $LOG_FILE

cd spmv/DynVec/

for x in $DATA
do
    ./spmv $x >> $LOG_FILE
done

cd ../..
