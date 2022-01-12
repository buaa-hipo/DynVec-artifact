export OMP_NUM_THREADS=1
DATA_ROOT=$1
LOG_ROOT=$2
LOG_FILE=$LOG_ROOT/log_conflice_free
DATA="amazon0312.txt higgs-twitter.txt soc-pokec.txt"
echo "" > $LOG_FILE

CUR_DIR=`pwd`

cd page_rank/conflict_free/graphs/page_rank

for x in $DATA
do
    echo "Running $x ..." | tee -a $LOG_FILE
    ./page_rank_invec $DATA_ROOT/$x >> $LOG_FILE
done

cd $CUR_DIR
