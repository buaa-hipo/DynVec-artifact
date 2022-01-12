#include "common.h"
#include "eval.h"

void read_pure_inc(FLOAT* C, FLOAT* A, int* B, VECREG_I* perm, SELECT_T* select, int nnz) {
    // in our test, the nnz must be multiple times of VECTOR_LEN, so no need for protection
    for(int i=0;i<nnz;i++) {
        C[i] = A[i];
    }
}

void read_pure(FLOAT* C, FLOAT* A, int* B, VECREG_I* perm, SELECT_T* select, int nnz) {
    // in our test, the nnz must be multiple times of VECTOR_LEN, so no need for protection
    for(int i=0;i<nnz;i++) {
        C[i] = A[B[i]];
    }
}

void read_with_gather(FLOAT* C, FLOAT* A, int* B, VECREG_I* perm, SELECT_T* select, int nnz) {
    // in our test, the nnz must be multiple times of VECTOR_LEN, so no need for protection
    for(int i=0;i<nnz;i+=VECTOR_LEN) {
        // Read A[B[i]] 
        VECREG_GI ind = VLOAD_GI(reinterpret_cast<VECREG_GI*>(B+i));
        VECREG ymm1 = VGATHER(A, ind, sizeof(FLOAT));
        VSTORE(C+i, ymm1);
    }
}

// regular increment with no equalivilance
void read_with_1xload(FLOAT* C, FLOAT* A, int* B, VECREG_I* perm, SELECT_T* select, int nnz) {
    // #pragma unroll 4
    for(int i=0, j=0;i<nnz;i+=VECTOR_LEN, ++j) {
        //VECREG ymm1 = VLOAD(A+B[j]);
        VECREG ymm1 = VLOAD(A+B[j]);
        VSTORE(C+i, ymm1);
    }
}

// regular increment with equalivilance
void read_with_1xload_1xperm(FLOAT* C, FLOAT* A, int* B, VECREG_I* perm, SELECT_T* select, int nnz) {
    // for(int i=0;i<nnz;i+=VECTOR_LEN) {
    //     VECREG ymm1 = VLOAD(A+B[i]);
    for(int i=0, j=0;i<nnz;i+=VECTOR_LEN, ++j) {
        VECREG ymm1 = VLOAD(A+B[j]);
        VECREG_I idx = VLOAD_I(perm);
        VECREG ymm2 = VPERM(ymm1, idx);
        VSTORE(C+i, ymm2);
    }
}

void read_with_2xload(FLOAT* C, FLOAT* A, int* B, VECREG_I* perm, SELECT_T* select, int nnz) {
    //for(int i=0;i<nnz;i+=VECTOR_LEN) {
    for(int i=0, j=0;i<nnz;i+=VECTOR_LEN, j+=2) {
        VECREG ymm1 = VLOAD(A+B[j]);
        VECREG ymm2 = VLOAD(A+B[j+1]);
        VECREG_I idx1 = VLOAD_I(perm);
        VECREG_I idx2 = VLOAD_I(perm+1);
        VECREG ymm1_ = VPERM(ymm1, idx1);
        VECREG ymm2_ = VPERM(ymm2, idx2);
        // select
        //VECREG sel = VSET32(SELECT_0);
        VECREG res = reinterpret_cast<VECREG>(VBLEND_I(reinterpret_cast<VECREG_I>(ymm1_), reinterpret_cast<VECREG_I>(ymm2_), select[0]));
        VSTORE(C+i, res);
    }
}

void read_with_3xload(FLOAT* C, FLOAT* A, int* B, VECREG_I* perm, SELECT_T* select, int nnz) {
    //for(int i=0;i<nnz;i+=VECTOR_LEN) {
    for(int i=0, j=0;i<nnz;i+=VECTOR_LEN, j+=3) {
        VECREG ymm1 = VLOAD(A+B[j]);
        VECREG ymm2 = VLOAD(A+B[j+1]);
        VECREG ymm3 = VLOAD(A+B[j+2]);
        VECREG_I idx1 = VLOAD_I(perm);
        VECREG_I idx2 = VLOAD_I(perm+1);
        VECREG_I idx3 = VLOAD_I(perm+2);
        VECREG ymm1_ = VPERM(ymm1, idx1);
        VECREG ymm2_ = VPERM(ymm2, idx2);
        VECREG ymm3_ = VPERM(ymm3, idx3);
        // select
        VECREG res0 = reinterpret_cast<VECREG>(VBLEND_I(reinterpret_cast<VECREG_I>(ymm1_), reinterpret_cast<VECREG_I>(ymm2_), select[0]));
        VECREG res1 = reinterpret_cast<VECREG>(VBLEND_I(reinterpret_cast<VECREG_I>(res0), reinterpret_cast<VECREG_I>(ymm3_), select[1]));
        VSTORE(C+i, res1);
    }
}

void read_with_4xload(FLOAT* C, FLOAT* A, int* B, VECREG_I* perm, SELECT_T* select, int nnz) {
    //for(int i=0;i<nnz;i+=VECTOR_LEN) {
    for(int i=0, j=0;i<nnz;i+=VECTOR_LEN, j+=4) {
        VECREG ymm1 = VLOAD(A+B[j]);
        VECREG ymm2 = VLOAD(A+B[j+1]);
        VECREG ymm3 = VLOAD(A+B[j+2]);
        VECREG ymm4 = VLOAD(A+B[j+3]);
        VECREG_I idx1 = VLOAD_I(perm);
        VECREG_I idx2 = VLOAD_I(perm+1);
        VECREG_I idx3 = VLOAD_I(perm+2);
        VECREG_I idx4 = VLOAD_I(perm+3);
        VECREG ymm1_ = VPERM(ymm1, idx1);
        VECREG ymm2_ = VPERM(ymm2, idx2);
        VECREG ymm3_ = VPERM(ymm3, idx3);
        VECREG ymm4_ = VPERM(ymm4, idx4);
        // select
        VECREG res0 = reinterpret_cast<VECREG>(VBLEND_I(reinterpret_cast<VECREG_I>(ymm1_), reinterpret_cast<VECREG_I>(ymm2_), select[0]));
        VECREG res1 = reinterpret_cast<VECREG>(VBLEND_I(reinterpret_cast<VECREG_I>(ymm3_), reinterpret_cast<VECREG_I>(ymm4_), select[1]));
        VECREG res = reinterpret_cast<VECREG>(VBLEND_I(reinterpret_cast<VECREG_I>(res0), reinterpret_cast<VECREG_I>(res1), select[2]));
        VSTORE(C+i, res);
    }
}

void read_with_5xload(FLOAT* C, FLOAT* A, int* B, VECREG_I* perm, SELECT_T* select, int nnz) {
    //for(int i=0;i<nnz;i+=VECTOR_LEN) {
    for(int i=0, j=0;i<nnz;i+=VECTOR_LEN, j+=5) {
        VECREG ymm1 = VLOAD(A+B[j]);
        VECREG ymm2 = VLOAD(A+B[j+1]);
        VECREG ymm3 = VLOAD(A+B[j+2]);
        VECREG ymm4 = VLOAD(A+B[j+3]);
        VECREG ymm5 = VLOAD(A+B[j+4]);
        VECREG_I idx1 = VLOAD_I(perm);
        VECREG_I idx2 = VLOAD_I(perm+1);
        VECREG_I idx3 = VLOAD_I(perm+2);
        VECREG_I idx4 = VLOAD_I(perm+3);
        VECREG_I idx5 = VLOAD_I(perm+4);
        VECREG ymm1_ = VPERM(ymm1, idx1);
        VECREG ymm2_ = VPERM(ymm2, idx2);
        VECREG ymm3_ = VPERM(ymm3, idx3);
        VECREG ymm4_ = VPERM(ymm4, idx4);
        VECREG ymm5_ = VPERM(ymm5, idx5);
        // select
        VECREG res0 = reinterpret_cast<VECREG>(VBLEND_I(reinterpret_cast<VECREG_I>(ymm1_), reinterpret_cast<VECREG_I>(ymm2_), select[0]));
        VECREG res1 = reinterpret_cast<VECREG>(VBLEND_I(reinterpret_cast<VECREG_I>(ymm3_), reinterpret_cast<VECREG_I>(ymm4_), select[1]));
        VECREG res2 = reinterpret_cast<VECREG>(VBLEND_I(reinterpret_cast<VECREG_I>(res0), reinterpret_cast<VECREG_I>(res1), select[2]));
        VECREG res = reinterpret_cast<VECREG>(VBLEND_I(reinterpret_cast<VECREG_I>(res2), reinterpret_cast<VECREG_I>(ymm5_), select[3]));
        VSTORE(C+i, res);
    }
}

void read_with_6xload(FLOAT* C, FLOAT* A, int* B, VECREG_I* perm, SELECT_T* select, int nnz) {
    //for(int i=0;i<nnz;i+=VECTOR_LEN) {
    for(int i=0, j=0;i<nnz;i+=VECTOR_LEN, j+=6) {
        VECREG ymm1 = VLOAD(A+B[j]);
        VECREG ymm2 = VLOAD(A+B[j+1]);
        VECREG ymm3 = VLOAD(A+B[j+2]);
        VECREG ymm4 = VLOAD(A+B[j+3]);
        VECREG ymm5 = VLOAD(A+B[j+4]);
        VECREG ymm6 = VLOAD(A+B[j+5]);
        VECREG_I idx1 = VLOAD_I(perm);
        VECREG_I idx2 = VLOAD_I(perm+1);
        VECREG_I idx3 = VLOAD_I(perm+2);
        VECREG_I idx4 = VLOAD_I(perm+3);
        VECREG_I idx5 = VLOAD_I(perm+4);
        VECREG_I idx6 = VLOAD_I(perm+5);
        VECREG ymm1_ = VPERM(ymm1, idx1);
        VECREG ymm2_ = VPERM(ymm2, idx2);
        VECREG ymm3_ = VPERM(ymm3, idx3);
        VECREG ymm4_ = VPERM(ymm4, idx4);
        VECREG ymm5_ = VPERM(ymm5, idx5);
        VECREG ymm6_ = VPERM(ymm6, idx6);
        // select
        VECREG res0 = reinterpret_cast<VECREG>(VBLEND_I(reinterpret_cast<VECREG_I>(ymm1_), reinterpret_cast<VECREG_I>(ymm2_), select[0]));
        VECREG res1 = reinterpret_cast<VECREG>(VBLEND_I(reinterpret_cast<VECREG_I>(ymm3_), reinterpret_cast<VECREG_I>(ymm4_), select[1]));
        VECREG res2 = reinterpret_cast<VECREG>(VBLEND_I(reinterpret_cast<VECREG_I>(ymm5_), reinterpret_cast<VECREG_I>(ymm6_), select[2]));
        VECREG res3 = reinterpret_cast<VECREG>(VBLEND_I(reinterpret_cast<VECREG_I>(res0), reinterpret_cast<VECREG_I>(res1), select[3]));
        VECREG res = reinterpret_cast<VECREG>(VBLEND_I(reinterpret_cast<VECREG_I>(res2), reinterpret_cast<VECREG_I>(res3), select[4]));
        VSTORE(C+i, res);
    }
}

void read_with_7xload(FLOAT* C, FLOAT* A, int* B, VECREG_I* perm, SELECT_T* select, int nnz) {
    //for(int i=0;i<nnz;i+=VECTOR_LEN) {
    for(int i=0, j=0;i<nnz;i+=VECTOR_LEN, j+=7) {
        VECREG ymm1 = VLOAD(A+B[j]);
        VECREG ymm2 = VLOAD(A+B[j+1]);
        VECREG ymm3 = VLOAD(A+B[j+2]);
        VECREG ymm4 = VLOAD(A+B[j+3]);
        VECREG ymm5 = VLOAD(A+B[j+4]);
        VECREG ymm6 = VLOAD(A+B[j+5]);
        VECREG ymm7 = VLOAD(A+B[j+6]);
        VECREG_I idx1 = VLOAD_I(perm+0);
        VECREG_I idx2 = VLOAD_I(perm+1);
        VECREG_I idx3 = VLOAD_I(perm+2);
        VECREG_I idx4 = VLOAD_I(perm+3);
        VECREG_I idx5 = VLOAD_I(perm+4);
        VECREG_I idx6 = VLOAD_I(perm+5);
        VECREG_I idx7 = VLOAD_I(perm+6);
        VECREG ymm1_ = VPERM(ymm1, idx1);
        VECREG ymm2_ = VPERM(ymm2, idx2);
        VECREG ymm3_ = VPERM(ymm3, idx3);
        VECREG ymm4_ = VPERM(ymm4, idx4);
        VECREG ymm5_ = VPERM(ymm5, idx5);
        VECREG ymm6_ = VPERM(ymm6, idx6);
        VECREG ymm7_ = VPERM(ymm7, idx7);
        // select
        VECREG res0 = reinterpret_cast<VECREG>(VBLEND_I(reinterpret_cast<VECREG_I>(ymm1_), reinterpret_cast<VECREG_I>(ymm2_), select[0]));
        VECREG res1 = reinterpret_cast<VECREG>(VBLEND_I(reinterpret_cast<VECREG_I>(ymm3_), reinterpret_cast<VECREG_I>(ymm4_), select[1]));
        VECREG res2 = reinterpret_cast<VECREG>(VBLEND_I(reinterpret_cast<VECREG_I>(ymm5_), reinterpret_cast<VECREG_I>(ymm6_), select[2]));
        // 0,1,2,3
        VECREG res3 = reinterpret_cast<VECREG>(VBLEND_I(reinterpret_cast<VECREG_I>(res0), reinterpret_cast<VECREG_I>(res1), select[3]));
        // 4,5,6
        VECREG res4 = reinterpret_cast<VECREG>(VBLEND_I(reinterpret_cast<VECREG_I>(res2), reinterpret_cast<VECREG_I>(ymm7_), select[4]));
        // all
        VECREG res = reinterpret_cast<VECREG>(VBLEND_I(reinterpret_cast<VECREG_I>(res3), reinterpret_cast<VECREG_I>(res4), select[5]));
        VSTORE(C+i, res);
    }
}

void read_with_8xload(FLOAT* C, FLOAT* A, int* B, VECREG_I* perm, SELECT_T* select, int nnz) {
    //for(int i=0;i<nnz;i+=VECTOR_LEN) {
    for(int i=0, j=0;i<nnz;i+=VECTOR_LEN, j+=8) {
        VECREG ymm1 = VLOAD(A+B[j]);
        VECREG ymm2 = VLOAD(A+B[j+1]);
        VECREG ymm3 = VLOAD(A+B[j+2]);
        VECREG ymm4 = VLOAD(A+B[j+3]);
        VECREG ymm5 = VLOAD(A+B[j+4]);
        VECREG ymm6 = VLOAD(A+B[j+5]);
        VECREG ymm7 = VLOAD(A+B[j+6]);
        VECREG ymm8 = VLOAD(A+B[j+7]);
        VECREG_I idx1 = VLOAD_I(perm+0);
        VECREG_I idx2 = VLOAD_I(perm+1);
        VECREG_I idx3 = VLOAD_I(perm+2);
        VECREG_I idx4 = VLOAD_I(perm+3);
        VECREG_I idx5 = VLOAD_I(perm+4);
        VECREG_I idx6 = VLOAD_I(perm+5);
        VECREG_I idx7 = VLOAD_I(perm+6);
        VECREG_I idx8 = VLOAD_I(perm+7);
        VECREG ymm1_ = VPERM(ymm1, idx1);
        VECREG ymm2_ = VPERM(ymm2, idx2);
        VECREG ymm3_ = VPERM(ymm3, idx3);
        VECREG ymm4_ = VPERM(ymm4, idx4);
        VECREG ymm5_ = VPERM(ymm5, idx5);
        VECREG ymm6_ = VPERM(ymm6, idx6);
        VECREG ymm7_ = VPERM(ymm7, idx7);
        VECREG ymm8_ = VPERM(ymm8, idx8);
        // select
        VECREG res0 = reinterpret_cast<VECREG>(VBLEND_I(reinterpret_cast<VECREG_I>(ymm1_), reinterpret_cast<VECREG_I>(ymm2_), select[0]));
        VECREG res1 = reinterpret_cast<VECREG>(VBLEND_I(reinterpret_cast<VECREG_I>(ymm3_), reinterpret_cast<VECREG_I>(ymm4_), select[1]));
        VECREG res2 = reinterpret_cast<VECREG>(VBLEND_I(reinterpret_cast<VECREG_I>(ymm5_), reinterpret_cast<VECREG_I>(ymm6_), select[2]));
        VECREG res3 = reinterpret_cast<VECREG>(VBLEND_I(reinterpret_cast<VECREG_I>(ymm7_), reinterpret_cast<VECREG_I>(ymm8_), select[3]));
        // merge two
        VECREG res4 = reinterpret_cast<VECREG>(VBLEND_I(reinterpret_cast<VECREG_I>(res0), reinterpret_cast<VECREG_I>(res1), select[4]));
        VECREG res5 = reinterpret_cast<VECREG>(VBLEND_I(reinterpret_cast<VECREG_I>(res2), reinterpret_cast<VECREG_I>(res3), select[5]));
        // final merge
        VECREG res = reinterpret_cast<VECREG>(VBLEND_I(reinterpret_cast<VECREG_I>(res4), reinterpret_cast<VECREG_I>(res5), select[6]));
        VSTORE(C+i, res);
    }
}

#ifdef USE_AVX512

#define LOAD_1(A, B, i) \
    VECREG ymm##i = VLOAD(A+B[j+i]); \
    VECREG_I idx##i = VLOAD_I(perm+i); \
    VECREG _ymm##i = VPERM(ymm##i, idx##i);

#define LOAD_1xto(A, B, i, res) \
    VECREG ymm##i = VLOAD(A+B[j+i]); \
    VECREG_I idx##i = VLOAD_I(perm+i); \
    VECREG res = VPERM(ymm##i, idx##i);

#define LOAD_2(A, B,i1,i2,k,res) \
    LOAD_1(A,B,i1) \
    LOAD_1(A,B,i2) \
    VECREG res = reinterpret_cast<VECREG>(VBLEND_I(reinterpret_cast<VECREG_I>(_ymm##i1), reinterpret_cast<VECREG_I>(_ymm##i2), select[k]));

#define MERGE_2(res, res0, res1, k) \
    VECREG res = reinterpret_cast<VECREG>(VBLEND_I(reinterpret_cast<VECREG_I>(res0), reinterpret_cast<VECREG_I>(res1), select[k]));

void read_with_9xload(FLOAT* C, FLOAT* A, int* B, VECREG_I* perm, SELECT_T* select, int nnz) {
    //for(int i=0;i<nnz;i+=VECTOR_LEN) {
    for(int i=0, j=0;i<nnz;i+=VECTOR_LEN, j+=9) {
        LOAD_2(A,B,0,1,0,res0)
        LOAD_2(A,B,2,3,1,res1)
        LOAD_2(A,B,4,5,2,res2)
        LOAD_2(A,B,6,7,3,res3)
        LOAD_1xto(A,B,8,res4)
        MERGE_2(_res0, res0, res1, 4)
        MERGE_2(_res1, res2, res3, 5)
        MERGE_2(_res2,_res0,_res1, 6)
        MERGE_2( res ,_res2, res4, 7)
        VSTORE(C+i, res);
    }
}

void read_with_10xload(FLOAT* C, FLOAT* A, int* B, VECREG_I* perm, SELECT_T* select, int nnz) {
    //for(int i=0;i<nnz;i+=VECTOR_LEN) {
    for(int i=0, j=0;i<nnz;i+=VECTOR_LEN, j+=10) {
        LOAD_2(A,B,0,1,0,res0)
        LOAD_2(A,B,2,3,1,res1)
        LOAD_2(A,B,4,5,2,res2)
        LOAD_2(A,B,6,7,3,res3)
        LOAD_2(A,B,8,9,4,res4)
        MERGE_2(_res0, res0, res1, 5)
        MERGE_2(_res1, res2, res3, 6)
        MERGE_2(_res2,_res0,_res1, 7)
        MERGE_2( res ,_res2, res4, 8)
        VSTORE(C+i, res);
    }
}

void read_with_11xload(FLOAT* C, FLOAT* A, int* B, VECREG_I* perm, SELECT_T* select, int nnz) {
    //for(int i=0;i<nnz;i+=VECTOR_LEN) {
    for(int i=0, j=0;i<nnz;i+=VECTOR_LEN, j+=11) {
        LOAD_2(A,B,0,1,0,res0)
        LOAD_2(A,B,2,3,1,res1)
        LOAD_2(A,B,4,5,2,res2)
        LOAD_2(A,B,6,7,3,res3)
        LOAD_2(A,B,8,9,4,res4)
        LOAD_1xto(A,B,10,res5)
        MERGE_2(_res0, res0, res1, 5)
        MERGE_2(_res1, res2, res3, 6)
        MERGE_2(_res2, res4, res5, 7)
        MERGE_2(_res3,_res0,_res1, 8)
        MERGE_2 (res ,_res3,_res2, 9)
        VSTORE(C+i, res);
    }
}

void read_with_12xload(FLOAT* C, FLOAT* A, int* B, VECREG_I* perm, SELECT_T* select, int nnz) {
    //for(int i=0;i<nnz;i+=VECTOR_LEN) {
    for(int i=0, j=0;i<nnz;i+=VECTOR_LEN, j+=12) {
        LOAD_2(A,B,0,1,0,res0)
        LOAD_2(A,B,2,3,1,res1)
        LOAD_2(A,B,4,5,2,res2)
        LOAD_2(A,B,6,7,3,res3)
        LOAD_2(A,B,8,9,4,res4)
        LOAD_2(A,B,10,11,5,res5)
        MERGE_2(_res0, res0, res1, 6)
        MERGE_2(_res1, res2, res3, 7)
        MERGE_2(_res2, res4, res5, 8)
        MERGE_2(_res3,_res0,_res1, 9)
        MERGE_2( res ,_res3,_res2, 10)
        VSTORE(C+i, res);
    }
}

void read_with_13xload(FLOAT* C, FLOAT* A, int* B, VECREG_I* perm, SELECT_T* select, int nnz) {
    //for(int i=0;i<nnz;i+=VECTOR_LEN) {
    for(int i=0, j=0;i<nnz;i+=VECTOR_LEN, j+=13) {
        LOAD_2(A,B,0,1,0,res0)
        LOAD_2(A,B,2,3,1,res1)
        LOAD_2(A,B,4,5,2,res2)
        LOAD_2(A,B,6,7,3,res3)
        LOAD_2(A,B,8,9,4,res4)
        LOAD_2(A,B,10,11,5,res5)
        LOAD_1xto(A,B,12,res6)
        MERGE_2(_res0, res0, res1, 6)
        MERGE_2(_res1, res2, res3, 7)
        MERGE_2(_res2, res4, res5, 8)
        MERGE_2(_res3,_res0,_res1, 9)
        MERGE_2(_res4,_res3,_res2, 10)
        MERGE_2( res ,_res4, res6, 11)
        VSTORE(C+i, res);
    }
}

void read_with_14xload(FLOAT* C, FLOAT* A, int* B, VECREG_I* perm, SELECT_T* select, int nnz) {
    //for(int i=0;i<nnz;i+=VECTOR_LEN) {
    for(int i=0, j=0;i<nnz;i+=VECTOR_LEN, j+=14) {
        LOAD_2(A,B,0,1,0,res0)
        LOAD_2(A,B,2,3,1,res1)
        LOAD_2(A,B,4,5,2,res2)
        LOAD_2(A,B,6,7,3,res3)
        LOAD_2(A,B,8,9,4,res4)
        LOAD_2(A,B,10,11,5,res5)
        LOAD_2(A,B,12,13,6,res6)
        MERGE_2(_res0, res0, res1, 7)
        MERGE_2(_res1, res2, res3, 8)
        MERGE_2(_res2, res4, res5, 9)
        MERGE_2(_res3,_res0,_res1, 10)
        MERGE_2(_res4,_res3,_res2, 11)
        MERGE_2( res ,_res4, res6, 12)
        VSTORE(C+i, res);
    }
}

void read_with_15xload(FLOAT* C, FLOAT* A, int* B, VECREG_I* perm, SELECT_T* select, int nnz) {
    //for(int i=0;i<nnz;i+=VECTOR_LEN) {
    for(int i=0, j=0;i<nnz;i+=VECTOR_LEN, j+=15) {
        LOAD_2(A,B,0,1,0,res0)
        LOAD_2(A,B,2,3,1,res1)
        LOAD_2(A,B,4,5,2,res2)
        LOAD_2(A,B,6,7,3,res3)
        LOAD_2(A,B,8,9,4,res4)
        LOAD_2(A,B,10,11,5,res5)
        LOAD_2(A,B,12,13,6,res6)
        LOAD_1xto(A,B,14,res7)
        MERGE_2(_res0, res0, res1, 7)
        MERGE_2(_res1, res2, res3, 8)
        MERGE_2(_res2, res4, res5, 9)
        MERGE_2(_res3, res6, res7, 10)
        MERGE_2(_res4,_res0,_res1, 11)
        MERGE_2(_res5,_res2,_res3, 12)
        MERGE_2( res ,_res4,_res5, 13)
        VSTORE(C+i, res);
    }
}

void read_with_16xload(FLOAT* C, FLOAT* A, int* B, VECREG_I* perm, SELECT_T* select, int nnz) {
    //for(int i=0;i<nnz;i+=VECTOR_LEN) {
    for(int i=0, j=0;i<nnz;i+=VECTOR_LEN, j+=16) {
        LOAD_2(A,B,0,1,0,res0)
        LOAD_2(A,B,2,3,1,res1)
        LOAD_2(A,B,4,5,2,res2)
        LOAD_2(A,B,6,7,3,res3)
        LOAD_2(A,B,8,9,4,res4)
        LOAD_2(A,B,10,11,5,res5)
        LOAD_2(A,B,12,13,6,res6)
        LOAD_2(A,B,14,15,7,res7)
        MERGE_2(_res0, res0, res1, 8)
        MERGE_2(_res1, res2, res3, 9)
        MERGE_2(_res2, res4, res5, 10)
        MERGE_2(_res3, res6, res7, 11)
        MERGE_2(_res4,_res0,_res1, 12)
        MERGE_2(_res5,_res2,_res3, 13)
        MERGE_2( res ,_res4,_res5, 14)
        VSTORE(C+i, res);
    }
}

#endif

#define MANGLE_DATA(B, nnz, size)

#define READ_DATA(B, nnz, size) do {\
    char buff[10];\
    sprintf(buff, "B.%d.dat", size);\
    fp = fopen(buff, "rb");\
    fread(B, sizeof(int), nnz, fp);\
    fclose(fp); \
} while(0)

#define INIT(B, nnz, size) do {\
    for(int i=0;i<size*nnz/VECTOR_LEN;i+=size) { \
        for(int j=0;j<size;++j) { \
            B[i+j] = i*VECTOR_LEN/size+j*VECTOR_LEN; \
        } \
    } MANGLE_DATA(B,nnz, size); } while(0)

#define DEBUG(x) 

#define INIT_GATHER_INDEX(B, nnz, size) do {\
    DEBUG(printf("INIT INDEX for %d\n",size)); \
    for(int i=0;i<nnz;i+=VECTOR_LEN) { \
        for(int j=0;j<size;++j) { \
            B[i+j] = i+j*VECTOR_LEN; \
        } \
        for(int j=size;j<VECTOR_LEN;++j) { \
            B[i+j] = i+j; \
        } \
    } MANGLE_DATA(B,nnz, size); } while(0)

FILE* fp;

#define EVALUATE(test_load, k, gather_log, load_log) do {\
    INIT_GATHER_INDEX(B, nnz, k); \
    EVAL(read_with_gather, C, A, B, perm, select[k-2], nnz, gather_log); \
    INIT(B, nnz, k); \
    EVAL(test_load, C2, A, B, perm, select[k-2], nnz, gather_log); \
    check(C, C2, nnz); } while (0)\

int main() {
    printf("-------- Evaluating Gather Optimization --------\n");
    printf("VECTOR_SIZE=%d, VECTOR_LEN=%lu\n",VECTOR_SIZE, VECTOR_LEN);
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
    init_para_perm(perm, GATHER_PERM_DAT_FN);
    init_para_select(select, SELECT_DAT_FN);
    FLOAT* C = (FLOAT*)malloc(TEST_SIZE*sizeof(FLOAT)); 
    FLOAT* C2= (FLOAT*)malloc(TEST_SIZE*sizeof(FLOAT)); 
    FLOAT* A = (FLOAT*)malloc((TEST_SIZE+VECTOR_LEN)*sizeof(FLOAT));
    int* B = (int*)malloc(TEST_SIZE*sizeof(FLOAT));
    int nnz = TEST_SIZE;
    for(int i=0;i<nnz;++i) {
        B[i] = i;
        A[i] = i+0.1;
    }
    //READ_DATA(B, nnz, 0);
#ifdef RUN_PURE
    EVAL(read_pure, C, A, B, perm, select[0], nnz, "data/pure.dat");
    EVAL(read_pure_inc, C2, A, B, perm, select[0], nnz, "data/pure_inc.dat");
    check(C, C2, nnz);

    EVAL(read_with_gather, C, A, B, perm, select[0], nnz, "data/gather.dat");
    for(int i=0;i<nnz/VECTOR_LEN;++i) {
        B[i] = i*VECTOR_LEN;
    }
    EVAL(read_with_1xload, C2, A, B, perm, select[0], nnz, "data/load.dat");
    check(C, C2, nnz);
#endif
    for(int i=0;i<nnz;i+=VECTOR_LEN) {
        for(int j=0;j<VECTOR_LEN;++j) {
            B[i+j] = i+VECTOR_LEN-j-1;
        }
    }
    //READ_DATA(B, nnz, 1);
    //EVAL(read_pure, C, A, B, perm, select[6], nnz, "data/pure_1.dat");
    EVAL(read_with_gather, C, A, B, perm, select[0], nnz, "data/gather_1.dat");
    for(int i=0;i<nnz/VECTOR_LEN;++i) {
        B[i] = i*VECTOR_LEN;
    }
    EVAL(read_with_1xload_1xperm, C2, A, B, perm2, select[0], nnz, "data/load_1.dat");
    check(C, C2, nnz);
    // 2
    INIT_GATHER_INDEX(B, nnz, 2);
    //READ_DATA(B, nnz, 2);
    //EVAL(read_pure, C, A, B, perm, select[6], nnz, "data/pure_2.dat");
    EVAL(read_with_gather, C, A, B, perm, select[0], nnz, "data/gather_2.dat");
    INIT(B, nnz, 2);
    EVAL(read_with_2xload, C2, A, B, perm, select[0], nnz, "data/load_2.dat");
    check(C, C2, nnz);
    // 3
    INIT_GATHER_INDEX(B, nnz, 3);
    //READ_DATA(B, nnz, 3);
    //EVAL(read_pure, C, A, B, perm, select[6], nnz, "data/pure_3.dat");
    EVAL(read_with_gather, C, A, B, perm, select[1], nnz, "data/gather_3.dat");
    INIT(B, nnz, 3);
    EVAL(read_with_3xload, C2, A, B, perm, select[1], nnz, "data/load_3.dat");
    check(C, C2, nnz);
    // 4
    INIT_GATHER_INDEX(B, nnz, 4);
    //READ_DATA(B, nnz, 4);
    //EVAL(read_pure, C, A, B, perm, select[6], nnz, "data/pure_4.dat");
    EVAL(read_with_gather, C, A, B, perm, select[2], nnz, "data/gather_4.dat");
    INIT(B, nnz, 4);
    EVAL(read_with_4xload, C2, A, B, perm, select[2], nnz, "data/load_4.dat");
    check(C, C2, nnz);
    if(VECTOR_LEN>4) {
      // 5
      INIT_GATHER_INDEX(B, nnz, 5);
      //READ_DATA(B, nnz, 5);
      //EVAL(read_pure, C, A, B, perm, select[6], nnz, "data/pure_5.dat");
      EVAL(read_with_gather, C, A, B, perm, select[3], nnz, "data/gather_5.dat");
      INIT(B, nnz, 5);
      EVAL(read_with_5xload, C2, A, B, perm, select[3], nnz, "data/load_5.dat");
      check(C, C2, nnz);
      // 6
      INIT_GATHER_INDEX(B, nnz, 6);
      //READ_DATA(B, nnz, 6);
      //EVAL(read_pure, C, A, B, perm, select[6], nnz, "data/pure_6.dat");
      EVAL(read_with_gather, C, A, B, perm, select[4], nnz, "data/gather_6.dat");
      INIT(B, nnz, 6);
      EVAL(read_with_6xload, C2, A, B, perm, select[4], nnz, "data/load_6.dat");
      check(C, C2, nnz);
      // 7
      INIT_GATHER_INDEX(B, nnz, 7);
      //READ_DATA(B, nnz, 7);
      //EVAL(read_pure, C, A, B, perm, select[6], nnz, "data/pure_7.dat");
      EVAL(read_with_gather, C, A, B, perm, select[5], nnz, "data/gather_7.dat");
      INIT(B, nnz, 7);
      EVAL(read_with_7xload, C2, A, B, perm, select[5], nnz, "data/load_7.dat");
      check(C, C2, nnz);
      // 8
      INIT_GATHER_INDEX(B, nnz, 8);
      //READ_DATA(B, nnz, 8);
      //EVAL(read_pure, C, A, B, perm, select[6], nnz, "data/pure_8.dat");
      EVAL(read_with_gather, C, A, B, perm, select[6], nnz, "data/gather_8.dat");
      INIT(B, nnz, 8);
      EVAL(read_with_8xload, C2, A, B, perm, select[6], nnz, "data/load_8.dat");
      check(C, C2, nnz);
    } /* VECTOR_LEN > 4 */
#ifdef USE_AVX512
    if(VECTOR_LEN>8) {
        EVALUATE(read_with_9xload,  9, "data/gather_9.dat", "data/load_9.dat");
        EVALUATE(read_with_10xload,10, "data/gather_10.dat", "data/load_10.dat");
        EVALUATE(read_with_11xload,11, "data/gather_11.dat", "data/load_11.dat");
        EVALUATE(read_with_12xload,12, "data/gather_12.dat", "data/load_12.dat");
        EVALUATE(read_with_13xload,13, "data/gather_13.dat", "data/load_13.dat");
        EVALUATE(read_with_14xload,14, "data/gather_14.dat", "data/load_14.dat");
        EVALUATE(read_with_15xload,15, "data/gather_15.dat", "data/load_15.dat");
        EVALUATE(read_with_16xload,16, "data/gather_16.dat", "data/load_16.dat");
    }
#endif
    return 0;
}
