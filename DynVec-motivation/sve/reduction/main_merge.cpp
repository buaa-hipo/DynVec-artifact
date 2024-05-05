#include "mode.h"
#include <string.h>
#include <assert.h>
#include "common.h"
#include "eval.h"
constexpr int nnz = TEST_SIZE;
constexpr int mask_num = 5;

struct timespec timer_begin, timer_end;
clock_t begin_t, end_t;
volatile uint64_t begin_pct, end_pct;
volatile uint64_t begin_frq, end_frq;
void reduce_inc_sve(const FLOAT* src, FLOAT* dst, void*_idx, int nnz);
void reduce_eq_sve(const FLOAT* src, FLOAT* dst, void*_idx, int nnz);
void reduce_pure(const FLOAT* src, FLOAT* dst, void*_idx, int nnz);
void reduce16_sve(const FLOAT* src, FLOAT* dst, void*_idx, int nnz);
void reduce8_sve(const FLOAT* src, FLOAT* dst, void*_idx, int nnz);
void reduce4_sve(const FLOAT* src, FLOAT* dst, void*_idx, int nnz);
int main() {
    FLOAT* src = new FLOAT[nnz+VECTOR_LEN*2];
    FLOAT* dst0 = new FLOAT[nnz+VECTOR_LEN*2];
    FLOAT* dst1 = new FLOAT[nnz+VECTOR_LEN*2];
    FLOAT* dst2 = new FLOAT[nnz+VECTOR_LEN*2];
    memset(dst0, 0, nnz+VECTOR_LEN*2);
    memset(dst1, 0, nnz+VECTOR_LEN*2);
    memset(dst2, 0, nnz+VECTOR_LEN*2);
    
    src = (FLOAT*)(((uint64_t)(src+VECTOR_LEN))&(~0x3fL));
    dst0 = (FLOAT*)(((uint64_t)(dst0+VECTOR_LEN))&(~0x3fL));
    dst1 = (FLOAT*)(((uint64_t)(dst1+VECTOR_LEN))&(~0x3fL));
    dst2 = (FLOAT*)(((uint64_t)(dst2+VECTOR_LEN))&(~0x3fL));
    for (int i = 0; i < nnz; ++i) {
        // src[i] = i % 8 + 1 + i % 4 + i % 16;
        src[i] = i + 1;
        assert(src[i] != 0);
    }
#ifndef SVE512_SINGLE
    int pattern[8] = {0, 1, 1, 0, 0, 3, 3, 1};
    // int pattern[8] = {0,1,2,3,4,5,6,7};
    int* idx = new int[nnz];
    for (int i = 0; i < nnz; i += 8*2) {
        memcpy(idx+i, pattern, sizeof(pattern));
        for (int j = 0; j < 8; ++j) {
            idx[i+j] += i;
        }
        memcpy(idx+i+8, pattern, sizeof(pattern));
        for (int j = 0; j < 8; ++j) {
            idx[i+j+8] += i;
        }
    }
#else
    int pattern[16] = {0, 1, 1, 0, 0, 3, 3, 1, 8, 9, 9, 8, 8, 11, 11, 9};
    // int pattern[8] = {0,1,2,3,4,5,6,7};
    int* idx = new int[nnz];
    for (int i = 0; i < nnz; i += 16*2) {
        memcpy(idx+i, pattern, sizeof(pattern));
        for (int j = 0; j < 16; ++j) {
            idx[i+j] += i;
        }
        memcpy(idx+i+16, pattern, sizeof(pattern));
        for (int j = 0; j < 16; ++j) {
            idx[i+j+16] += i;
        }
    }
#endif
    // float src[8] = {1, 2, 3, 4, 5, 6, 7, 8};
    // float dst0[16] = {0};
    // float dst1[16] = {0};

    // reduce8_pure(src, dst0, 8);
    // reduce8_avx(src, dst1, 8);
    EVAL1(reduce_pure, src, dst0, idx, nnz, "data/pure.dat");
    EVAL1(reduce4_sve, src, dst1, idx, nnz, "data/sve.dat");
    // EVAL1(reduce16_sve, src, dst1, idx, nnz, "data/sve.dat");
    // EVAL1(reduce8_sve, src, dst1, idx, nnz, "data/sve.dat");
    // EVAL1(reduce_pure, src, dst0, idx, nnz, "data/pure.dat");
    // EVAL1(reduce8_gather, src, dst2, masks, mask_num, nnz, "data/gather.dat");
    // EVAL1(reduce_inc_sve, src, dst1, idx, nnz, "data/sve.dat");
    // EVAL1(reduce_eq_sve, src, dst1, idx, nnz, "data/sve.dat");
    // reduce8_pure(src, dst0, nullptr,8);
    // reduce16_avx(src, dst1, nullptr, 8);
    // bool pass = true;
    // for (int i = 0; i < 8; ++i) {
    //     if (dst0[i] != dst1[i]) {
    //         printf("[%d] %f %f\n", i, dst0[i], dst1[i]);
    //         pass = false;
    //     }
    // }
    // if (pass) {
    //     printf("pass\n");
    // } else {
    //     printf("fail\n");
    // }
    // check(dst0, dst2, nnz);
    check(dst0, dst1, nnz);
}
