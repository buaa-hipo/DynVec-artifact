#ifndef CSR_MATRIX_H
#define CSR_MATRIX_H
typedef struct csr_sparse_matrix {
    int row_num;
    int column_num;
    int data_num;
    int * row_ptr;
    int * column_ptr;
    double * data_ptr;
}csr_sparse_matrix , *csrSparseMatrixPtr;
void csr_spmv(const csrSparseMatrixPtr matrix , const double * x , double * y ) ;
csrSparseMatrixPtr matrix_read_csr( const char * file_name ) ;
#endif
