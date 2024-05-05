#include <stdio.h>
#include <stdint.h>
#include <sys/time.h>
#include <time.h>
#include <unistd.h>

uint64_t  Armv8_CNTPCT_EL0_read()
{
	uint64_t val = 0;
	// asm volatile("mrs %0, CNTPCT_EL0": "=r" (val));
    asm volatile("mrs %0, CNTVCT_EL0": "=r" (val));
	return val;
}
 uint64_t  Armv8_CNTFRQ_EL0_read()
{
	uint64_t val = 0;
	asm volatile("mrs %0, CNTFRQ_EL0": "=r" (val));
	return val;
}
//struct timeval timer_begin, timer_end;
struct timespec timer_begin, timer_end;
clock_t begin_t, end_t;
volatile uint64_t begin_pct, end_pct;
volatile uint64_t begin_frq, end_frq;
inline void begin_timer() {
    begin_t = clock();
}

inline void end_timer() {
    end_t = clock();
    printf("%ld, ", end_t - begin_t);
}

inline double get_time(int times) {
    uint64_t nsec = (end_t - begin_t)*1000000000/CLOCKS_PER_SEC;
    return ((double)nsec)/times;
}
inline void print_metrics(int nnz, double sec) {
    double bytes = nnz * VECTOR_LEN * sizeof(FLOAT);
    double bandwidth = (bytes / sec) / 1e9; // GB/s
    printf("%.6lf msec, %lf GB/s (with %ld bytes)\n", sec*1000, bandwidth, nnz*8*sizeof(FLOAT));
}

inline double get_bandwidth(int nnz, double nsec) {
    double bytes = nnz * VECTOR_LEN * sizeof(FLOAT);
    double bandwidth = (bytes / nsec) ; // GB/s
    return bandwidth;
}

#define WARM_UP 3
// repeat time to find the best result
#define REPEAT_TIME 10
// 1000
#define EVAL_TIME 1
#define STEP (ARRAY_SIZE/16)
#define EVAL(TEST, C, A, B, perm, select, nnz, log_fn) do {\
    printf("Evaluating %s:\t", #TEST); fflush(stdout);\
    double worst_bw = DBL_MAX; \
    double best_bw = 0; \
    FILE* fp = fopen(log_fn, "w"); \
    printf("\nVCT:");\
    for(int k=128/*avoid influence of timing/loops*/;k<=nnz;k+=(k<STEP?k:STEP)) { \
        for(int i=0;i<WARM_UP;i++) \
            TEST(C,A,B,reinterpret_cast<PERM_T*>(perm),reinterpret_cast<PERM_T*>(select),k);\
        begin_timer();\
        for(int i=0;i<EVAL_TIME;i++) \
            TEST(C,A,B,reinterpret_cast<PERM_T*>(perm),reinterpret_cast<PERM_T*>(select),k);\
        end_timer();\
        double bw = get_bandwidth(k,get_time(EVAL_TIME)); \
        worst_bw = MIN(worst_bw, bw);\
        best_bw = MAX(best_bw, bw); \
        /* printf("%d (%d) Finish\n",k,nnz);*/ \
        fprintf(fp, "%d %lf\n", k, bw); \
    } \
    printf("\n\n");\
    printf("worst: %lf GB/s, best: %lf GB/s\n\n", worst_bw, best_bw); \
    fclose(fp); \
} while(0)

#define EVAL_REDUCTION(TEST, C, A, nnz, log_fn) do {\
    printf("Evaluating %s:\t", #TEST); fflush(stdout);\
    double worst_bw = DBL_MAX; \
    double best_bw = 0; \
    FILE* fp = fopen(log_fn, "w"); \
    printf("\nVCT:");\
    for(int k=32/*avoid influence of timing/loops*/;k<=nnz;k+=(k<STEP?k:STEP)) { \
        for(int i=0;i<WARM_UP;i++) \
            TEST(C,A,k);\
        begin_timer();\
        for(int i=0;i<EVAL_TIME;i++) \
            TEST(C,A,k);\
        end_timer();\
        double bw = get_bandwidth(k,get_time(EVAL_TIME)); \
        worst_bw = MIN(worst_bw, bw);\
        best_bw = MAX(best_bw, bw); \
        /* printf("%d (%d) Finish\n",k,nnz);*/ \
        fprintf(fp, "%d %lf\n", k, bw); \
    } \
    printf("\n\n");\
    printf("worst: %lf GB/s, best: %lf GB/s\n\n", worst_bw, best_bw); \
    fclose(fp); \
} while(0)
