#include <stdlib.h>
#include <stdio.h>

#include <fstream>
#include <iostream>
#include "csr_matrix.h"
#define BUFFER_SIZE 1024
#include "mmio.h"
#include "util.h"
#include "mm_malloc.h"
PageRankStructurePtr pagerank_read(std::string file_name ) {
    PageRankStructurePtr page_rank_structure_ptr = (PageRankStructurePtr)malloc(sizeof(PageRankStructure));
    int nnodes;
    int nedges;
    std::ifstream fin( file_name);
    fin >> nnodes >> nedges;
    int *n1 = (int *)_mm_malloc(sizeof(int)*nedges, 64);
    int *n2 = (int *)_mm_malloc(sizeof(int)*nedges, 64);
    int *nneibor_int = (int *)_mm_malloc(sizeof(int)*nnodes, 64);

    float *nneibor = (float *)_mm_malloc(sizeof(float)*nnodes, 64);
    for(int i=0;i<nnodes;i++) {
        nneibor_int[i] = 0;
    }


    int t = 0;

    int w;
    while(fin >> n1[t] >> n2[t] >> w) {
        nneibor_int[n1[t]] += 1;
        t++;
    }

    float *rank = (float *)_mm_malloc(sizeof(float)*nnodes, 64);
    float *sum = (float *)_mm_malloc(sizeof(float)*nnodes, 64);

    for(int i=0;i<nnodes;i++) {
	    rank[i] = 1.0;
	    sum[i] = 0.0;
    }
    for( int i = 0 ; i < nnodes ; i++ )
        nneibor[i] = (float)nneibor_int[i];
    page_rank_structure_ptr->nedges = nedges;
    page_rank_structure_ptr->nnodes = nnodes; 

    page_rank_structure_ptr->n1 = n1;
    page_rank_structure_ptr->n2 = n2;
    page_rank_structure_ptr->nneibor = nneibor;
    page_rank_structure_ptr->sum = sum;
    page_rank_structure_ptr->rank = rank;
     
    return page_rank_structure_ptr;
}
void csr_spmv(const csrSparseMatrixPtr matrix , const double * x , double * y ) {
    //const int column_num = matrix->column_num;
    //const int row_num = matrix->row_num;
    //const int data_num = matrix->data_num;
    const int * row_ptr = matrix->row_ptr;
    const int * column_ptr = matrix->column_ptr;
    const double * data_ptr = matrix->data_ptr;
    for( int i = 0 ; i < matrix->row_num ; i++ ) {
        const int data_begin = row_ptr[i] ;
        const int data_end = row_ptr[i+1] ;
        y[i] = 0 ;
        for( int j = data_begin ; j < data_end ; j++ ) {
            y[i] += data_ptr[ j ] *  x[ column_ptr[j] ];
        }
    }
}

#define VALUE_TYPE double
csrSparseMatrixPtr matrix_read_csr( const char * filename ) {
    csrSparseMatrixPtr res;

   res = (csrSparseMatrixPtr) malloc( sizeof( csr_sparse_matrix ) );

    int m, n, nnzA;
    int *csrRowPtrA;
    int *csrColIdxA;
    VALUE_TYPE *csrValA;

    //ex: ./spmv webbase-1M.mtx
    //int argi = 1;

    printf( "-------------- %s --------------\n",filename) ;

    // read matrix from mtx file
    int ret_code;
    MM_typecode matcode;
    FILE *f;

    int nnzA_mtx_report;
    int isInteger = 0, isReal = 0, isPattern = 0, isSymmetric = 0;

    // load matrix
    if ((f = fopen(filename, "r")) == NULL)
        return NULL;

    if (mm_read_banner(f, &matcode) != 0)
    {
        printf( "Could not process Matrix Market banner.\n" );
        return NULL;
    }

    if ( mm_is_complex( matcode ) )
    {
        printf("Sorry, data type 'COMPLEX' is not supported. " );
        return NULL;
    }

    if ( mm_is_pattern( matcode ) )  { isPattern = 1; /*cout << "type = Pattern" << endl;*/ }
    if ( mm_is_real ( matcode) )     { isReal = 1; /*cout << "type = real" << endl;*/ }
    if ( mm_is_integer ( matcode ) ) { isInteger = 1; /*cout << "type = integer" << endl;*/ }

    /* find out size of sparse matrix .... */
    ret_code = mm_read_mtx_crd_size(f, &m, &n, &nnzA_mtx_report);
    if (ret_code != 0)
        return NULL;

    if ( mm_is_symmetric( matcode ) || mm_is_hermitian( matcode ) )
    {
        isSymmetric = 1;
        //cout << "symmetric = true" << endl;
    }
    else
    {
        //cout << "symmetric = false" << endl;
    }

//    int * addr = ( int * ) malloc( ( m+1 + nnzA_mtx_report * 2 ) * sizeof(int)  );
    
//    int *csrRowPtrA_counter = addr;
//    memset(csrRowPtrA_counter, 0, (m+1) * sizeof(int));
//    int *csrRowIdxA_tmp = addr + m + 1;
//    int *csrColIdxA_tmp = addr + nnzA_mtx_report;

    int *csrRowPtrA_counter = (int *)_mm_malloc((m+1) * sizeof(int),ANONYMOUSLIB_X86_CACHELINE);
    memset(csrRowPtrA_counter, 0, (m+1) * sizeof(int));
    int *csrRowIdxA_tmp = (int *)_mm_malloc(nnzA_mtx_report * sizeof(int),ANONYMOUSLIB_X86_CACHELINE);
    int *csrColIdxA_tmp = (int *)_mm_malloc(nnzA_mtx_report * sizeof(int),ANONYMOUSLIB_X86_CACHELINE);

    VALUE_TYPE *csrValA_tmp    = (VALUE_TYPE *)_mm_malloc(nnzA_mtx_report * sizeof(VALUE_TYPE),ANONYMOUSLIB_X86_CACHELINE);

    /* NOTE: when reading in doubles, ANSI C requires the use of the "l"  */
    /*   specifier as in "%lg", "%lf", "%le", otherwise errors will occur */
    /*  (ANSI C X3.159-1989, Sec. 4.9.6.2, p. 136 lines 13-15)            */
    for (int i = 0; i < nnzA_mtx_report; i++)
    {
        int idxi, idxj;
        double fval;
        int ival;

        if (isReal)
            fscanf(f, "%d %d %lg\n", &idxi, &idxj, &fval);
        else if (isInteger)
        {
            fscanf(f, "%d %d %d\n", &idxi, &idxj, &ival);
            fval = ival;
        }
        else if (isPattern)
        {
            fscanf(f, "%d %d\n", &idxi, &idxj);
            fval = 1.0;
        }

        // adjust from 1-based to 0-based
        idxi--;
        idxj--;
        csrRowPtrA_counter[idxi]++;
        csrRowIdxA_tmp[i] = idxi;
        csrColIdxA_tmp[i] = idxj;
        csrValA_tmp[i] = fval;
//        my_printf_int(idxi);
    }
        if (f != stdin)
        fclose(f);

    if (isSymmetric)
    {
        for (int i = 0; i < nnzA_mtx_report; i++)
        {
            if (csrRowIdxA_tmp[i] != csrColIdxA_tmp[i])
                csrRowPtrA_counter[csrColIdxA_tmp[i]]++;
        }
    }

    // exclusive scan for csrRowPtrA_counter
    int old_val, new_val;

    old_val = csrRowPtrA_counter[0];
    csrRowPtrA_counter[0] = 0;
    for (int i = 1; i <= m; i++)
    {
        new_val = csrRowPtrA_counter[i];
        csrRowPtrA_counter[i] = old_val + csrRowPtrA_counter[i-1];
        old_val = new_val;
    }
    nnzA = csrRowPtrA_counter[m];
    csrRowPtrA = (int *)_mm_malloc((m+1) * sizeof(int),ANONYMOUSLIB_X86_CACHELINE);
    memcpy(csrRowPtrA, csrRowPtrA_counter, (m+1) * sizeof(int));
    memset(csrRowPtrA_counter, 0, (m+1) * sizeof(int));

    csrColIdxA = (int *)_mm_malloc(nnzA * sizeof(int),ANONYMOUSLIB_X86_CACHELINE);
    csrValA    = (VALUE_TYPE *)_mm_malloc(nnzA * sizeof(VALUE_TYPE),ANONYMOUSLIB_X86_CACHELINE);


    if (isSymmetric)
    {
        for (int i = 0; i < nnzA_mtx_report; i++)
        {
            if (csrRowIdxA_tmp[i] != csrColIdxA_tmp[i])
            {
                int offset = csrRowPtrA[csrRowIdxA_tmp[i]] + csrRowPtrA_counter[csrRowIdxA_tmp[i]];
                csrColIdxA[offset] = csrColIdxA_tmp[i];
                csrValA[offset] = csrValA_tmp[i];
                csrRowPtrA_counter[csrRowIdxA_tmp[i]]++;

                offset = csrRowPtrA[csrColIdxA_tmp[i]] + csrRowPtrA_counter[csrColIdxA_tmp[i]];
                csrColIdxA[offset] = csrRowIdxA_tmp[i];
                csrValA[offset] = csrValA_tmp[i];
                csrRowPtrA_counter[csrColIdxA_tmp[i]]++;
            }
            else
            {
                int offset = csrRowPtrA[csrRowIdxA_tmp[i]] + csrRowPtrA_counter[csrRowIdxA_tmp[i]];
                csrColIdxA[offset] = csrColIdxA_tmp[i];
                csrValA[offset] = csrValA_tmp[i];
                csrRowPtrA_counter[csrRowIdxA_tmp[i]]++;
            }
        }
    }
    else
    {
        for (int i = 0; i < nnzA_mtx_report; i++)
        {
            int offset = csrRowPtrA[csrRowIdxA_tmp[i]] + csrRowPtrA_counter[csrRowIdxA_tmp[i]];
            csrColIdxA[offset] = csrColIdxA_tmp[i];
            csrValA[offset] = csrValA_tmp[i];
            csrRowPtrA_counter[csrRowIdxA_tmp[i]]++;
        }
    }

    // free tmp space
//    free(addr);
    free(csrRowPtrA_counter) ;
    free(csrRowIdxA_tmp);
    free(csrColIdxA_tmp);

    free(csrValA_tmp);



    // set csrValA to 1, easy for checking floating-point results
    for (int i = 0; i < nnzA; i++) {
//#ifndef DEBUG
        csrValA[i] = rand() % 10;

//#else
        //csrValA[i] = 1;
//#endif
    }

    printf( " ( m = %d , n = %d  ) nnz = %d \n" ,m , n , nnzA );

    //double gb = getB<int, VALUE_TYPE>(m, nnzA);
    //double gflop = getFLOP<int>(nnzA);
/*    
    for( int i = 0 ; i < 10 ; i++ ) {
      printf("%d %d %d\n",i, csrRowPtrA[i],csrRowPtrA[i+1] - csrRowPtrA[i]);
    }

    exit(1);*/
    res->row_ptr = csrRowPtrA;
    res->column_ptr = csrColIdxA;
    res->data_ptr = csrValA;
    res->row_num = m ;
    res->column_num = n;
    res->data_num = nnzA;

    return res;
}



