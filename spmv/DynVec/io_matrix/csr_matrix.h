#ifndef CSR_MATRIX_H
#define CSR_MATRIX_H

#include <string>
typedef struct csr_sparse_matrix {
    int row_num;
    int column_num;
    int data_num;
    int * row_ptr;
    int * column_ptr;
    double * data_ptr;
}csrSparseMatrix , *csrSparseMatrixPtr;
void csr_spmv(const csrSparseMatrixPtr matrix , const double * x , double * y ) ;
csrSparseMatrixPtr matrix_read_csr( const char * file_name ) ;

typedef struct PageRankStructure {
    int nedges;
    int nnodes;
    int * n1;
    int * n2;
    float * nneibor;
    float * sum;
    float * rank;
}PageRankStructure , *PageRankStructurePtr;
PageRankStructurePtr pagerank_read(std::string file_name ) ;

#endif
