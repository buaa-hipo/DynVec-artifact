#pragma once
#include <stdio.h>
#include <cstdlib>
#include <cfloat>
#include <arm_sve.h>

#ifdef USE_DOUBLE
    #define FLOAT double
    #define VECTOR_LEN (svcntw() / 2)

    #define VECREG svfloat64_t
    #define VECREG_GI svuint64_t
    #define VECREG_I svuint64_t

    #define VLOAD svld1_f64
    #define VSTORE svst1_f64
    #define VSTORE_I svst1_u64
    #define VGATHER svld1_gather_u64index_f64
    #define VLOAD_GI svld1_u64
    #define VLOAD_I svld1_u64

    #define VCMP svcmpeq_n_u64
    #define VCMPNE svcmpne_n_u64
    #define VPERM svtbl_f64
    #define VBLEND svsel_f64

    #define VADD svadd_f64_m
    #define VREDUCTION svaddv_f64

    #define INDEX_TYPE uint64_t
    #define PERM_T uint64_t
    #define SELECT_T uint64_t
    #ifdef USE_256
        #define SVE256_DOUBLE
    #else 
        #define SVE512_DOUBLE
    #endif
#else
    #define FLOAT float
    #define VECTOR_LEN svcntw()

    #define VECREG svfloat32_t
    #define VECREG_GI svuint32_t
    #define VECREG_I svuint32_t

    #define VLOAD svld1_f32
    #define VSTORE svst1_f32
    #define VSTORE_I svst1_u32
    #define VGATHER svld1_gather_u32index_f32
    #define VLOAD_GI svld1_u32
    #define VLOAD_I svld1_u32

    #define VCMP svcmpeq_n_u32
    #define VCMPNE svcmpne_n_u32
    #define VPERM svtbl_f32
    #define VBLEND svsel_f32

    #define VADD svadd_f32_m
    #define VREDUCTION svaddv_f32

    #define INDEX_TYPE uint32_t
    #define PERM_T uint32_t
    #define SELECT_T uint32_t
    #ifdef USE_256
        #define SVE256_SINGLE
    #else 
        #define SVE512_SINGLE
    #endif
#endif

#ifndef ARRAY_SIZE
#define ARRAY_SIZE (1024*1024)
#endif
#define TEST_SIZE (8*ARRAY_SIZE)

#define MIN(x,y) ((x)<(y)?(x):(y))
#define MAX(x,y) ((x)>(y)?(x):(y))

#ifdef USE_128
#define INPUT_DIR "128-input/"
#else
    #ifdef USE_256
    #define INPUT_DIR "256-input/"
    #else
    #define INPUT_DIR "512-input/"
    #endif
#endif

#ifdef USE_DOUBLE
#define GATHER_PERM_DAT_FN  INPUT_DIR"double-input/perm.dat"
#define SCATTER_PERM_DAT_FN INPUT_DIR"double-input/perm-store.dat"
#define SELECT_DAT_FN INPUT_DIR"double-input/select.dat"
#else
#define GATHER_PERM_DAT_FN INPUT_DIR"single-input/perm.dat"
#define SCATTER_PERM_DAT_FN INPUT_DIR"single-input/perm-store.dat"
#define SELECT_DAT_FN INPUT_DIR"single-input/select.dat"
#endif

// 128bits sve
// void init_para_perm(int perm[4][4], const char* fn) {
// 512bits sve
#ifdef USE_256
inline void init_para_perm(PERM_T perm[8][8], const char* fn)
#else
    #ifdef USE_DOUBLE
        inline void init_para_perm(PERM_T perm[8][8], const char* fn)
    #else
        inline void init_para_perm(PERM_T perm[16][16], const char* fn)
    #endif
#endif
{
    printf("Parameter PERM file: %s\n", fn);
    FILE* f=fopen(fn, "r");
    for(int i=0;i<VECTOR_LEN - 1;++i) {
#ifdef USE_DOUBLE
        for(int j=0;j<VECTOR_LEN;j+=2)
#else
        for(int j=0;j<VECTOR_LEN;++j)
#endif
        {
            int val = 0;
            fscanf(f, "%d", &val);
            perm[i][j] = val;
        }
    }
    fclose(f);
}

#ifdef USE_256
inline void init_para_select(SELECT_T select[8][8], const char* fn)
#else
    #ifdef USE_DOUBLE
        inline void init_para_select(SELECT_T select[8][8], const char* fn)
    #else
        inline void init_para_select(SELECT_T select[16][16], const char* fn)
    #endif
#endif
{
    printf("Parameter SELECT file: %s\n", fn);
    FILE* f = fopen(fn, "r");
    for(int n=0;n<VECTOR_LEN - 1;++n) {
        bool flag = false;
        for(int i=0;i<VECTOR_LEN;++i) {
            int x;
            fscanf(f,"%d", &x);
            select[n][i] = x;
        }
    }
    for(int i = 0; i < VECTOR_LEN - 1; i++) {
        for(int j = 0; j < VECTOR_LEN; j++) {
            printf("%d ", (int)select[i][j]);
        }
        printf("\n");
    }
    fclose(f);
}



inline void check(FLOAT* CREF, FLOAT*COPT, int nnz) {
#ifdef USE_DOUBLE
    FLOAT threshold = 1e-14;
#else
    FLOAT threshold = 1e-7;
#endif

    int num = 0;
    for(int i=0;i<nnz;++i) {
        // printf("%f %f\n", CREF[i], COPT[i]);
        if (abs(CREF[i] - COPT[i]) > threshold) {
#ifdef USE_DOUBLE
            printf("ERROR @ %d: ref %lf (%lx), opt %lf (%lx), ", i, (double)CREF[i], reinterpret_cast<uint64_t*>(CREF)[i], (double)COPT[i], reinterpret_cast<uint64_t*>(COPT)[i]);
#else
            printf("ERROR @ %d: ref %f (%x), opt %f (%x), ", i, CREF[i], reinterpret_cast<uint32_t*>(CREF)[i], COPT[i], reinterpret_cast<uint32_t*>(COPT)[i]);
#endif
            ++num;
            printf("++ error num = %d, max = %lu\n",num, VECTOR_LEN);
        }
        // else if(i<8) {
        //     printf("CORRECT@%d: ref %lf, opt %lf\n", i, (double)CREF[i], (double)COPT[i]);
        // }
        if(num>VECTOR_LEN) {
            printf("TOO MUCH ERROR! RETURN\n");
            return ;
        }
    }
    printf("---- PASS ----\n");
}
