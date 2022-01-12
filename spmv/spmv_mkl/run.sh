export MKL_DYNAMIC=false
export MKL_NUM_THREADS=1
export OMP_NUM_THREADS=1

$DATA_PATH=${1}
ls $DATA_PATH | xargs -n 1 echo ${DATA_PATH} | sed "s/ //g" > file_list.txt
flist=`cat file_list.txt`
rm -rf log_spmv_mkl
for file in $flist
do
  echo FILE: $file
  ./app $file | tee -a log_spmv_mkl
done
