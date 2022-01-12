#include <stdio.h>
#include <stdint.h>
#include <sys/time.h>
#include <time.h>
//struct timeval timer_begin, timer_end;
struct timespec timer_begin, timer_end;
inline void begin_timer() {
    //gettimeofday(&timer_begin, 0);
    clock_gettime(CLOCK_REALTIME, &timer_begin);
}

inline void end_timer() {
    //gettimeofday(&timer_end, 0);
    clock_gettime(CLOCK_REALTIME, &timer_end);
}
// effective bandwidth
// inline void print_metrics(int nnz, int times) {
//     uint64_t tb = timer_begin.tv_sec * 1000000LL + timer_begin.tv_usec;
//     uint64_t te = timer_end.tv_sec * 1000000LL + timer_end.tv_usec;
//     uint64_t usec = te - tb;
//     double sec = usec * 1.0e-6 / times;
//     double bytes = nnz * 8 * sizeof(FLOAT);
//     double bandwidth = (bytes / sec) / 1e9; // GB/s
//     printf("%lf sec, %lf GB/s (with %ld bytes)\n", sec, bandwidth, nnz*8*sizeof(FLOAT));
// }
inline double get_time(int times) {
    uint64_t tb = timer_begin.tv_sec * 1000000000LL + timer_begin.tv_nsec;
    uint64_t te = timer_end.tv_sec * 1000000000LL + timer_end.tv_nsec;
    uint64_t nsec = te - tb;
    double sec = nsec * 1.0e-9 / times;
    return sec;
}
inline void print_metrics(int nnz, double sec) {
    double bytes = nnz * 8 * sizeof(FLOAT);
    double bandwidth = (bytes / sec) / 1e9; // GB/s
    printf("%.6lf msec, %lf GB/s (with %ld bytes)\n", sec*1000, bandwidth, nnz*8*sizeof(FLOAT));
}

inline double get_bandwidth(int nnz, double sec) {
    double bytes = nnz * 8 * sizeof(FLOAT);
    double bandwidth = (bytes / sec) / 1e9; // GB/s
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
    for(int k=32/*avoid influence of timing/loops*/;k<=nnz;k+=(k<STEP?k:STEP)) { \
        for(int i=0;i<WARM_UP;i++) \
            TEST(C,A,B,reinterpret_cast<VECREG_I*>(perm),select,k);\
        begin_timer();\
        for(int i=0;i<EVAL_TIME;i++) \
            TEST(C,A,B,reinterpret_cast<VECREG_I*>(perm),select,k);\
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