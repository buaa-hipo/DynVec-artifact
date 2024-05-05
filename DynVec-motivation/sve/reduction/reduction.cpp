// #define LOAD_MASK
#include <string.h>
#include "mode.h"
#include <assert.h>
#include <iostream>
#include <functional>
#include <stdio.h>
#include "common.h"
#include "eval.h"
void reduce_pure(const FLOAT* src, FLOAT* dst, void*_idx, int nnz);
using namespace std;

void reduce_inc_sve(const FLOAT* src, FLOAT* dst, void*_idx, int nnz) {
    memset(dst,0, nnz*sizeof(FLOAT));
    svbool_t p32_all = svptrue_b32();
    for (int i = 0; i < nnz; i += VECTOR_LEN) {
        VECREG v0 =  VLOAD(p32_all, src+i);
        VECREG v1 =  VLOAD(p32_all, dst+i);
        VECREG v2 = VADD(p32_all, v0, v1);
        VSTORE(p32_all, dst+i, v2);
    }

}
void reduce_eq_sve(const FLOAT* src, FLOAT* dst, void*_idx, int nnz) {
    memset(dst,0, nnz*sizeof(FLOAT));
    svbool_t p32_all = svptrue_b32();
    for (int i = 0; i < nnz; i += VECTOR_LEN) {
        VECREG v0 =  VLOAD(p32_all, src+i);
        FLOAT v1 = VREDUCTION(p32_all, v0);
        dst[i] += v1;
    }
}

#ifdef SVE256_DOUBLE
void reduce4_sve(const FLOAT* src, FLOAT* dst, void*_idx, int nnz) {
    INDEX_TYPE pattern1[] = {3,2,UINT32_MAX,UINT32_MAX};
    INDEX_TYPE pattern2[] = {UINT32_MAX,2,UINT32_MAX,UINT32_MAX};
    INDEX_TYPE pattern3[] = {0,3,UINT32_MAX,1};
    svbool_t p32_all = svptrue_b32();
    VECREG_I idx1 = VLOAD_I(p32_all, reinterpret_cast<const INDEX_TYPE*>(pattern1));
    VECREG_I idx2 = VLOAD_I(p32_all, reinterpret_cast<const INDEX_TYPE*>(pattern2));
    VECREG_I idx3 = VLOAD_I(p32_all, reinterpret_cast<const INDEX_TYPE*>(pattern3));
    svbool_t p_v1_mask = VCMPNE(p32_all, idx1, UINT32_MAX);
    svbool_t p_v3_mask = VCMPNE(p32_all, idx3, UINT32_MAX);

    for(int i=0, j=0;i<nnz;i+=(VECTOR_LEN * 2)) {
        VECREG v1 = VLOAD(p32_all, src + i);
        VECREG v2 = VLOAD(p32_all, src + i + 4);
        
        VECREG v1_permed = VPERM(v1, idx1);
        VECREG v2_permed = VPERM(v2, idx2); // [0, 6, 0, 0]
        VECREG v3 = VADD(p_v1_mask, v1_permed, v1); // so v3 will be [0+3, 1+2, 0, 0]
        VECREG v4 = VADD(p32_all, v2, v2_permed); // [4, 5 + 6, 6, 7]
        VECREG v4_permed = VPERM(v4, idx3); // so v4 will be [4, 7, 0, 5+6]
        VECREG res = VADD(p_v3_mask, v3, v4_permed);
        
        VSTORE(p32_all, dst + i, res);
    }
}
#define reduce_sve reduce4_sve
#endif
// void reduce8_pure(const FLOAT* src, FLOAT* dst, void* masks, int mask_num, int nnz);
#if defined SVE512_DOUBLE || defined SVE256_SINGLE
void reduce8_sve(const FLOAT* src, FLOAT* dst, void*_idx, int nnz) {
    INDEX_TYPE pattern1[] = {3,UINT32_MAX,7,UINT32_MAX,UINT32_MAX,6,UINT32_MAX,UINT32_MAX};
    INDEX_TYPE pattern2[] = {4,2,UINT32_MAX,UINT32_MAX,UINT32_MAX,UINT32_MAX,UINT32_MAX,UINT32_MAX};
    INDEX_TYPE pattern3[] = {0,1,UINT32_MAX,5,UINT32_MAX,UINT32_MAX,UINT32_MAX,UINT32_MAX};
    svbool_t p32_all = svptrue_b32();
    
    VECREG_I idx1 = VLOAD_I(p32_all, reinterpret_cast<const INDEX_TYPE*>(pattern1));
    VECREG_I idx2 = VLOAD_I(p32_all, reinterpret_cast<const INDEX_TYPE*>(pattern2));
    VECREG_I idx3 = VLOAD_I(p32_all, reinterpret_cast<const INDEX_TYPE*>(pattern3));
    svbool_t p_v1_mask = VCMPNE(p32_all, idx1, UINT32_MAX);
    svbool_t p_v2_mask = VCMPNE(p32_all, idx2, UINT32_MAX);
    
    for(int i=0, j=0;i<nnz;i+=VECTOR_LEN) {
        VECREG v1 = VLOAD(p32_all, src + i);
        
        VECREG v1_permed = VPERM(v1, idx1);
        VECREG v2 = VADD(p_v1_mask, v1, v1_permed);
        VECREG v2_permed = VPERM(v2, idx2);
        VECREG v3 = VADD(p_v2_mask, v2, v2_permed);
        VECREG res = VPERM(v3, idx3);
        
        VSTORE(p32_all, dst + i, res);
    }
}
#define reduce_sve reduce8_sve
#endif

#ifdef SVE512_SINGLE
void reduce16_sve(const FLOAT* src, FLOAT* dst, void*_idx, int nnz) {
    INDEX_TYPE pattern1[] = {3,UINT32_MAX,7,UINT32_MAX,UINT32_MAX,6,UINT32_MAX,UINT32_MAX,11,UINT32_MAX,15,UINT32_MAX,UINT32_MAX,14,UINT32_MAX,UINT32_MAX};
    INDEX_TYPE pattern2[] = {4,2,UINT32_MAX,UINT32_MAX,UINT32_MAX,UINT32_MAX,UINT32_MAX,UINT32_MAX,12,10,UINT32_MAX,UINT32_MAX,UINT32_MAX,UINT32_MAX,UINT32_MAX,UINT32_MAX};
    INDEX_TYPE pattern3[] = {0,1,UINT32_MAX,5,UINT32_MAX,UINT32_MAX,UINT32_MAX,UINT32_MAX,8,9,UINT32_MAX,13,UINT32_MAX,UINT32_MAX,UINT32_MAX,UINT32_MAX};
    svbool_t p32_all = svptrue_b32();
    
    VECREG_I idx1 = VLOAD_I(p32_all, reinterpret_cast<const INDEX_TYPE*>(pattern1));
    VECREG_I idx2 = VLOAD_I(p32_all, reinterpret_cast<const INDEX_TYPE*>(pattern2));
    VECREG_I idx3 = VLOAD_I(p32_all, reinterpret_cast<const INDEX_TYPE*>(pattern3));
    svbool_t p_v1_mask = VCMPNE(p32_all, idx1, UINT32_MAX);
    svbool_t p_v2_mask = VCMPNE(p32_all, idx2, UINT32_MAX);
    
    for(int i=0, j=0;i<nnz;i+=VECTOR_LEN) {
        VECREG v1 = VLOAD(p32_all, src + i);
        
        VECREG v1_permed = VPERM(v1, idx1);
        VECREG v2 = VADD(p_v1_mask, v1, v1_permed);
        VECREG v2_permed = VPERM(v2, idx2);
        VECREG v3 = VADD(p_v2_mask, v2, v2_permed);
        VECREG res = VPERM(v3, idx3);
        
        VSTORE(p32_all, dst + i, res);
    }
}
#define reduce_sve reduce16_sve
#endif
