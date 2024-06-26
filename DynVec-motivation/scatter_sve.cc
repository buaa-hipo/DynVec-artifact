#include "common.h"
#include "eval.h"

void store_pure_inc(FLOAT* C, FLOAT* A, INDEX_TYPE* B, PERM_T* perm, SELECT_T* select, int nnz) {
    // in our test, the nnz must be multiple times of VECTOR_LEN, so no need for protection
    for(int i=0;i<nnz;i++) {
        C[i] = A[i];
    }
}

void store_pure(FLOAT* C, FLOAT* A, INDEX_TYPE* B, PERM_T* perm, SELECT_T* select, int nnz) {
    // in our test, the nnz must be multiple times of VECTOR_LEN, so no need for protection
    for(int i=0;i<nnz;i++) {
        C[B[i]] = A[i];
    }
}

// D[Idx[:]] = V[:]
void scatter(FLOAT* C, FLOAT* A, INDEX_TYPE* B, PERM_T* perm, SELECT_T* select, int nnz) {
    svbool_t p32_all = svptrue_b32();
    for(int i=0;i<nnz;i+=VECTOR_LEN) {
        VECREG v1 = VLOAD(p32_all, A+i);
        // Store C[B[i]] 
        VECREG_GI ind = VLOAD_GI(p32_all, reinterpret_cast<const INDEX_TYPE*>(B+i));
        VSCATTER(p32_all, C, ind, v1);
    }
}

void scatter_opt_1xstore(FLOAT* C, FLOAT* A, INDEX_TYPE* B, PERM_T* perm, SELECT_T* select, int nnz) {
    svbool_t p32_all = svptrue_b32();
    for(int i=0, j=0;i<nnz;i+=VECTOR_LEN, ++j) {
        VECREG v1 = VLOAD(p32_all, A+i);
        VECREG_I idx = VLOAD_I(p32_all, reinterpret_cast<const INDEX_TYPE*>(perm));
        VECREG v2 = VPERM(v1, idx);
        VSTORE(p32_all, C+B[j], v2);
    }
}

void scatter_opt_1xstore_inc(FLOAT* C, FLOAT* A, INDEX_TYPE* B, PERM_T* perm, SELECT_T* select, int nnz) {
    svbool_t p32_all = svptrue_b32();
    for(int i=0, j=0;i<nnz;i+=VECTOR_LEN, ++j) {
        VECREG v1 = VLOAD(p32_all, A+i);
        VSTORE(p32_all, C+B[j], v1);
    }
}

FILE* fp;

int main() {
    printf("-------- Evaluating Scatter Optimization --------\n");
    printf("VECTOR_LEN=%d\n",VECTOR_LEN);
    print_info();
#ifdef USE_512
#ifdef USE_DOUBLE
    int64_t perm2[] = { 7,6,5,4,3,2,1,0 };
#else
    int32_t perm2[] = { 15,14,13,12,11,10,9,8,7,6,5,4,3,2,1,0 };
#endif
#else // Not USE_AVX512
#ifdef USE_DOUBLE
    int64_t perm2[] = { 3, 2, 1, 0 };
#else
    int32_t perm2[] = { 7,6,5,4,3,2,1,0 };
#endif
#endif
#if defined USE_256
    SELECT_T select[8][8];
    PERM_T perm[8][8];
#else
    #if defined USE_DOUBLE
        SELECT_T select[8][8];
        PERM_T perm[8][8];
    #else
        SELECT_T select[16][16];
        PERM_T perm[16][16];
    #endif
#endif
    init_para_perm(perm, SCATTER_PERM_DAT_FN);
    init_para_select(select, SELECT_DAT_FN);
    FLOAT* C = (FLOAT*)malloc(TEST_SIZE*sizeof(FLOAT)); 
    FLOAT* C2= (FLOAT*)malloc(TEST_SIZE*sizeof(FLOAT)); 
    FLOAT* A = (FLOAT*)malloc((TEST_SIZE+VECTOR_LEN)*sizeof(FLOAT));
    INDEX_TYPE* B = (INDEX_TYPE*)malloc(TEST_SIZE*sizeof(INDEX_TYPE));
    INDEX_TYPE* B2= (INDEX_TYPE*)malloc(TEST_SIZE*sizeof(INDEX_TYPE));
    int nnz = TEST_SIZE;
    for(int i=0;i<nnz;++i) {
        B[i] = i;
        A[i] = i+0.1;
    }
    printf("-------------------------------------------------\n");
    //READ_DATA(B, nnz, 0);
    for(int i=0;i<nnz;i+=VECTOR_LEN) {
        for(int j=0;j<VECTOR_LEN;++j) {
            B[i+j] = i+VECTOR_LEN-j-1;
        }
    }
    for(int i=0;i<nnz/VECTOR_LEN;++i) {
        B2[i] = i*VECTOR_LEN;
    }
    //READ_DATA(B, nnz, 1);
    EVAL(store_pure_inc, C, A, B, perm, select[0], nnz, "data/scatter_pure_inc.dat");
    EVAL(scatter_opt_1xstore_inc, C2, A, B2, perm2, select[0], nnz, "data/store_1_inc.dat");
    check(C, C2, nnz);
    EVAL(store_pure, C, A, B, perm, select[0], nnz, "data/scatter_pure_1.dat");
    EVAL(scatter_opt_1xstore, C2, A, B2, perm2, select[0], nnz, "data/store_1.dat");
    check(C, C2, nnz);
    EVAL(scatter, C, A, B, perm, select[0], nnz, "data/scatter_1.dat");
    EVAL(scatter_opt_1xstore, C2, A, B2, perm2, select[0], nnz, "data/store_1.dat");
    check(C, C2, nnz);
}
