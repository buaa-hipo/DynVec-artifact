export OMP_NUM_THREADS=1
DATA_ROOT=../mx_data_less_than_1G/
ls $DATA_ROOT|xargs -n 1  echo $DATA_ROOT|sed "s/ //g"|xargs -n 1 ./spmv >log_spmv_ourmethod
#ls ~/mx_data_less_than_1G/|xargs -n 1  echo ~/mx_data_less_than_1G/|sed "s/ //g"|xargs -n 1 /home/lcxaiw/Benchmark_SpMV_using_CSR5/CSR5_avx512/spmv >log_spmv_csr5
#source /opt/intel/parallel_studio_xe_2019/psxevars.sh 
#ls ~/mx_data_less_than_1G/|xargs -n 1  echo ~/mx_data_less_than_1G/|sed "s/ //g"|xargs -n 1 ~/icc_spmv_simd/run.sh >log_spmv_mkl
