#include "csr_matrix.h"
#include <stdlib.h>
#include "csr_analyse.h"
#include "csr_spmv_simd.h"

#include "test_utils.h"

extern "C" {
    #include "util.h"
}
#include "Timers.hpp"
#include <mkl.h>
#define MAX 10
template<typename T>
bool check_equal(const T * v1, const T * v2, const int num ) {
    bool flag = true;
    for( int i = 0 ; i < num ; i++ ) {
        if( (v1[i]-v2[i]) > 1e-3 || (v2[i]-v1[i]) > 1e-3 ) {
            flag = false;
           std::cout<< i<< " "  << v1[i]  << " "<< v2[i]<<"\n";
        }
    }
    if(flag) 
        std::cout<<"Correct"<<std::endl;
    else 
        std::cout<<"False"<<std::endl;
    return flag;
}

int main( int argv , char ** argc)  {
    bool with_papi = false;
    if(argv >= 3) {
        with_papi = (atoi(argc[2])!=0);
    }
    csrSparseMatrixPtr matrix = matrix_read_csr(argc[1]);
    int row_num = matrix->row_num;
    int column_num = matrix->column_num;
    const int data_num = matrix->data_num;
    const int * row_ptr = matrix->row_ptr;
    const DTYPE * data_ptr = matrix->data_ptr;
    const int * column_ptr = matrix->column_ptr;
    double * x =  SIMPLE_MALLOC( double , column_num );
    double * y = SIMPLE_MALLOC( double , row_num ) ;

    double * y_timer = SIMPLE_MALLOC( double , row_num ) ;
    double * y_ref = SIMPLE_MALLOC( double , row_num );
    for( int i = 0 ; i < row_num ; i++ ) {
        y[i] = 1;
        y_ref[i] = 1;
        y_timer[i] = 1;
    }
    for( int i = 0 ; i < column_num ; i++ ) {
        x[i] = 1;
    }
    const int detection_record_num = data_num / DetectionRecord_SIZE;
    char trans[] = {'n'};
    int size[] = {row_num,row_num};
    double alpha = 1;
    double beta = 1;
    char matdescra[6] = { 'G', 'x', 'x', 'C', 'x', 'x'};

    mkl_dcsrmv ( trans , &row_num, &column_num , &alpha,matdescra,    data_ptr, column_ptr, row_ptr,&row_ptr[1], x, &beta,y);
    Timer::startTimer("mkl");
    for( int i = 0 ; i < 1000; i++ )
    mkl_dcsrmv ( trans , &row_num, &column_num , &alpha,matdescra,    data_ptr, column_ptr, row_ptr,&row_ptr[1], x, &beta,y_timer);
    Timer::endTimer("mkl");

    Timer::printTimer("mkl");

    Timer::printGFLOPS("mkl", (long long)data_num * 2 * 1000);

    if(with_papi) {
       papi_init();
    } else {
       printf("PAPI profiling is disabled.\n");
    }
    std::string base_name(argc[1]);
    std::vector<std::string> path = splitpath(base_name);
    base_name = remove_extension(path.back());
    std::string mkl_name = base_name + std::string(".mkl");
    PAPI_TEST_EVAL(50, 1000, data_num*2*1000, mkl_name.c_str(), mkl_dcsrmv(trans , &row_num, &column_num, &alpha,matdescra, data_ptr, column_ptr, row_ptr, &row_ptr[1], x, &beta, y_timer) );
    if(with_papi) {
        papi_fini();
    }

//    DetectionRecordPtr detection_record_ptr = analyse(row_ptr,data_num,row_num);
//    csr_spmv_simd( data_ptr , column_ptr , x , y_ref , detection_record_ptr , detection_record_num );

    csr_spmv( matrix , x , y_ref );
    check_equal( y,y_ref,row_num );    
//    my_printf_double( varience_double( y , y_ref , row_num  ) );


}
