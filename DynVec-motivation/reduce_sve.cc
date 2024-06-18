#include "common.h"
#include "eval.h"
#include <cstring>

void init_data(FLOAT* A, int nnz) {
    for (int i = 0; i < nnz; i++) {
        A[i] = i;
    }
}

void reduction_native_c(FLOAT* C, FLOAT* A, int nnz) {
    for(int i=0;i<nnz;i+=8) {
        FLOAT* src = A + i;
        FLOAT* dst = C + i;
        FLOAT res1 = src[0] + src[3] + src[4];
        FLOAT res2 = src[1] + src[2] + src[7];
        FLOAT res3 = src[5] + src[6];

        dst[0] = res1;
        dst[1] = res2;
        dst[3] = res3;
    }
}

#if (defined USE_SINGLE && defined USE_512)
void reduction_gather_reduction(FLOAT* C, FLOAT* A, int nnz) {
    svbool_t p32_all = svptrue_b32();

    INDEX_TYPE pattern1[] = {0,3,4,UINT32_MAX,UINT32_MAX,UINT32_MAX,UINT32_MAX,UINT32_MAX,UINT32_MAX,UINT32_MAX,UINT32_MAX,UINT32_MAX,UINT32_MAX,UINT32_MAX,UINT32_MAX,UINT32_MAX};
    INDEX_TYPE pattern2[] = {1,2,7,UINT32_MAX,UINT32_MAX,UINT32_MAX,UINT32_MAX,UINT32_MAX,UINT32_MAX,UINT32_MAX,UINT32_MAX,UINT32_MAX,UINT32_MAX,UINT32_MAX,UINT32_MAX,UINT32_MAX};
    INDEX_TYPE pattern3[] = {5,6,UINT32_MAX,UINT32_MAX,UINT32_MAX,UINT32_MAX,UINT32_MAX,UINT32_MAX,UINT32_MAX,UINT32_MAX,UINT32_MAX,UINT32_MAX,UINT32_MAX,UINT32_MAX,UINT32_MAX,UINT32_MAX};
    INDEX_TYPE pattern4[] = {8,11,12,UINT32_MAX,UINT32_MAX,UINT32_MAX,UINT32_MAX,UINT32_MAX,UINT32_MAX,UINT32_MAX,UINT32_MAX,UINT32_MAX,UINT32_MAX,UINT32_MAX,UINT32_MAX,UINT32_MAX};
    INDEX_TYPE pattern5[] = {9,10,15,UINT32_MAX,UINT32_MAX,UINT32_MAX,UINT32_MAX,UINT32_MAX,UINT32_MAX,UINT32_MAX,UINT32_MAX,UINT32_MAX,UINT32_MAX,UINT32_MAX,UINT32_MAX,UINT32_MAX};
    INDEX_TYPE pattern6[] = {13,14,UINT32_MAX,UINT32_MAX,UINT32_MAX,UINT32_MAX,UINT32_MAX,UINT32_MAX,UINT32_MAX,UINT32_MAX,UINT32_MAX,UINT32_MAX,UINT32_MAX,UINT32_MAX,UINT32_MAX,UINT32_MAX};
    VECREG_I idx1 = VLOAD_I(p32_all, reinterpret_cast<const INDEX_TYPE*>(pattern1));
    VECREG_I idx2 = VLOAD_I(p32_all, reinterpret_cast<const INDEX_TYPE*>(pattern2));
    VECREG_I idx3 = VLOAD_I(p32_all, reinterpret_cast<const INDEX_TYPE*>(pattern3));
    VECREG_I idx4 = VLOAD_I(p32_all, reinterpret_cast<const INDEX_TYPE*>(pattern4));
    VECREG_I idx5 = VLOAD_I(p32_all, reinterpret_cast<const INDEX_TYPE*>(pattern5));
    VECREG_I idx6 = VLOAD_I(p32_all, reinterpret_cast<const INDEX_TYPE*>(pattern6));
    svbool_t p_pat1 = VCMPNE(p32_all, idx1, UINT32_MAX);
    svbool_t p_pat2 = VCMPNE(p32_all, idx2, UINT32_MAX);
    svbool_t p_pat3 = VCMPNE(p32_all, idx3, UINT32_MAX);
    svbool_t p_pat4 = VCMPNE(p32_all, idx4, UINT32_MAX);
    svbool_t p_pat5 = VCMPNE(p32_all, idx5, UINT32_MAX);
    svbool_t p_pat6 = VCMPNE(p32_all, idx6, UINT32_MAX);
    for(int i=0;i<nnz;i+=VECTOR_LEN) {
        FLOAT* src = A + i;
        FLOAT* dst = C + i;
        
        VECREG vdata1 = VGATHER(p_pat1, src, idx1);
        VECREG vdata2 = VGATHER(p_pat2, src, idx2);
        VECREG vdata3 = VGATHER(p_pat3, src, idx3);
        VECREG vdata4 = VGATHER(p_pat4, src, idx4);
        VECREG vdata5 = VGATHER(p_pat5, src, idx5);
        VECREG vdata6 = VGATHER(p_pat6, src, idx6);
        
        FLOAT res1 = VREDUCTION(p_pat1, vdata1);
        FLOAT res2 = VREDUCTION(p_pat2, vdata2);
        FLOAT res3 = VREDUCTION(p_pat3, vdata3);
        FLOAT res4 = VREDUCTION(p_pat4, vdata4);
        FLOAT res5 = VREDUCTION(p_pat5, vdata5);
        FLOAT res6 = VREDUCTION(p_pat6, vdata6);

        dst[0] = res1;
        dst[1] = res2;
        dst[3] = res3;
        dst[8] = res4;
        dst[9] = res5;
        dst[11] = res6;
    }
}
#elif ((defined USE_DOUBLE && defined USE_512) || (defined USE_SINGLE && defined USE_256))
void reduction_gather_reduction(FLOAT* C, FLOAT* A, int nnz) {
    svbool_t p32_all = svptrue_b32();

    INDEX_TYPE pattern1[] = {0,3,4,UINT32_MAX,UINT32_MAX,UINT32_MAX,UINT32_MAX,UINT32_MAX,UINT32_MAX,UINT32_MAX,UINT32_MAX,UINT32_MAX,UINT32_MAX,UINT32_MAX,UINT32_MAX,UINT32_MAX};
    INDEX_TYPE pattern2[] = {1,2,7,UINT32_MAX,UINT32_MAX,UINT32_MAX,UINT32_MAX,UINT32_MAX,UINT32_MAX,UINT32_MAX,UINT32_MAX,UINT32_MAX,UINT32_MAX,UINT32_MAX,UINT32_MAX,UINT32_MAX};
    INDEX_TYPE pattern3[] = {5,6,UINT32_MAX,UINT32_MAX,UINT32_MAX,UINT32_MAX,UINT32_MAX,UINT32_MAX,UINT32_MAX,UINT32_MAX,UINT32_MAX,UINT32_MAX,UINT32_MAX,UINT32_MAX,UINT32_MAX,UINT32_MAX};
    VECREG_I idx1 = VLOAD_I(p32_all, reinterpret_cast<const INDEX_TYPE*>(pattern1));
    VECREG_I idx2 = VLOAD_I(p32_all, reinterpret_cast<const INDEX_TYPE*>(pattern2));
    VECREG_I idx3 = VLOAD_I(p32_all, reinterpret_cast<const INDEX_TYPE*>(pattern3));
    svbool_t p_pat1 = VCMPNE(p32_all, idx1, UINT32_MAX);
    svbool_t p_pat2 = VCMPNE(p32_all, idx2, UINT32_MAX);
    svbool_t p_pat3 = VCMPNE(p32_all, idx3, UINT32_MAX);
    for(int i=0;i<nnz;i+=VECTOR_LEN) {
        FLOAT* src = A + i;
        FLOAT* dst = C + i;
        
        VECREG vdata1 = VGATHER(p_pat1, src, idx1);
        VECREG vdata2 = VGATHER(p_pat2, src, idx2);
        VECREG vdata3 = VGATHER(p_pat3, src, idx3);
        
        FLOAT res1 = VREDUCTION(p_pat1, vdata1);
        FLOAT res2 = VREDUCTION(p_pat2, vdata2);
        FLOAT res3 = VREDUCTION(p_pat3, vdata3);
        
        dst[0] = res1;
        dst[1] = res2;
        dst[3] = res3;
    }
}
#elif (defined USE_DOUBLE && defined USE_256)
void reduction_gather_reduction(FLOAT* C, FLOAT* A, int nnz) {
    svbool_t p32_all = svptrue_b32();

    INDEX_TYPE pattern1[] = {0,3,4,UINT32_MAX};
    INDEX_TYPE pattern2[] = {1,2,7,UINT32_MAX};
    INDEX_TYPE pattern3[] = {5,6,UINT32_MAX,UINT32_MAX};
    VECREG_I idx1 = VLOAD_I(p32_all, reinterpret_cast<const INDEX_TYPE*>(pattern1));
    VECREG_I idx2 = VLOAD_I(p32_all, reinterpret_cast<const INDEX_TYPE*>(pattern2));
    VECREG_I idx3 = VLOAD_I(p32_all, reinterpret_cast<const INDEX_TYPE*>(pattern3));
    svbool_t p_pat1 = VCMPNE(p32_all, idx1, UINT32_MAX);
    svbool_t p_pat2 = VCMPNE(p32_all, idx2, UINT32_MAX);
    svbool_t p_pat3 = VCMPNE(p32_all, idx3, UINT32_MAX);
    for(int i=0;i<nnz;i+=(VECTOR_LEN * 2)) {
        FLOAT* src = A + i;
        FLOAT* dst = C + i;
        
        VECREG vdata1 = VGATHER(p_pat1, src, idx1);
        VECREG vdata2 = VGATHER(p_pat2, src, idx2);
        VECREG vdata3 = VGATHER(p_pat3, src, idx3);
        
        FLOAT res1 = VREDUCTION(p_pat1, vdata1);
        FLOAT res2 = VREDUCTION(p_pat2, vdata2);
        FLOAT res3 = VREDUCTION(p_pat3, vdata3);
        
        dst[0] = res1;
        dst[1] = res2;
        dst[3] = res3;
    }
}
#endif

#if (defined USE_SINGLE && defined USE_512)
void reduction_with_1xload_permute_add(FLOAT* C, FLOAT* A, int nnz) {
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
        VECREG v1 = VLOAD(p32_all, A + i);
        // VECREG v2 = VLOAD(p32_all, A);
        
        VECREG v1_permed = VPERM(v1, idx1);
        VECREG v2 = VADD(p_v1_mask, v1, v1_permed);
        VECREG v2_permed = VPERM(v2, idx2);
        VECREG v3 = VADD(p_v2_mask, v2, v2_permed);
        VECREG res = VPERM(v3, idx3);
        
        VSTORE(p32_all, C + i, res);
    }
}
#elif ((defined USE_DOUBLE && defined USE_512) || (defined USE_SINGLE && defined USE_256))
void reduction_with_1xload_permute_add(FLOAT* C, FLOAT* A, int nnz) {
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
        VECREG v1 = VLOAD(p32_all, A + i);
        // VECREG v2 = VLOAD(p32_all, A);
        
        VECREG v1_permed = VPERM(v1, idx1);
        VECREG v2 = VADD(p_v1_mask, v1, v1_permed);
        VECREG v2_permed = VPERM(v2, idx2);
        VECREG v3 = VADD(p_v2_mask, v2, v2_permed);
        VECREG res = VPERM(v3, idx3);
        
        VSTORE(p32_all, C + i, res);
    }
}
#elif (defined USE_DOUBLE && defined USE_256)
void reduction_with_1xload_permute_add(FLOAT* C, FLOAT* A, int nnz) {
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
        VECREG v1 = VLOAD(p32_all, A + i);
        VECREG v2 = VLOAD(p32_all, A + i + 4);
        // VECREG v2 = VLOAD(p32_all, A);
        
        VECREG v1_permed = VPERM(v1, idx1);
        VECREG v2_permed = VPERM(v2, idx2); // [0, 6, 0, 0]
        VECREG v3 = VADD(p_v1_mask, v1_permed, v1); // so v3 will be [0+3, 1+2, 0, 0]
        VECREG v4 = VADD(p32_all, v2, v2_permed); // [4, 5 + 6, 6, 7]
        VECREG v4_permed = VPERM(v4, idx3); // so v4 will be [4, 7, 0, 5+6]
        VECREG res = VADD(p_v3_mask, v3, v4_permed);
        
        VSTORE(p32_all, C + i, res);
    }
}
#endif

int main()
{
    printf("vector size = %d!\n", VECTOR_LEN);
    int nnz = 8192;
    printf("Input nnz:\n");
    scanf("%d", &nnz);
    if (nnz % 16 != 0) {
        printf("Input nnz not multiples of 16!\n");
        exit(-1);
    }
    int res_len = nnz;
    FLOAT* C = (FLOAT*)malloc(res_len*sizeof(FLOAT));
    FLOAT* C2= (FLOAT*)malloc(res_len*sizeof(FLOAT));
    FLOAT* C3= (FLOAT*)malloc(res_len*sizeof(FLOAT));
    FLOAT* A = (FLOAT*)malloc(nnz*sizeof(FLOAT));    
    INDEX_TYPE* B = (INDEX_TYPE*)malloc(nnz*sizeof(INDEX_TYPE));
    printf("nnz = %d\n", nnz);

    memset(C, 0, res_len*sizeof(FLOAT));
    memset(C, 1, res_len*sizeof(FLOAT));
    memset(C, 2, res_len*sizeof(FLOAT));
    init_data(A, nnz);

#if (defined USE_SINGLE && defined USE_512)
    char *name_native = "data/reduction_native_512s.dat";
    char *name_gather_reduce = "data/reduction_gather_reduce_512s.dat";
    char *name_dynvec = "data/reduction_dynvec_512s.dat";
#elif (defined USE_SINGLE && defined USE_256)
    char *name_native = "data/reduction_native_256s.dat";
    char *name_gather_reduce = "data/reduction_gather_reduce_256s.dat";
    char *name_dynvec = "data/reduction_dynvec_256s.dat";
#elif (defined USE_DOUBLE && defined USE_512)
    char *name_native = "data/reduction_native_512d.dat";
    char *name_gather_reduce = "data/reduction_gather_reduce_512d.dat";
    char *name_dynvec = "data/reduction_dynvec_512d.dat";
#elif (defined USE_DOUBLE && defined USE_256)
    char *name_native = "data/reduction_native_256d.dat";
    char *name_gather_reduce = "data/reduction_gather_reduce_256d.dat";
    char *name_dynvec = "data/reduction_dynvec_256d.dat";
#endif

    EVAL_REDUCTION(reduction_native_c, C, A, nnz, name_native);
    EVAL_REDUCTION(reduction_gather_reduction, C2, A, nnz, name_gather_reduce);
    check(C, C2, res_len);
    EVAL_REDUCTION(reduction_with_1xload_permute_add, C3, A, nnz, name_dynvec);
    check(C, C3, res_len);
    
    return 0;
}
