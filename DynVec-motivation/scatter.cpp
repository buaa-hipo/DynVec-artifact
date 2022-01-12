#include "common.h"
#include "eval.h"

void store_pure_inc(FLOAT* C, FLOAT* A, int* B, VECREG_I* perm, SELECT_T* select, int nnz) {
    // in our test, the nnz must be multiple times of VECTOR_LEN, so no need for protection
    for(int i=0;i<nnz;i++) {
        C[i] = A[i];
    }
}

void store_pure(FLOAT* C, FLOAT* A, int* B, VECREG_I* perm, SELECT_T* select, int nnz) {
    // in our test, the nnz must be multiple times of VECTOR_LEN, so no need for protection
    for(int i=0;i<nnz;i++) {
        C[B[i]] = A[i];
    }
}

// D[Idx[:]] = V[:]
void scatter(FLOAT* C, FLOAT* A, int* B, VECREG_I* perm, SELECT_T* select, int nnz) {
    /* There are no scatter instruction support in AVX2. It is only supported from AVX512F */
#ifdef USE_AVX512
    for(int i=0;i<nnz;i+=VECTOR_LEN) {
        VECREG ymm1 = VLOAD(A+i);
        // Store C[B[i]] 
        VECREG_GI ind = VLOAD_GI(reinterpret_cast<VECREG_GI*>(B+i));
        VSCATTER(C, ind, ymm1, sizeof(FLOAT));
    }
#else
    printf("Skip scatter test as there are no scatter support in AVX2.");
#endif
}

void scatter_opt_1xstore(FLOAT* C, FLOAT* A, int* B, VECREG_I* perm, SELECT_T* select, int nnz) {
    for(int i=0, j=0;i<nnz;i+=VECTOR_LEN, ++j) {
        VECREG ymm1 = VLOAD(A+i);
        VECREG_I idx = VLOAD_I(perm);
        VECREG ymm2 = VPERM(ymm1, idx);
        VSTORE(C+B[j], ymm2);
    }
}

void scatter_opt_1xstore_inc(FLOAT* C, FLOAT* A, int* B, VECREG_I* perm, SELECT_T* select, int nnz) {
    for(int i=0, j=0;i<nnz;i+=VECTOR_LEN, ++j) {
        VECREG ymm1 = VLOAD(A+i);
        VSTORE(C+B[j], ymm1);
    }
}

FILE* fp;

int main() {
    printf("-------- Evaluating Scatter Optimization --------\n");
    printf("VECTOR_SIZE=%d, VECTOR_LEN=%lu\n",VECTOR_SIZE, VECTOR_LEN);
    print_info();
#ifdef USE_AVX512
    SELECT_T select[15][15];
    int perm[VECTOR_LEN][16];
#ifdef USE_DOUBLE
    uint64_t perm2[] = { 7,6,5,4,3,2,1,0 };
#else
    uint32_t perm2[] = { 15,14,13,12,11,10,9,8,7,6,5,4,3,2,1,0 };
#endif
#else // Not USE_AVX512
    SELECT_T select[7][7];
    int perm[VECTOR_LEN][8];
#ifdef USE_DOUBLE
    int perm2[] = { 6,7,4,5,2,3,0,1 };
#else
    int perm2[] = { 7,6,5,4,3,2,1,0 };
#endif
#endif
    init_para_perm(perm, SCATTER_PERM_DAT_FN);
    init_para_select(select, SELECT_DAT_FN);
    FLOAT* C = (FLOAT*)malloc(TEST_SIZE*sizeof(FLOAT)); 
    FLOAT* C2= (FLOAT*)malloc(TEST_SIZE*sizeof(FLOAT)); 
    FLOAT* A = (FLOAT*)malloc((TEST_SIZE+VECTOR_LEN)*sizeof(FLOAT));
    int* B = (int*)malloc(TEST_SIZE*sizeof(FLOAT));
    int* B2= (int*)malloc(TEST_SIZE*sizeof(FLOAT));
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
    EVAL(store_pure, C, A, B, perm, select[6], nnz, "data/scatter_pure_1.dat");
#ifdef USE_AVX512
    EVAL(scatter, C, A, B, perm, select[0], nnz, "data/scatter_1.dat");
#endif
    EVAL(scatter_opt_1xstore, C2, A, B2, perm2, select[0], nnz, "data/store_1.dat");
    check(C, C2, nnz);
}
