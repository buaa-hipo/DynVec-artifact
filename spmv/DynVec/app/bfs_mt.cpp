/**
 * LLVM equivalent of:
 *
 * int sum(int a, int b) {
 *     return a + b;
 * }
 */
#include <omp.h>
#include <thread>
#include <atomic>
using std::thread;
#include <semaphore.h>
#include <set>
#include "Timers.hpp"
#include "intelligent_unroll.hpp"
// #include "dynvec.h"
#include "util.h"
#include "csr_matrix.h"

#include "test_utils.h"
struct Barrier
{
    sem_t sem_barrier, sem_count;
    std::atomic_int _count{0};
    std::atomic_int _count_target;
    std::atomic_int exited{0};
    int id;

    Barrier(int _id) : id(_id)
    {
        sem_init(&sem_barrier, 0, 0);
        sem_init(&sem_count, 0, 1);
    }

    void barrier()
    {
        sem_wait(&sem_count);
        _count.fetch_add(1);
        auto c = _count.load();
        if (c + exited.load() == _count_target.load())
        {
            _count.store(0);
            sem_post(&sem_count);
            for (int i = 0; i < c - 1; ++i)
            {
                // printf("release barrier %d\n", id);
                // fflush(stdout);
                sem_post(&sem_barrier);
            }
        }
        else
        {
            sem_post(&sem_count);
            // printf("fetch barrier %d\n", id);
            // fflush(stdout);
            sem_wait(&sem_barrier);
        }
    }
    void exit_barrier()
    {
        sem_wait(&sem_count);
        exited.fetch_add(1);
        if (_count.load() + exited.load() == _count_target.load())
        {
            auto c = _count.load();
            _count.store(0);
            sem_post(&sem_count);
            for (int i = 0; i < c; ++i)
            {
                // printf("release barrier exit %d\n", id);
                // fflush(stdout);
                sem_post(&sem_barrier);
            }
            // printf("exit %d up\n", id);
            // fflush(stdout);
        }
        else
        {
            sem_post(&sem_count);
            // printf("exit %d down\n", id);
            // fflush(stdout);
            // sem_wait(&sem_barrier);
        }
    }
};
Barrier b1(1);
Barrier b2(2);

#define PRINTINT(x)              \
    do                           \
    {                            \
        printf(#x " %d\n", (x)); \
        fflush(stdout);          \
    } while (0)
void spmv_local(DATATYPE *y_ptr, const DATATYPE *x_ptr, const DATATYPE *data_ptr, const int *column_ptr, const int *row_ptr, const int row_num)
{
#pragma omp parallel for schedule(dynamic)
    for (int i = 0; i < row_num; i++)
    {
        DATATYPE sum = 0;
        for (int j = row_ptr[i]; j < row_ptr[i + 1]; j++)
        {
            sum += x_ptr[column_ptr[j]] * data_ptr[j];
        }
        y_ptr[i] += sum;
    }
}

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
        if (v1[i] == 0 && v2[i] == 0)
            continue;
        else if (v1[i] == 0)
        {
            std::cout << i << " " << v1[i] << " " << v2[i] << "\n";
            continue;
        }
        if ((v1[i] - v2[i]) / v1[i] > 1e-3 || (v2[i] - v1[i]) / v1[i] > 1e-3)
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

inline void bfs_naive(const int src_vertex, double *y_array, int *row_ptr_all, int *column_ptr, double *x_array, double *data_ptr, int row_num, int column_num, int *res)
{
    set<int> visited;
    visited.insert(src_vertex);
    res[0] = src_vertex;
    int pre_size = -1;
    while (visited.size() != pre_size)
    {
        pre_size = visited.size();

        spmv_local(y_array, x_array, data_ptr, column_ptr, row_ptr_all, row_num);

        for (int i = 0; i < column_num; i++)
        {
            x_array[i] = 0;
            if (y_array[i] && visited.find(i) == visited.end())
            {
                x_array[i] = 1;
                visited.insert(i);
                res[visited.size() - 1] = i;
            }
        }
    }
}

inline void bfs_dynvec(const int src_vertex, FuncType func, double *y_array, int *row_ptr_all, int *column_ptr, double *x_array, double *data_ptr, int data_num, int column_num, int *res)
{
    set<int> visited;
    visited.insert(src_vertex);
    res[0] = src_vertex;
    int pre_size = -1;
    while (visited.size() != pre_size)
    {
        pre_size = visited.size();
        // for (int i = 0; i < column_num; i++) {
        //     printf("%lf, ", x_array[i]);
        // }
        // printf("\n");

        // for (int i = 0; i < column_num; i++) {
        //     printf("%lf, ", y_array[i]);
        // }
        // printf("\n");
        func(y_array, row_ptr_all, column_ptr, x_array, data_ptr);
        for (int i = data_num / vector_nums * vector_nums; i < data_num; i++)
            y_array[row_ptr_all[i]] += x_array[column_ptr[i]] * data_ptr[i];
        // jit will allocate new data for all the ptr, so we don't need to copy all these ptrs per iteration.
        // memcpy(column_ptr, column_ptr_bak, data_num);
        // memcpy(row_ptr_all, row_ptr_all_bak, data_num);
        // memcpy(data_ptr, data_ptr_bak, data_num);
        // name2ptr_map[ "row_ptr" ] = row_ptr_all;

        // name2ptr_map[ "column_ptr" ] = column_ptr;
        // name2ptr_map[ "x_array" ] = x_array;
        // name2ptr_map[ "data_ptr" ] = data_ptr;
        // name2ptr_map[ "y_array" ] = y_array;
        // we don't need to perform jit per iteration cause the x_array will never be transform
        // func = (FuncType) compiler( spmv_str,name2ptr_map,data_num/vector_nums );

        b1.barrier();
        for (int i = 0; i < column_num; i++)
        {
            x_array[i] = 0;
            if (y_array[i] && visited.find(i) == visited.end())
            {
                x_array[i] = 1;
                visited.insert(i);
                res[visited.size() - 1] = i;
            }
        }
        b2.barrier();
    }
    // b1.exit_barrier();
    // b2.exit_barrier();
}
int thread_num;
inline void bfs_dynvec_mt(const int src_vertex, std::function<void()> *func, double *y_array, int *row_ptr_all, int *column_ptr, double *x_array, double *data_ptr, int data_num, int column_num, int *res)
{
    set<int> visited;
    visited.insert(src_vertex);
    res[0] = src_vertex;
    int pre_size = -1;
    while (visited.size() != pre_size)
    {
        pre_size = visited.size();
        // for (int i = 0; i < column_num; i++) {
        //     printf("%lf, ", x_array[i]);
        // }
        // printf("\n");

        // for (int i = 0; i < column_num; i++) {
        //     printf("%lf, ", y_array[i]);
        // }
        // printf("\n");

#pragma omp parallel for
        for (int i = 0; i < thread_num; ++i)
            func[i]();
        // jit will allocate new data for all the ptr, so we don't need to copy all these ptrs per iteration.
        // memcpy(column_ptr, column_ptr_bak, data_num);
        // memcpy(row_ptr_all, row_ptr_all_bak, data_num);
        // memcpy(data_ptr, data_ptr_bak, data_num);
        // name2ptr_map[ "row_ptr" ] = row_ptr_all;

        // name2ptr_map[ "column_ptr" ] = column_ptr;
        // name2ptr_map[ "x_array" ] = x_array;
        // name2ptr_map[ "data_ptr" ] = data_ptr;
        // name2ptr_map[ "y_array" ] = y_array;
        // we don't need to perform jit per iteration cause the x_array will never be transform
        // func = (FuncType) compiler( spmv_str,name2ptr_map,data_num/vector_nums );

        // b1.barrier();
        for (int i = 0; i < column_num; i++)
        {
            x_array[i] = 0;
            if (y_array[i] && visited.find(i) == visited.end())
            {
                x_array[i] = 1;
                visited.insert(i);
                res[visited.size() - 1] = i;
            }
        }
        // b2.barrier();
    }
    // b1.exit_barrier();
    // b2.exit_barrier();
}

// #define LITTEL_CASE2
int main(int argc, char const *argv[])
{
    bool with_papi = false;
    // bfs file src_vertex with_papi
    int src_vertex = atoi(argv[2]);
    // if (argc >= 4)
    // {
    //     with_papi = (atoi(argv[3]) != 0);
    // }
#ifdef LITTEL_CASE
    csrSparseMatrix sparseMatrix = little_test();
    csrSparseMatrixPtr sparseMatrixPtr = &sparseMatrix;
#elif defined LITTEL_CASE2

    csrSparseMatrix sparseMatrix = little_test2(1024, 1024);
    csrSparseMatrixPtr sparseMatrixPtr = &sparseMatrix;
#else
    if (argc <= 4)
    {
        printf("./bfs_mt <filename> <src-id> <thread-num> <mode>\n");
        return 0;
    }
    csrSparseMatrixPtr sparseMatrixPtr = matrix_read_csr(argv[1]);
    if (sparseMatrixPtr == NULL)
    {
        printf("Error: sparse matrix not supported\n");
        return 0;
    }
#endif
    omp_set_num_threads(atoi(argv[3]));

    double *data_ptr = sparseMatrixPtr->data_ptr;
    int *column_ptr = sparseMatrixPtr->column_ptr;
    int *row_ptr = sparseMatrixPtr->row_ptr;

    const int data_num = sparseMatrixPtr->data_num;
    int *column_ptr_bak = SIMPLE_MALLOC(int, data_num);
    memcpy(column_ptr_bak, column_ptr, data_num);
    int *data_ptr_bak = SIMPLE_MALLOC(int, data_num);
    memcpy(data_ptr_bak, data_ptr, data_num);
    const int row_num = sparseMatrixPtr->row_num;
    const int column_num = sparseMatrixPtr->column_num;
    if (column_num != row_num)
        return 0;
    double *x_array = SIMPLE_MALLOC(double, column_num);
    int *res0 = SIMPLE_MALLOC(int, column_num);
    int *res1 = SIMPLE_MALLOC(int, column_num);
    double *y_array = SIMPLE_MALLOC(double, row_num);
    double *y_array_bak = SIMPLE_MALLOC(double, row_num);

    double *y_array_time = SIMPLE_MALLOC(double, row_num);

    memset(x_array, 0, column_num * sizeof(double));
    memset(res0, 0, column_num * sizeof(int));
    memset(res1, 0, column_num * sizeof(int));
    x_array[src_vertex] = 1;
    init_vec(y_array, row_num, 0);
    init_vec(y_array_bak, row_num, 0);

    init_vec(y_array_time, row_num, 0);
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

    std::map<std::string, void *> name2ptr_map;
    name2ptr_map["row_ptr"] = row_ptr_all;

    name2ptr_map["column_ptr"] = column_ptr;
    name2ptr_map["x_array"] = x_array;
    name2ptr_map["data_ptr"] = data_ptr;
    name2ptr_map["y_array"] = y_array;

    LOG(INFO) << data_num / vector_nums;

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
    // Timer::startTimer("compile");
    FuncType func_int64; // = (FuncType)compiler(spmv_str, name2ptr_map, data_num / vector_nums);

    // Timer::endTimer("compile");

    // Timer::printTimer("llvmcompile");
    // Timer::printTimer("compile");

    memset(x_array, 0, column_num * sizeof(double));
    x_array[src_vertex] = 1;
    thread_num = atoi(argv[3]);
    b1._count_target.store(thread_num);
    b2._count_target.store(thread_num);
    std::thread **threads = new std::thread *[thread_num];
    vector<std::function<void()>> callee;
    int processed_row_num = 0;
    int mode = atoi(argv[4]);
    for (int i = 0; i < thread_num; ++i)
    {

        double *data_ptr = sparseMatrixPtr->data_ptr;
        int *column_ptr = sparseMatrixPtr->column_ptr;
        int *row_ptr = sparseMatrixPtr->row_ptr;

        row_ptr += processed_row_num;

        data_ptr = data_ptr + row_ptr[0];
        column_ptr = column_ptr + row_ptr[0];
        // for (int p = 0; p < row_num; ++p)
        // {
        //     printf("%d: %d\n", p, row_ptr[p]);
        // }
        const int row_num =
            mode = 0 ? min(sparseMatrixPtr->row_num * (i + 1) / thread_num, sparseMatrixPtr->row_num) - sparseMatrixPtr->row_num * i / thread_num
                     : [=]()
        {int res = 0; int cur_data_num = 0; while(cur_data_num < data_num / thread_num && res + processed_row_num < sparseMatrixPtr->row_num) {res++; cur_data_num = row_ptr[res] - row_ptr[0];} return res; }();

        const int data_num = row_ptr[row_num] - row_ptr[0];
        // const int data_num = sparseMatrixPtr->data_num;
        const int column_num = sparseMatrixPtr->column_num;
        // printf("data_num: %d\n", data_num);
        // printf("sparse_row_num: %d\n", sparseMatrixPtr->row_num);
        // printf("row_num: %d\n", row_num);

        int *row_ptr_all = SIMPLE_MALLOC(int, data_num);

        for (int row_i = 0; row_i < row_num; row_i++)
        {
            int begin = row_ptr[row_i] - row_ptr[0];
            int end = row_ptr[row_i + 1] - row_ptr[0];
            for (int j = begin; j < end; j++)
            {
                row_ptr_all[j] = row_i + processed_row_num;
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
        // spmv_str =
        //" input:  int * row_ptr,   \
            //        int * column_ptr,\
            //        double * x_array,\
            //        double * data_ptr\
            // output:double * y_array \
            // lambda i : \
            //        y_array[ i ] += data_ptr[i] \
            //        ";
        std::map<std::string, void *> name2ptr_map;
        int st_row = row_num / thread_num * i;
        // while(*row_ptr_all < st_row) ++row_ptr_all;
        name2ptr_map["row_ptr"] = row_ptr_all;

        name2ptr_map["column_ptr"] = column_ptr;
        name2ptr_map["x_array"] = x_array;
        name2ptr_map["data_ptr"] = data_ptr;
        name2ptr_map["y_array"] = y_array;

        //     LOG(INFO) << data_num / vector_nums;

        // Timer::startTimer("compile");
        func_int64 = (FuncType)compiler(spmv_str, name2ptr_map, data_num / vector_nums);
        // uint64_t func_int64 = compiler(spmv_str, name2ptr_map, data_num / vector_nums);

        // Timer::endTimer("compile");

        // Timer::printTimer("llvmcompile");
        // Timer::printTimer("compile");
        // threads[i] = new std::thread([=](){

        // std::string jit_name = base_name + std::string(".jit");
        callee.emplace_back(
            [=]()
            {
                // spmv_dynvec((FuncType)func_int64, y_array, row_ptr_all, column_ptr, x_array, data_ptr, data_num);
                // bfs_dynvec(src_vertex, (FuncType)func_int64, y_array_time, row_ptr_all, column_ptr, x_array, data_ptr, data_num, column_num, res1);
                // bfs_dynvec(src_vertex, (FuncType)func_int64, y_array, row_ptr_all, column_ptr, x_array, data_ptr, data_num, column_num, res1);
                func_int64(y_array, row_ptr_all, column_ptr, x_array, data_ptr);
                for (int i = data_num / vector_nums * vector_nums; i < data_num; i++)
                    y_array[row_ptr_all[i]] += x_array[column_ptr[i]] * data_ptr[i];
                // int flops = sparseMatrixPtr->data_num * 2;
                // std::string base_name(argv[1]);
                // std::string jit_name = base_name + std::string(".jit");
                // PAPI_TEST_EVAL1(50, 1000, flops, jit_name.c_str(), bfs_dynvec(src_vertex, (FuncType)func_int64, y_array_time, row_ptr_all, column_ptr, x_array, data_ptr, data_num, column_num, res1));
            });

        // });
        processed_row_num += row_num;
    }
    int flops = sparseMatrixPtr->data_num * 2;
    // Timer::startTimer("dynvec");
    // for (int i = 0; i < thread_num; ++i)
    // {
    //     threads[i] = new thread(callee[i]);
    // }
    // for (int i = 0; i < thread_num; ++i)
    // {
    //     threads[i]->join();
    // }
    // Timer::endTimer("dynvec");
    // Timer::printTimer("dynvec", 1050);
    // Timer::printGFLOPS("dynvec", flops, 1050);

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
        printf("PAPI profiling is disabled.\n");
    }

    // int flops = data_num * 2;
    std::string base_name(argv[1]);
    std::vector<std::string> path = splitpath(base_name);
    base_name = remove_extension(path.back());
    std::string aot_name = base_name + std::string(".aot");
    // bfs_naive(src_vertex, y_array_bak, row_ptr, column_ptr, x_array, data_ptr, row_num, column_num, res0);
    // spmv_local( y_array_bak, x_array,data_ptr,column_ptr,row_ptr,row_num );
    memset(x_array, 0, column_num * sizeof(double));
    x_array[src_vertex] = 1;
    // Timer::startTimer("naive");
    // bfs_naive(src_vertex, y_array_time, row_ptr, column_ptr, x_array, data_ptr, row_num, column_num, res0);
    // Timer::endTimer("naive");
    // Timer::printTimer("naive");
    PAPI_TEST_EVAL(50, 1000, flops, aot_name.c_str(), bfs_naive(src_vertex, y_array_time, row_ptr, column_ptr, x_array, data_ptr, row_num, column_num, res0));

    memset(x_array, 0, column_num * sizeof(double));
    x_array[src_vertex] = 1;

    std::string jit_name = base_name + std::string(".jit");
    // bfs_dynvec(src_vertex, (FuncType)func_int64, y_array, row_ptr_all, column_ptr, x_array, data_ptr, data_num, column_num, res1);
    // PAPI_TEST_EVAL(50, 1000, flops, jit_name.c_str(), bfs_dynvec(src_vertex, (FuncType)func_int64, y_array_time, row_ptr_all, column_ptr, x_array, data_ptr, data_num, column_num, res1));
    PAPI_TEST_EVAL(50, 1000, flops, jit_name.c_str(), bfs_dynvec_mt(src_vertex, callee.data(), y_array_time, row_ptr_all, column_ptr, x_array, data_ptr, data_num, column_num, res1));

    if (with_papi)
    {
        // papi_fini();
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
    if (!check_equal(res0, res1, column_num))
    {
        return 1;
    }
    return 0;
}
