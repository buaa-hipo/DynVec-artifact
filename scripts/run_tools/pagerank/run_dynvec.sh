export OMP_NUM_THREADS=1
DATA_ROOT=$1
LOG_ROOT=$2
LOG_FILE=$LOG_ROOT/log_dynvec
DATA="amazon0312.txt higgs-twitter.txt soc-pokec.txt"
echo "" > $LOG_FILE

cd intelligent-unroll

for x in $DATA
do
    echo "Running $x ..." | tee -a $LOG_FILE
    ./pagerank $DATA_ROOT/$x >> $LOG_FILE
done

cd ..
