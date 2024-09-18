/**
 * LLVM equivalent of:
 *
 * int sum(int a, int b) {
 *     return a + b;
 * }
 */
#include "Timers.hpp"
#include "intelligent_unroll.hpp"
// #include "dynvec.h"
#include "util.h"
#include "csr_matrix.h"
#include <functional>
#include "test_utils.h"

#define PRINTINT(x)              \
    do                           \
    {                            \
        printf(#x " %d\n", (x)); \
        fflush(stdout);          \
    } while (0)
void spmv_local(DATATYPE *y_ptr, const DATATYPE *x_ptr, const DATATYPE *data_ptr, const int *column_ptr, const int *row_ptr, const int row_num)
{
#pragma omp parallel for schedule(static)
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

using FuncType = int (*)(float *, int *, int *, float *, float *);

const int max_bits_ = sizeof(float) * ByteSize;
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

inline void pagerank(float *sum, const int *n1, int *n2, float *rank, float *nneibor, int nedges)
{
    for (int j = 0; j < nedges; j++)
    {
        int nx = n1[j];
        int ny = n2[j];
        sum[ny] += rank[nx] / nneibor[nx];
    }
}

inline void pagerank_dynvec(FuncType func, float *sum, int *n1, int *n2, float *rank, float *nneibor, int nedges)
{
    func(sum, n1, n2, rank, nneibor);
    for (int i = (nedges / vector_nums * vector_nums); i < nedges; i++)
    {
        sum[n2[i]] += rank[n1[i]] / nneibor[n1[i]];
    }
}
int thread_num;
inline void pagerank_dynvec_mt(std::function<void()> *func, float *sum, int *n1, int *n2, float *rank, float *nneibor, int nedges)
{

    for (int i = 0; i < thread_num; ++i)
        func[i]();
}

// #define LITTEL_CASE2
int main(int argc, char const *argv[])
{
    bool with_papi = false;
    // if (argc >= 3)
    // {
    //     with_papi = (atoi(argv[2]) != 0);
    // }
    if (argc <= 1)
    {
        printf("Erro: You need to modify a file to read\n");
        return 0;
    }
    PageRankStructurePtr page_rank_structure_ptr = pagerank_read_mtx(argv[1]);
    if (!page_rank_structure_ptr)
        return -1;

    float *sum = page_rank_structure_ptr->sum;

    float *rank = page_rank_structure_ptr->rank;

    int *n1 = page_rank_structure_ptr->n1;

    int *n2 = page_rank_structure_ptr->n2;
    float *nneibor = page_rank_structure_ptr->nneibor;
    int nedges = page_rank_structure_ptr->nedges;
    int nnodes = page_rank_structure_ptr->nnodes;
    float *sum_bak = SIMPLE_MALLOC(float, nnodes);

    float *sum_time = SIMPLE_MALLOC(float, nnodes);
    for (int i = 0; i < nnodes; i++)
        sum_bak[i] = sum[i];

    std::string pagerank_str =
        "input: int * n1,   \
            int * n2,\
            float * rank,\
            float * nneibor\
     output:float * sum \
     lambda i : \
            sum[ n2[i] ] += rank[n1[i]] \
            / nneibor[n1[i]]\
            ";
    std::map<std::string, void *> name2ptr_map;
    name2ptr_map["n1"] = n1;

    name2ptr_map["n2"] = n2;
    name2ptr_map["rank"] = rank;
    name2ptr_map["nneibor"] = nneibor;
    name2ptr_map["sum"] = sum;
    // const int max_bits_ = sizeof(float) * ByteSize;
    //     #ifdef __AVX512CD__
    //         const int vector_bits = 512;

    //         const int vector_nums = vector_bits / max_bits_;
    //     #else
    //         #ifdef __AVX2__
    //         const int vector_bits = 256;

    //         const int vector_nums = vector_bits / max_bits_;
    //         #else
    //         const int vector_nums = -1;
    //         LOG(FATAL) << "Unsupported architetures";
    //         #endif
    //     #endif
    LOG(INFO) << vector_nums;

    LOG(INFO) << nedges / vector_nums;

    Timer::startTimer("compile");
    uint64_t func_int64 = compiler(pagerank_str, name2ptr_map, nedges / vector_nums);
    Timer::endTimer("compile");

    Timer::printTimer("llvmcompile");
    Timer::printTimer("compile");
    thread_num = atoi(argv[2]);
    vector<std::function<void()>> callee;
    int processed_row_num = 0;
    int mode = atoi(argv[3]);
    for (int i = 0; i < thread_num; ++i)
    {

        float *nneibor = page_rank_structure_ptr->nneibor;
        int *n1 = page_rank_structure_ptr->n1;
        int *n2 = page_rank_structure_ptr->n2;

        n2 += processed_row_num;

        nneibor = nneibor + n2[0];
        n1 = n1 + n2[0];
        LOG(INFO) << "n2[0] " << n2[0];
        // for (int p = 0; p < row_num; ++p)
        // {
        //     printf("%d: %d\n", p, n2[p]);
        // }
        LOG(INFO) << "nnodes" << nnodes;
        LOG(INFO) << "nedges" << nedges;
        const int nnodes =
            mode = 0 ? min(page_rank_structure_ptr->nnodes * (i + 1) / thread_num, page_rank_structure_ptr->nnodes) - page_rank_structure_ptr->nnodes * i / thread_num
                     : [=]()
        {int res = 0; int cur_data_num = 0; while(cur_data_num < page_rank_structure_ptr->nedges / thread_num && res + processed_row_num < page_rank_structure_ptr->nnodes) {res++; cur_data_num = n2[res] - n2[0];} return res; }();

        const int nedges = n2[nnodes] - n2[0];
        LOG(INFO) << "nnodes" << nnodes;
        LOG(INFO) << "nedges" << nedges;
        // const int data_num = sparseMatrixPtr->data_num;
        // const int column_num = sparseMatrixPtr->column_num;
        // printf("data_num: %d\n", data_num);
        // printf("sparse_row_num: %d\n", sparseMatrixPtr->nnodes);
        // printf("nnodes: %d\n", nnodes);

        // int *n2_all = SIMPLE_MALLOC(int, nedges);

        // for (int row_i = 0; row_i < nnodes; row_i++)
        // {
        //     int begin = n2[row_i] - n2[0];
        //     int end = n2[row_i + 1] - n2[0];
        //     for (int j = begin; j < end; j++)
        //     {
        //         n2_all[j] = row_i + processed_row_num;
        //     }
        // }

        std::string pagerank_str =
            "input: int * n1,   \
            int * n2,\
            float * rank,\
            float * nneibor\
     output:float * sum \
     lambda i : \
            sum[ n2[i] ] += rank[n1[i]] \
            / nneibor[n1[i]]\
            ";
        std::map<std::string, void *> name2ptr_map;
        name2ptr_map["n1"] = n1;

        name2ptr_map["n2"] = n2;
        name2ptr_map["rank"] = rank;
        name2ptr_map["nneibor"] = nneibor;
        name2ptr_map["sum"] = sum;

        //     LOG(INFO) << data_num / vector_nums;

        // Timer::startTimer("compile");
        func_int64 = compiler(pagerank_str, name2ptr_map, nedges / vector_nums);
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
                ((FuncType)func_int64)(sum, n1, n2, rank, nneibor);
                for (int i = (nedges / vector_nums * vector_nums); i < nedges; i++)
                {
                    sum[n2[i]] += rank[n1[i]] / nneibor[n1[i]];
                }
                // int flops = sparseMatrixPtr->data_num * 2;
                // std::string base_name(argv[1]);
                // std::string jit_name = base_name + std::string(".jit");
                // PAPI_TEST_EVAL1(50, 1000, flops, jit_name.c_str(), bfs_dynvec(src_vertex, (FuncType)func_int64, y_array_time, row_ptr_all, column_ptr, x_array, data_ptr, data_num, column_num, res1));
            });

        // });
        processed_row_num += nedges;
    }

    // using FuncType = int(*)( float*,int*,int*,float*,float*);
    // FuncType func = (FuncType)(func_int64);
    // Timer::startTimer("aot");
    // for(int j=0;j<nedges;j++) {
    //     int nx = n1[j];
    //     int ny = n2[j];
    //     sum_bak[ny] += rank[nx] / nneibor[nx];
    // }

    // Timer::endTimer("aot");

    // Timer::printTimer("aot");

    if (with_papi)
    {
        papi_init();
    }
    else
    {
        printf("PAPI profiling is disabled.\n");
    }

    int flops = nedges * 2;
    std::string base_name(argv[1]);
    std::vector<std::string> path = splitpath(base_name);
    base_name = remove_extension(path.back());
    std::string aot_name = base_name + std::string(".aot");
    pagerank(sum_bak, n1, n2, rank, nneibor, nedges);
    // PAPI_TEST_EVAL(50, 1000, flops, aot_name.c_str(), pagerank(sum_bak, n1, n2, rank, nneibor, nedges));

    std::string jit_name = base_name + std::string(".jit");
    pagerank_dynvec_mt(callee.data(), sum, n1, n2, rank, nneibor, nedges);
    // PAPI_TEST_EVAL(50, 1000, flops, jit_name.c_str(), pagerank_dynvec_mt(callee.data(), sum, n1, n2, rank, nneibor, nedges));

    if (with_papi)
    {
        papi_fini();
    }

    //     func( sum, n1,n2,rank,nneibor );
    //     LOG(INFO) << nedges / vector_nums * vector_nums;
    //     for( int i = (nedges / vector_nums * vector_nums) ; i < nedges ; i++ ) {
    //         sum[ n2[ i ] ] += rank[n1[i]] / nneibor[n1[i]];
    //     }

    // #define WORM_TIMES 50
    //      for( int i = 0 ; i < WORM_TIMES ; i++ ) {
    //         func( sum_time, n1,n2,rank,nneibor );
    //         for( int i = (nedges / vector_nums * vector_nums) ; i < nedges ; i++ ) {
    //             sum_time[ n2[ i ] ] += rank[n1[i]] / nneibor[n1[i]];
    //         }
    //      }

    // #define TIMES 1000
    //     Timer::startTimer("jit");
    //      for( int i = 0 ; i < TIMES ; i++ ){
    //         func( sum_time, n1,n2,rank,nneibor );
    //         for( int i = (nedges / vector_nums * vector_nums) ; i < nedges ; i++ ) {
    //             sum_time[ n2[ i ] ] += rank[n1[i]] / nneibor[n1[i]];
    //         }
    //      }

    //     Timer::endTimer("jit");
    //     Timer::printTimer("jit",TIMES);
    //     Timer::printGFLOPS( "jit", nedges * 2 , TIMES );
    if (!check_equal(sum_bak, sum, nnodes))
    {
        return 1;
    }
    return 0;
}
