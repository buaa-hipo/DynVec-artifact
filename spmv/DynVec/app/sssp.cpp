/**
 * LLVM equivalent of:
 *
 * int sum(int a, int b) {
 *     return a + b;
 * }
 */
#include <set>
#include "Timers.hpp"
#include "intelligent_unroll.hpp"
// #include "dynvec.h"
#include "util.h"
#include "csr_matrix.h"

#include "test_utils.h"

#define PRINTINT(x)              \
    do                           \
    {                            \
        printf(#x " %d\n", (x)); \
        fflush(stdout);          \
    } while (0)

void init_vec(DATATYPE *dence_vec_ptr, const int data_num, const DATATYPE data, const bool change = false)
{
    if (change)
    {
        for (int i = 0; i < data_num; i++)
        {
            dence_vec_ptr[i] = i;
        }
    }
    else
    {
        for (int i = 0; i < data_num; i++)
        {
            dence_vec_ptr[i] = data;
        }
    }
}
template <typename T>
bool check_equal(const T *v1, const T *v2, const int num)
{
    bool flag = true;
    for (int i = 0; i < num; i++)
    {
        if (!((v1[i] == 0 && v2[i] == 0) || (v1[i] == INFINITY && v2[i] == INFINITY)) && ((v1[i] - v2[i]) / v1[i] > 1e-3 || (v2[i] - v1[i]) / v1[i] > 1e-3))
        {
            flag = false;
            std::cout << i << " " << v1[i] << " " << v2[i] << "\n";
        }
    }
    if (flag)
        std::cout << "Correct" << std::endl;
    else
        std::cout << "False" << std::endl;
    return flag;
}

template <typename T>
void print_vec(T *data_ptr, const int num)
{
    for (int i = 0; i < num; i++)
    {
        std::cout << data_ptr[i] << " ";
    }
    std::cout << std::endl;
}

using FuncType = int (*)(double *, int *, int *, double *, double *);

const int max_bits_ = sizeof(double) * ByteSize;
#ifdef __AVX512CD__
const int vector_bits = 512;

const int vector_nums = vector_bits / max_bits_;
#elif defined __SVE512__
const int vector_bits = 512;

const int vector_nums = vector_bits / max_bits_;
#else
#ifdef __AVX2__
const int vector_bits = 256;

const int vector_nums = vector_bits / max_bits_;
#elif defined __SVE__
const int vector_bits = 256;

const int vector_nums = vector_bits / max_bits_;
#else
const int vector_nums = -1;
#error "Unsupported architetures";
#endif
#endif

// for sssp, m == n
inline void sssp_naive(double *y_array, int *row_ptr, int *column_ptr, double *x_array, double *data_ptr, int column_num, int row_num)
{
    bool flg = true;
    while (flg) {
        flg = false;
        for (int i = 0; i < row_num; i++)
        {
            double sum = INFINITY;
            for (int j = row_ptr[i]; j < row_ptr[i+1]; j++) {
                sum = min(sum, (x_array[column_ptr[j]] + data_ptr[j]));
            }
            y_array[i] = sum;
        }
        for (int i = 0; i < column_num; i++) {
            if (y_array[i] != INFINITY) {
                if(y_array[i] < x_array[i]) {
                    flg = true;
                    x_array[i] = y_array[i];
                }
            }
        }
    }
}

inline void sssp_dynvec(FuncType func, double *y_array, int *row_ptr_all, int *column_ptr, double *x_array, double *data_ptr, int data_num, int column_num)
{
    bool flg = true;
    while (flg) {
        flg = false;
        func(y_array, row_ptr_all, column_ptr, x_array, data_ptr);
        for( int i = data_num / vector_nums * vector_nums ; i < data_num ; i++ ) {
            if(y_array[row_ptr_all[i]] == 0)  y_array[row_ptr_all[i]] = INFINITY; // TODO: some initial value (0) need to be handle in statements codegen
            y_array[ row_ptr_all[ i ] ] = min(y_array[ row_ptr_all[ i ] ], x_array[column_ptr[i]] + data_ptr[ i ]);
        }
        for (int i = 0; i < column_num; i++) {
            if (y_array[i] != INFINITY) {
                if(y_array[i] < x_array[i]) {
                    flg = true;
                    x_array[i] = y_array[i];
                }
            }
        }
    }
}

// #define LITTEL_CASE2
int main(int argc, char const *argv[])
{
    bool with_papi = false;
    // bfs file src_vertex with_papi
    int src_vertex = atoi(argv[2]);
    if (argc >= 4)
    {
        with_papi = (atoi(argv[3]) != 0);
    }
#ifdef LITTEL_CASE
    csrSparseMatrix sparseMatrix = little_test();
    csrSparseMatrixPtr sparseMatrixPtr = &sparseMatrix;
#elif defined LITTEL_CASE2

    csrSparseMatrix sparseMatrix = little_test2(1024, 1024);
    csrSparseMatrixPtr sparseMatrixPtr = &sparseMatrix;
#else
    if (argc <= 1)
    {
        printf("Erro: You need to modify a file to read\n");
        return 0;
    }
    csrSparseMatrixPtr<double> sparseMatrixPtr = matrix_read_csr<double>(argv[1]);
    if (sparseMatrixPtr == NULL)
    {
        printf("Error: sparse matrix not supported\n");
        return 0;
    }
#endif

    double *data_ptr = sparseMatrixPtr->data_ptr;
    int *column_ptr = sparseMatrixPtr->column_ptr;
    int *row_ptr = sparseMatrixPtr->row_ptr;

    const int data_num = sparseMatrixPtr->data_num;
    const int row_num = sparseMatrixPtr->row_num;
    const int column_num = sparseMatrixPtr->column_num;
    if(row_num != column_num) return 0;
    double *x_array_time = SIMPLE_MALLOC(double, column_num);
    double *y_array_time = SIMPLE_MALLOC(double, row_num);
    init_vec(x_array_time, column_num, INFINITY);
    init_vec(y_array_time, row_num, INFINITY);
    x_array_time[src_vertex] = 0;
    y_array_time[src_vertex] = 0;

    double *x_array0 = SIMPLE_MALLOC(double, column_num);
    double *x_array1 = SIMPLE_MALLOC(double, column_num);
    double *y_array = SIMPLE_MALLOC(double, row_num);
    double *y_array_bak = SIMPLE_MALLOC(double, row_num);
    init_vec(x_array0, column_num, INFINITY);
    init_vec(y_array, row_num, INFINITY);
    init_vec(x_array1, column_num, INFINITY);
    init_vec(y_array_bak, row_num, INFINITY);
    x_array0[src_vertex] = 0;
    x_array1[src_vertex] = 0;
    y_array[src_vertex] = 0;
    y_array_bak[src_vertex] = 0;

    int *row_ptr_all = SIMPLE_MALLOC(int, data_num);
    int *row_ptr_all_bak = SIMPLE_MALLOC(int, data_num);
    for (int row_i = 0; row_i < row_num; row_i++)
    {
        int begin = row_ptr[row_i];
        int end = row_ptr[row_i + 1];
        for (int j = begin; j < end; j++)
        {
            row_ptr_all[j] = row_i;
        }
    }
    memcpy(row_ptr_all_bak, row_ptr_all, data_num);
    // x $= y means x = min (x, y)
    std::string sssp_str =
        "input: int * row_ptr,   \
            int * column_ptr,\
            double * x_array,\
            double * data_ptr\
        output:double * y_array \
        lambda i : \
            y_array[ row_ptr[i] ] $= data_ptr[i] \
            + x_array[column_ptr[i]]\
            ";

    std::map<std::string, void *> name2ptr_map;
    name2ptr_map["row_ptr"] = row_ptr_all;

    name2ptr_map["column_ptr"] = column_ptr;
    name2ptr_map["x_array"] = x_array1;
    name2ptr_map["data_ptr"] = data_ptr;
    name2ptr_map["y_array"] = y_array;

    //LOG(INFO) << data_num / vector_nums;

    Timer::startTimer("compile");
    FuncType func_int64 = (FuncType)compiler(sssp_str, name2ptr_map, data_num / vector_nums);

    Timer::endTimer("compile");

    Timer::printTimer("llvmcompile");
    Timer::printTimer("compile");
    // using FuncType = int(*)( double*,int*,int*,double*,double*);
    // FuncType func = (FuncType)(func_int64);
    // Timer::startTimer("aot");
    //      spmv_local( y_array_bak, x_array,data_ptr,column_ptr,row_ptr,row_num );

    // Timer::endTimer("aot");

    // Timer::printTimer("aot");

    if (with_papi)
    {
        // papi_init();
    }
    else
    {
        // printf("PAPI profiling is disabled.\n");
    }

    int flops = data_num * 2;
    std::string base_name(argv[1]);
    std::vector<std::string> path = splitpath(base_name);
    base_name = remove_extension(path.back());
    std::string aot_name = base_name + std::string(".aot");
    sssp_naive(y_array_bak, row_ptr, column_ptr, x_array0, data_ptr, column_num, row_num);
    PAPI_TEST_EVAL(50, 1000, flops, aot_name.c_str(), sssp_naive( y_array_time, row_ptr, column_ptr, x_array0, data_ptr, column_num, row_num ) );

    std::string jit_name = base_name + std::string(".jit");
    sssp_dynvec((FuncType)func_int64, y_array, row_ptr_all, column_ptr, x_array1, data_ptr, data_num, column_num);
    PAPI_TEST_EVAL(50, 1000, flops, jit_name.c_str(), sssp_dynvec((FuncType)func_int64, y_array_time, row_ptr_all, column_ptr, x_array1, data_ptr, data_num, column_num) );

    if (with_papi)
    {
        // papi_fini();
    }

    if (!check_equal(x_array0, x_array1, column_num))
    {
        return 1;
    }
    return 0;
}
