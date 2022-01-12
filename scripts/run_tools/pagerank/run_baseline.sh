export OMP_NUM_THREADS=1
DATA_ROOT=$1
LOG_ROOT=$2
LOG_FILE=$LOG_ROOT/log_baseline
DATA="amazon0312.txt higgs-twitter.txt soc-pokec.txt"
echo "" > $LOG_FILE

cd page_rank/page_rank_base

for x in $DATA
do
    echo "Running $x ..." | tee -a $LOG_FILE
    ./sum $DATA_ROOT/$x >> $LOG_FILE
done

cd ../..
