#include "csr_matrix.h"
#include "util.h"
void csr_spmv(const csrSparseMatrixPtr matrix , const double * x , double * y ) {
    const int column_num = matrix->column_num;
    const int row_num = matrix->row_num;
    const int data_num = matrix->data_num;
    const int * row_ptr = matrix->row_ptr;
    const int * column_ptr = matrix->column_ptr;
    const double * data_ptr = matrix->data_ptr;
    for( int i = 0 ; i < matrix->row_num ; i++ ) {
        const int data_begin = row_ptr[i] ;
        const int data_end = row_ptr[i+1] ;
        double res = y[i] ;
        for( int j = data_begin ; j < data_end ; j++ ) {
            res += data_ptr[ j ] *  x[ column_ptr[j] ];
        }
    	y[i] = res;
    }
}

