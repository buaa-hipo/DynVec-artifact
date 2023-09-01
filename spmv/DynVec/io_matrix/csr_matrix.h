#ifndef CSR_MATRIX_H
#define CSR_MATRIX_H

#include <string>
template<typename FPTYPE>
struct csr_sparse_matrix {
    int row_num;
    int column_num;
    int data_num;
    int * row_ptr;
    int * column_ptr;
    FPTYPE * data_ptr;
};
template <typename FPTYPE>
using csrSparseMatrix = csr_sparse_matrix<FPTYPE>;

template <typename FPTYPE>
using csrSparseMatrixPtr = csr_sparse_matrix<FPTYPE>*;

template <typename FPTYPE>
void csr_spmv(const csrSparseMatrixPtr<FPTYPE> matrix , const double * x , double * y ) ;
template<typename VALUE_TYPE>
csrSparseMatrixPtr<VALUE_TYPE> matrix_read_csr( const char * file_name ) ;

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
