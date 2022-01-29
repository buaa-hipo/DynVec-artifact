/**
 * LLVM equivalent of:
 *
 * int sum(int a, int b) {
 *     return a + b;
 * }
 */
#include "Timers.hpp"
#include "intelligent_unroll.hpp"
//#include "dynvec.h"
#include "util.h"
#include "csr_matrix.h"

#include "test_utils.h"

#define PRINTINT(x) do {    \
                    printf( #x" %d\n" , (x));fflush(stdout); \
                        } while(0)
void spmv_local(double * y_ptr,const double * x_ptr,const double * data_ptr, const int * column_ptr, const int * row_ptr, const int row_num ) {
    for (int i = 0 ; i < row_num ; i++ ) {
        double sum = y_ptr[i];
        for (int j = row_ptr[i]; j < row_ptr[i+1] ; j++ ) {
            sum += x_ptr[column_ptr[j]] * data_ptr[j];
        }
        y_ptr[i] += sum;
    }
}

void init_vec(double * dence_vec_ptr, const int data_num , const double data, const bool change = false ) {
    if( change ) {
        for( int i = 0 ; i < data_num ; i++ ) {
            dence_vec_ptr[i] = i;
        }
    } else {
        for( int i = 0 ; i < data_num ; i++ ) {
            dence_vec_ptr[i] = data ;

        }
    }
}
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

template<typename T>
void print_vec( T * data_ptr, const int num ) {
    for( int i = 0 ; i < num ; i++ ) {
        std::cout<< data_ptr[i] << " ";
    }
    std::cout<<std::endl;
}

const int max_bits_ = sizeof(double) * ByteSize;
#ifdef __AVX512CD__
    const int vector_bits = 512;

    const int vector_nums = vector_bits / max_bits_;
#else 
    #ifdef __AVX2__
    const int vector_bits = 256;

    const int vector_nums = vector_bits / max_bits_;
    #else
    const int vector_nums = -1;
    #error "Unsupported architetures";
    #endif
#endif

using FuncType = int(*)( double*,int*,int*,double*,double*);

void spmv_dynvec(FuncType func, double* y_array, int* row_ptr_all, int* column_ptr, double* x_array, double* data_ptr, int data_num) {
    func( y_array,row_ptr_all, column_ptr, x_array,data_ptr );
    for( int i = data_num / vector_nums * vector_nums ; i < data_num ; i++ ) {
        y_array[ row_ptr_all[ i ] ] += x_array[column_ptr[i]] * data_ptr[ i ];
    }
}

//#define LITTEL_CASE2
int main( int argc , char const * argv[] ) {
    bool with_papi = false;
    if(argc >= 3) {
        with_papi = (atoi(argv[2])!=0);
    }
    #ifdef LITTEL_CASE
        csrSparseMatrix sparseMatrix = little_test();
        csrSparseMatrixPtr  sparseMatrixPtr = &sparseMatrix;
    #elif defined LITTEL_CASE2

        csrSparseMatrix sparseMatrix = little_test2(1024 ,1024);
        csrSparseMatrixPtr  sparseMatrixPtr = &sparseMatrix;
    #else
        if(argc <= 1 ) {
            printf("Erro: You need to modify a file to read\n");
            return 0;
        }
        csrSparseMatrixPtr sparseMatrixPtr = matrix_read_csr( argv[1]);
        if(sparseMatrixPtr==NULL) {
            printf("Error: sparse matrix not supported\n");
            return 0;
        }
    #endif
  
    
    double * data_ptr = sparseMatrixPtr->data_ptr;
    int * column_ptr = sparseMatrixPtr->column_ptr;
    int * row_ptr = sparseMatrixPtr->row_ptr;

    const int data_num = sparseMatrixPtr->data_num;
    const int row_num = sparseMatrixPtr->row_num;
    const int column_num = sparseMatrixPtr->column_num;
    double * x_array = SIMPLE_MALLOC( double , column_num );
    double * y_array = SIMPLE_MALLOC( double, row_num );
    double * y_array_bak = SIMPLE_MALLOC( double , row_num );
    
    double * y_array_time = SIMPLE_MALLOC( double, row_num );
//    init_vec(x_array,column_num,1);

    init_vec( x_array, column_num , 1 ,true);
//    init_vec( x_array, column_num , 1 ,true);

    //init_vec(x_array,column_num,1);
    init_vec( y_array, row_num , 0 );
    init_vec( y_array_bak, row_num , 0 );

    init_vec( y_array_time, row_num , 0 );
    int * row_ptr_all = SIMPLE_MALLOC( int, data_num );
    for(int row_i = 0 ; row_i < row_num; row_i++) {
        int begin = row_ptr[row_i];
        int end = row_ptr[row_i+1];
        for( int j = begin ; j < end ; j++ ) {
            row_ptr_all[j] = row_i;
        }
    }
    std::string spmv_str = 
    "input: int * row_ptr,   \
            int * column_ptr,\
            double * x_array,\
            double * data_ptr\
     output:double * y_array \
     lambda i : \
            y_array[ row_ptr[i] ] += data_ptr[i] \
            * x_array[column_ptr[i]]\
            ";
    std::map<std::string,void*> name2ptr_map;
    name2ptr_map[ "row_ptr" ] = row_ptr_all;

    name2ptr_map[ "column_ptr" ] = column_ptr;
    name2ptr_map[ "x_array" ] = x_array;
    name2ptr_map[ "data_ptr" ] = data_ptr;
    name2ptr_map[ "y_array" ] = y_array;

    LOG(INFO) << data_num/vector_nums;

    Timer::startTimer("compile");
    uint64_t func_int64 = compiler( spmv_str,name2ptr_map,data_num/vector_nums );

    Timer::endTimer("compile");

    Timer::printTimer("llvmcompile");
    Timer::printTimer("compile");
    // using FuncType = int(*)( double*,int*,int*,double*,double*);
    // FuncType func = (FuncType)(func_int64);
    // Timer::startTimer("aot");
    //      spmv_local( y_array_bak, x_array,data_ptr,column_ptr,row_ptr,row_num );

    // Timer::endTimer("aot");

    // Timer::printTimer("aot");

    if(with_papi) {
       papi_init();
    } else {
       printf("PAPI profiling is disabled.\n");
    }


    int flops = data_num * 2;
    std::string base_name(argv[1]);
    std::vector<std::string> path = splitpath(base_name);
    base_name = remove_extension(path.back());
    std::string aot_name = base_name + std::string(".aot");
    spmv_local( y_array_bak, x_array,data_ptr,column_ptr,row_ptr,row_num );
    PAPI_TEST_EVAL(50, 1000, flops, aot_name.c_str(), spmv_local( y_array_time, x_array,data_ptr,column_ptr,row_ptr,row_num ) );

    std::string jit_name = base_name + std::string(".jit");
    spmv_dynvec((FuncType)func_int64, y_array, row_ptr_all, column_ptr, x_array, data_ptr, data_num);
    PAPI_TEST_EVAL(50, 1000, flops, jit_name.c_str(), spmv_dynvec((FuncType)func_int64, y_array_time, row_ptr_all, column_ptr, x_array, data_ptr, data_num) );

    if(with_papi) {
        papi_fini();
    }

    
//     func( y_array,row_ptr_all, column_ptr, x_array,data_ptr );
//     LOG(INFO) << data_num / vector_nums * vector_nums;
//     for( int i = (data_num / vector_nums * vector_nums) ; i < data_num ; i++ ) {
//         y_array[ row_ptr_all[ i ] ] += x_array[column_ptr[i]] * data_ptr[ i ];
//     }
// #define WARM_TIME 50
//      for( int i = 0 ; i < WARM_TIME ; i++ ) {
//         func( y_array_time,row_ptr_all, column_ptr, x_array,data_ptr );
//         for( int i = data_num / vector_nums * vector_nums ; i < data_num ; i++ ) {

//             y_array_time[ row_ptr_all[ i ] ] += x_array[column_ptr[i]] * data_ptr[ i ];
//         }
//      }

// #define TIMES 1000
//     Timer::startTimer("jit");
//      for( int i = 0 ; i < TIMES ; i++ ){
//         func( y_array_time,row_ptr_all, column_ptr, x_array,data_ptr );
//         for( int i = data_num / vector_nums * vector_nums ; i < data_num ; i++ ) {

//             y_array_time[ row_ptr_all[ i ] ] += x_array[column_ptr[i]] * data_ptr[ i ];
//         }

//      }

//     Timer::endTimer("jit");
//     Timer::printTimer("jit",TIMES);
//     Timer::printGFLOPS( "jit", data_num * 2 , TIMES );
    if(!check_equal( y_array_bak, y_array, row_num )) {
        return 1;
    }
    return 0;
}
