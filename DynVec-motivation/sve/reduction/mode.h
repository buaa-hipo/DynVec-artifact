#pragma once
#define USE_256
//#define USE_512
//#define USE_FLOAT
#define USE_DOUBLE

#define EVAL1(TEST, C, A, idx, nnz, log_fn) do {\
    printf("Evaluating %s:\t", #TEST); fflush(stdout);\
    double worst_bw = DBL_MAX; \
    double best_bw = 0; \
    FILE* fp = fopen(log_fn, "w"); \
    for(int k=2048/*avoid influence of timing/loops*/;k<=nnz;k+=(k<STEP?k:STEP)) { \
        for(int i=0;i<WARM_UP;i++) \
            TEST(C,A,idx,k);\
        begin_timer();\
        for(int i=0;i<EVAL_TIME;i++) \
            TEST(C,A,idx,k);\
        end_timer();\
        double bw = get_bandwidth(k,get_time(EVAL_TIME)); \
        worst_bw = MIN(worst_bw, bw);\
        best_bw = MAX(best_bw, bw); \
        /*printf("%d (%d) Finish\n",k,nnz);*/ \
        fprintf(fp, "%d %lf\n", k, bw); \
    } \
    printf("worst: %lf GB/s, best: %lf GB/s\n", worst_bw, best_bw); \
    fclose(fp); \
} while(0)

// #if defined AVX512_DOUBLE || defined AVX2_SINGLE
// #define reduce_gather reduce8_gather
// #define reduce_avx reduce8_avx
// using mask_t = __mmask8;
// inline void dump8(VECREG m) {
//     FLOAT buf[8];
//     VSTORE(buf, m);
//     for (int i = 0; i < 8; ++i) {
//         printf("%10f ", buf[i]);
//     }
//     printf("\n");
//     fflush(stdout);
// }
// #define dump dump8
// #endif

// #if defined AVX512_SINGLE
// #define reduce_gather reduce16_gather
// #define reduce_avx reduce16_avx
// using mask_t = __mmask16;
// inline void dump16(VECREG m) {
//     FLOAT buf[16];
//     VSTORE(buf, m);
//     for (int i = 0; i < 16; ++i) {
//         printf("%10f ", buf[i]);
//     }
//     printf("\n");
//     fflush(stdout);
// }
// #define dump dump16
// #endif
