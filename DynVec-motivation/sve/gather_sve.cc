#include "common.h"
#include "eval.h"

void init_data(FLOAT* A, FLOAT* B) {
    int nnz = TEST_SIZE;
    for(int i=0;i<nnz;++i) {
        B[i] = i;
        A[i] = i+0.1;
    }
}

void read_pure_inc(FLOAT* C, FLOAT* A, INDEX_TYPE* B, PERM_T* perm, SELECT_T* select, int nnz) {
    // in our test, the nnz must be multiple times of VECTOR_LEN, so no need for protection
    for(int i=0;i<nnz;i++) {
        C[i] = A[i];
    }
}

void read_pure(FLOAT* C, FLOAT* A, INDEX_TYPE* B, PERM_T* perm, SELECT_T* select, int nnz) {
    // in our test, the nnz must be multiple times of VECTOR_LEN, so no need for protection
    for(int i=0;i<nnz;i++) {
        C[i] = A[B[i]];
    }
}

void read_with_gather(FLOAT* C, FLOAT* A, INDEX_TYPE* B, PERM_T* perm, SELECT_T* select, int nnz) {
    // in our test, the nnz must be multiple times of VECTOR_LEN, so no need for protection
    svbool_t p32_all = svptrue_b32();
    // printf("nnz = %d", nnz);
    // for(int i = 0; i < nnz; i++) printf("%d ", B[i]);
    // printf("\n");
    for(int i=0;i<nnz;i+=VECTOR_LEN) {
        // Read A[B[i]] 
        VECREG_GI ind = VLOAD_GI(p32_all, reinterpret_cast<const INDEX_TYPE*>(B+i));

	/*
        printf("\nB\n");
        for(int j = 0; j < 16; j++) printf("%d ", (B+i)[j]);
        printf("\nread B\n");
        for(int j = 0; j < 16; j++) printf("%d ", readb[j]);
        printf("\n");
	*/
        VECREG v1 = VGATHER(p32_all, A, ind);
	//float m = A[B[i]];
        VSTORE(p32_all, C+i, v1);
	/*
        printf("\nread C\n");
        for(int j = 0; j < 16; j++) printf("%f ", (C+i)[j]);
        printf("\n");
	*/
    }
}

// regular increment with no equalivilance
void read_with_1xload(FLOAT* C, FLOAT* A, INDEX_TYPE* B, PERM_T* perm, SELECT_T* select, int nnz) {
    svbool_t p32_all = svptrue_b32();
    for(int i=0, j=0;i<nnz;i+=VECTOR_LEN, ++j) {
        VECREG v1 = VLOAD(p32_all, A+B[j]);
        VSTORE(p32_all, C+i, v1);
    }
}

// regular increment with equalivilance
void read_with_1xload_1xperm(FLOAT* C, FLOAT* A, INDEX_TYPE* B, PERM_T* perm, SELECT_T* select, int nnz) {
    svbool_t p32_all = svptrue_b32();
    for(int i=0, j=0;i<nnz;i+=VECTOR_LEN, ++j) {
        VECREG v1 = VLOAD(p32_all, A+B[j]);
        VECREG_I idx = VLOAD_I(p32_all, reinterpret_cast<const INDEX_TYPE*>(perm));

	/*
        uint32_t readperm[16];
        VSTORE_I(p32_all, readperm, idx);
        printf("\nperm\n");
        for(int j = 0; j < 16; j++) printf("%d ", perm[j]);
        printf("\nread perm\n");
        for(int j = 0; j < 16; j++) printf("%d ", readperm[j]);
        printf("\n");
	*/

        VECREG v2 = VPERM(v1, idx);
        VSTORE(p32_all, C+i, v2);

	/*
        printf("\nread C\n");
        for(int j = 0; j < 16; j++) printf("%f ", (C+i)[j]);
        printf("\n");
	*/
    }
}

void print_vec_4(VECREG v) {
    FLOAT values[4];
    svbool_t p32_all = svptrue_b32();
    VSTORE(p32_all, values, v);
    printf("\nvalues:\n");
    for(int j = 0; j < 4; j++) printf("%lf ", values[j]);
    printf("\n");
}

void read_with_2xload(FLOAT* C, FLOAT* A, INDEX_TYPE* B, PERM_T* perm, SELECT_T* select, int nnz) {
    svbool_t p32_all = svptrue_b32();
    for(int i=0, j=0;i<nnz;i+=VECTOR_LEN, j+=2) {
        VECREG v1 = VLOAD(p32_all, A+B[j]);
        VECREG v2 = VLOAD(p32_all, A+B[j+1]);
        // VECREG v2 = VLOAD(p32_all, A);
        VECREG_I idx1 = VLOAD_I(p32_all, reinterpret_cast<const INDEX_TYPE*>(perm));
        VECREG_I idx2 = VLOAD_I(p32_all, reinterpret_cast<const INDEX_TYPE*>(perm+8));
        VECREG v1_ = VPERM(v1, idx1);
        VECREG v2_ = VPERM(v2, idx2);

        // printf("v1_\n");
        // print_vec_4(v1_);
        // printf("v2\n");
        // print_vec_4(v2);
        // printf("v2_\n");
        // print_vec_4(v2_);
        // INDEX_TYPE values[4];
        // VSTORE_I(p32_all, values, idx2);
        // printf("\nidx2\n");
        // for(int j = 0; j < 4; j++) printf("%d ", values[j]);
        // printf("\n");

	/*
        FLOAT readv1_[16];
        VSTORE(p32_all, readv1_, v1_);
        printf("\nv1_\n");
        for(int j = 0; j < 16; j++) printf("%f ", readv1_[j]);
        VSTORE(p32_all, readv1_, v2_);
        printf("\nv2_\n");
        for(int j = 0; j < 16; j++) printf("%f ", readv1_[j]);
        printf("\n");
	*/


        // select
        //VECREG sel = VSET32(SELECT_0);
        VECREG_I v_select = VLOAD_I(p32_all, reinterpret_cast<const INDEX_TYPE*>(select));

	/*
        uint32_t readsel[16];
        VSTORE_I(p32_all, readsel, v_select);
        printf("\nv_select\n");
        for(int j = 0; j < 16; j++) printf("%d ", readsel[j]);
	*/

        svbool_t sel = VCMP(p32_all, v_select, 0);
        VECREG res = VBLEND(sel, v1_, v2_);
        // printf("res\n");
        // print_vec_4(res);
        VSTORE(p32_all, C+i, res);
    }
}

void read_with_3xload(FLOAT* C, FLOAT* A, INDEX_TYPE* B, PERM_T* perm, SELECT_T* select, int nnz) {
    svbool_t p32_all = svptrue_b32();
    for(int i=0, j=0;i<nnz;i+=VECTOR_LEN, j+=3) {
        VECREG v1 = VLOAD(p32_all, A+B[j]);
        VECREG v2 = VLOAD(p32_all, A+B[j+1]);
        VECREG v3 = VLOAD(p32_all, A+B[j+2]);
        VECREG_I idx1 = VLOAD_I(p32_all, reinterpret_cast<const INDEX_TYPE*>(perm));
        VECREG_I idx2 = VLOAD_I(p32_all, reinterpret_cast<const INDEX_TYPE*>(perm+8));
        VECREG_I idx3 = VLOAD_I(p32_all, reinterpret_cast<const INDEX_TYPE*>(perm+8*2));
        VECREG v1_ = VPERM(v1, idx1);
        VECREG v2_ = VPERM(v2, idx2);
        VECREG v3_ = VPERM(v3, idx3);

        // select
        VECREG_I v_select0 = VLOAD_I(p32_all, reinterpret_cast<const INDEX_TYPE*>(select));
        VECREG_I v_select1 = VLOAD_I(p32_all, reinterpret_cast<const INDEX_TYPE*>(select+8));

        svbool_t sel0 = VCMP(p32_all, v_select0, 0);
        svbool_t sel1 = VCMP(p32_all, v_select1, 0);
        VECREG blend0 = VBLEND(sel0, v1_, v2_);
        VECREG res = VBLEND(sel1, blend0, v3_);
        VSTORE(p32_all, C+i, res);
    }
}

void read_with_4xload(FLOAT* C, FLOAT* A, INDEX_TYPE* B, PERM_T* perm, SELECT_T* select, int nnz) {
    svbool_t p32_all = svptrue_b32();
    for(int i=0, j=0;i<nnz;i+=VECTOR_LEN, j+=4) {
        VECREG v1 = VLOAD(p32_all, A+B[j]);
        VECREG v2 = VLOAD(p32_all, A+B[j+1]);
        VECREG v3 = VLOAD(p32_all, A+B[j+2]);
        VECREG v4 = VLOAD(p32_all, A+B[j+3]);
        VECREG_I idx1 = VLOAD_I(p32_all, reinterpret_cast<const INDEX_TYPE*>(perm));
        VECREG_I idx2 = VLOAD_I(p32_all, reinterpret_cast<const INDEX_TYPE*>(perm+8));
        VECREG_I idx3 = VLOAD_I(p32_all, reinterpret_cast<const INDEX_TYPE*>(perm+8*2));
        VECREG_I idx4 = VLOAD_I(p32_all, reinterpret_cast<const INDEX_TYPE*>(perm+8*3));
        VECREG v1_ = VPERM(v1, idx1);
        VECREG v2_ = VPERM(v2, idx2);
        VECREG v3_ = VPERM(v3, idx3);
        VECREG v4_ = VPERM(v4, idx4);

        // select
        VECREG_I v_select0 = VLOAD_I(p32_all, reinterpret_cast<const INDEX_TYPE*>(select));
        VECREG_I v_select1 = VLOAD_I(p32_all, reinterpret_cast<const INDEX_TYPE*>(select+8));
        VECREG_I v_select2 = VLOAD_I(p32_all, reinterpret_cast<const INDEX_TYPE*>(select+8*2));

        svbool_t sel0 = VCMP(p32_all, v_select0, 0);
        svbool_t sel1 = VCMP(p32_all, v_select1, 0);
        svbool_t sel2 = VCMP(p32_all, v_select2, 0);
        VECREG blend0 = VBLEND(sel0, v1_, v2_);
        VECREG blend1 = VBLEND(sel1, blend0, v3_);
        VECREG res = VBLEND(sel2, blend1, v4_);
        VSTORE(p32_all, C+i, res);
    }
}

void print_vec_8(VECREG v) {
    FLOAT values[8];
    svbool_t p32_all = svptrue_b32();
    VSTORE(p32_all, values, v);
    printf("\nvalues:\n");
    for(int j = 0; j < 8; j++) printf("%f ", values[j]);
    printf("\n");
}

void read_with_8xload(FLOAT* C, FLOAT* A, INDEX_TYPE* B, PERM_T* perm, SELECT_T* select, int nnz) {
    svbool_t p32_all = svptrue_b32();
    for(int i=0, j=0;i<nnz;i+=VECTOR_LEN, j+=8) {
        VECREG v1 = VLOAD(p32_all, A+B[j]);
        VECREG v2 = VLOAD(p32_all, A+B[j+1]);
        VECREG v3 = VLOAD(p32_all, A+B[j+2]);
        VECREG v4 = VLOAD(p32_all, A+B[j+3]);
        VECREG v5 = VLOAD(p32_all, A+B[j+4]);
        VECREG v6 = VLOAD(p32_all, A+B[j+5]);
        VECREG v7 = VLOAD(p32_all, A+B[j+6]);
        VECREG v8 = VLOAD(p32_all, A+B[j+7]);

        VECREG_I idx1 = VLOAD_I(p32_all, reinterpret_cast<const INDEX_TYPE*>(perm));
        VECREG_I idx2 = VLOAD_I(p32_all, reinterpret_cast<const INDEX_TYPE*>(perm+8));
        VECREG_I idx3 = VLOAD_I(p32_all, reinterpret_cast<const INDEX_TYPE*>(perm+8*2));
        VECREG_I idx4 = VLOAD_I(p32_all, reinterpret_cast<const INDEX_TYPE*>(perm+8*3));
        VECREG_I idx5 = VLOAD_I(p32_all, reinterpret_cast<const INDEX_TYPE*>(perm+8*4));
        VECREG_I idx6 = VLOAD_I(p32_all, reinterpret_cast<const INDEX_TYPE*>(perm+8*5));
        VECREG_I idx7 = VLOAD_I(p32_all, reinterpret_cast<const INDEX_TYPE*>(perm+8*6));
        VECREG_I idx8 = VLOAD_I(p32_all, reinterpret_cast<const INDEX_TYPE*>(perm+8*7));
        VECREG v1_ = VPERM(v1, idx1);
        VECREG v2_ = VPERM(v2, idx2);
        VECREG v3_ = VPERM(v3, idx3);
        VECREG v4_ = VPERM(v4, idx4);
        VECREG v5_ = VPERM(v5, idx5);
        VECREG v6_ = VPERM(v6, idx6);
        VECREG v7_ = VPERM(v7, idx7);
        VECREG v8_ = VPERM(v8, idx8);

        // select
        VECREG_I v_select0 = VLOAD_I(p32_all, reinterpret_cast<const INDEX_TYPE*>(select));
        VECREG_I v_select1 = VLOAD_I(p32_all, reinterpret_cast<const INDEX_TYPE*>(select+8));
        VECREG_I v_select2 = VLOAD_I(p32_all, reinterpret_cast<const INDEX_TYPE*>(select+8*2));
        VECREG_I v_select3 = VLOAD_I(p32_all, reinterpret_cast<const INDEX_TYPE*>(select+8*3));
        VECREG_I v_select4 = VLOAD_I(p32_all, reinterpret_cast<const INDEX_TYPE*>(select+8*4));
        VECREG_I v_select5 = VLOAD_I(p32_all, reinterpret_cast<const INDEX_TYPE*>(select+8*5));
        VECREG_I v_select6 = VLOAD_I(p32_all, reinterpret_cast<const INDEX_TYPE*>(select+8*6));

        svbool_t sel0 = VCMP(p32_all, v_select0, 0);
        svbool_t sel1 = VCMP(p32_all, v_select1, 0);
        svbool_t sel2 = VCMP(p32_all, v_select2, 0);
        svbool_t sel3 = VCMP(p32_all, v_select3, 0);
        svbool_t sel4 = VCMP(p32_all, v_select4, 0);
        svbool_t sel5 = VCMP(p32_all, v_select5, 0);
        svbool_t sel6 = VCMP(p32_all, v_select6, 0);
        VECREG blend0 = VBLEND(sel0, v1_, v2_);
        VECREG blend1 = VBLEND(sel1, blend0, v3_);
        VECREG blend2 = VBLEND(sel2, blend1, v4_);
        VECREG blend3 = VBLEND(sel3, blend2, v5_);
        VECREG blend4 = VBLEND(sel4, blend3, v6_);
        VECREG blend5 = VBLEND(sel5, blend4, v7_);
        VECREG res = VBLEND(sel6, blend5, v8_);

        VSTORE(p32_all, C+i, res);
    }
}

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

int main()
{
    printf("vector size = %d!\n", VECTOR_LEN);
    int nnz = 8192;
    printf("Input nnz:\n");
    scanf("%d", &nnz);
    int max_extra_lines = 4; // for extra loads
    FLOAT* C = (FLOAT*)malloc(nnz*sizeof(FLOAT));
    FLOAT* C2= (FLOAT*)malloc(nnz*sizeof(FLOAT));
    FLOAT* A = (FLOAT*)malloc((nnz+VECTOR_LEN*max_extra_lines)*sizeof(FLOAT));
    INDEX_TYPE* B = (INDEX_TYPE*)malloc(nnz*sizeof(INDEX_TYPE));
    for(int i=0;i<nnz+VECTOR_LEN*max_extra_lines;++i) {
        if (i < nnz) {
            B[i] = i;
        }
        // printf("B[i] = %d, i = %d\n", B[i], i);
        A[i] = i+0.1;
    }
    printf("nnz = %d\n", nnz);
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

    init_para_perm(perm, GATHER_PERM_DAT_FN);
    init_para_select(select, SELECT_DAT_FN);

    // 1
    // use special permutation for 1xload
    for(int i=0;i<nnz;i+=VECTOR_LEN) {
        for(int j=0;j<VECTOR_LEN;++j) {
            B[i+j] = i+VECTOR_LEN-j-1;
        }
    }
    #if defined USE_256
        #ifdef USE_DOUBLE
            int64_t perm2[] = { 3,2,1,0 };
        #else
            int32_t perm2[] = { 7,6,5,4,3,2,1,0 };
        #endif
    #else
        #ifdef USE_DOUBLE
            int64_t perm2[] = { 7,6,5,4,3,2,1,0 };
        #else
            int32_t perm2[] = { 15,14,13,12,11,10,9,8,7,6,5,4,3,2,1,0 };
        #endif
    #endif
    EVAL(read_with_gather, C, A, B, perm2, select[0], nnz, "data/gather_1.dat");
    for(int i=0;i<nnz/VECTOR_LEN;++i) {
        B[i] = i*VECTOR_LEN;
    }
    EVAL(read_with_1xload_1xperm, C2, A, B, perm2, select[0], nnz, "data/load_1.dat");
    check(C, C2, nnz);
    
    
    // 2
    INIT_GATHER_INDEX(B, nnz, 2);
    EVAL(read_with_gather, C, A, B, perm, select[0], nnz, "data/gather_2.dat");
    INIT(B, nnz, 2);
    EVAL(read_with_2xload, C2, A, B, perm, select[0], nnz, "data/load_2.dat");
    check(C, C2, nnz);

    // 3
    INIT_GATHER_INDEX(B, nnz, 3);
    EVAL(read_with_gather, C, A, B, perm, select[0], nnz, "data/gather_3.dat");
    INIT(B, nnz, 3);
    EVAL(read_with_3xload, C2, A, B, perm, select[0], nnz, "data/load_3.dat");
    check(C, C2, nnz);

    // 4
    INIT_GATHER_INDEX(B, nnz, 4);
    EVAL(read_with_gather, C, A, B, perm, select[0], nnz, "data/gather_4.dat");
    INIT(B, nnz, 4);
    EVAL(read_with_4xload, C2, A, B, perm, select[0], nnz, "data/load_4.dat");
    check(C, C2, nnz);

#ifndef USE_DOUBLE
    // 8
    INIT_GATHER_INDEX(B, nnz, 8);
    EVAL(read_with_gather, C, A, B, perm, select[0], nnz, "data/gather_8.dat");
    INIT(B, nnz, 8);
    EVAL(read_with_8xload, C2, A, B, perm, select[0], nnz, "data/load_8.dat");
    check(C, C2, nnz);
#endif

    return 0;
}
