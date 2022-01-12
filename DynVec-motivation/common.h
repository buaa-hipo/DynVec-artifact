#include <stdio.h>
#include <stdint.h>
#include <immintrin.h>
#include <float.h>

#define MIN(x,y) ((x)<(y)?(x):(y))
#define MAX(x,y) ((x)>(y)?(x):(y))

#define VECTOR_LEN (VECTOR_SIZE/(8*sizeof(FLOAT)))

#ifdef USE_DOUBLE
  #define FLOAT double
#else
  #define FLOAT float
#endif

#ifdef USE_AVX512
  #define VECREG_I __m512i
  #define VECTOR_SIZE 512
  #define VLOAD_I _mm512_loadu_si512
  // __m256i _mm256_shuffle_epi32 (__m256i a, const int imm8)
  #define VSHUFFLE _mm512_shuffle_epi32
  // __m256i _mm256_set_epi32 (int e7, int e6, int e5, int e4, int e3, int e2, int e1, int e0)
  #define VSET32 _mm512_set_epi32
  #define VBLEND_I VBLEND
  #ifdef USE_DOUBLE
    #define SELECT_T __mmask8
    #define VECREG_GI __m256i
    #define VLOAD_GI _mm256_loadu_si256
    #define VECREG __m512d
    // __m512d _mm512_i32gather_pd (__m256i vindex, void const* base_addr, int scale)
    #define VGATHER(A, ind, scale) _mm512_i32gather_pd(ind, A, scale)
    #define VSCATTER(A, ind, V, scale) _mm512_i32scatter_pd(A, ind, V, scale)
    // __m256 _mm256_load_ps (float const * mem_addr)
    #define VLOAD _mm512_loadu_pd
    #define VSTORE _mm512_storeu_pd
    #define VBLEND(a,b,sel) _mm512_mask_blend_pd(sel, a, b)
    // The double has no corresponding permute operation. Treat one double as two combined int32 and use the integer one
    #define VPERM(a, idx) reinterpret_cast<__m512d>(_mm512_permutexvar_epi64(reinterpret_cast<__m512i>(idx), reinterpret_cast<__m512i>(a)))
  #else
    #define SELECT_T __mmask16
    #define VECREG_GI __m512i
    #define VLOAD_GI VLOAD_I
    #define VECREG __m512
    // __m512 _mm512_i32gather_ps (__m256i vindex, void const* base_addr, int scale)
    #define VGATHER(A, ind, scale) _mm512_i32gather_ps(ind, A, scale)
    #define VSCATTER(A, ind, V, scale) _mm512_i32scatter_ps(A, ind, V, scale)
    // __m256 _mm256_load_ps (float const * mem_addr)
    #define VLOAD _mm512_loadu_ps
    #define VSTORE _mm512_storeu_ps
    #define VBLEND(a,b,sel) _mm512_mask_blend_ps(sel, a, b)
    // __m256 _mm256_permutevar8x32_ps (__m256 a, __m256i idx)
    #define VPERM(a, idx) reinterpret_cast<__m512>(_mm512_permutexvar_epi32(reinterpret_cast<__m512i>(idx), reinterpret_cast<__m512i>(a)))
  #endif

  // TODO: PERM + BLEND can be replaced by a single AVX512 instruction _mm512_permutex2var_ps
  /*    FOR j := 0 to 15
            i := j*32
            off := idx[i+3:i]*32
            dst[i+31:i] := idx[i+4] ? b[off+31:off] : a[off+31:off]
        ENDFOR
        dst[MAX:512] := 0
    */

#else
  #define VECREG_I __m256i
  #define SELECT_T VECREG_I
  #define VECTOR_SIZE 256
  #define VLOAD_I _mm256_loadu_si256
  // __m256i _mm256_shuffle_epi32 (__m256i a, const int imm8)
  #define VSHUFFLE _mm256_shuffle_epi32
  // __m256i _mm256_set_epi32 (int e7, int e6, int e5, int e4, int e3, int e2, int e1, int e0)
  #define VSET32 _mm256_set_epi32
  #define VBLEND_I _mm256_blendv_epi8
  #ifdef USE_DOUBLE
    #define VECREG_GI __m128i
    #define VLOAD_GI _mm_lddqu_si128
    #define VECREG __m256d
    // __m256 _mm256_i32gather_ps (float const* base_addr, __m256i vindex, const int scale)
    #define VGATHER _mm256_i32gather_pd
    // __m256 _mm256_load_ps (float const * mem_addr)
    #define VLOAD _mm256_loadu_pd
    #define VSTORE _mm256_storeu_pd
    #define VBLEND _mm256_blendv_pd
    // The double has no corresponding permute operation. Treat one double as two combined int32 and use the integer one
    #define VPERM(a, idx) _mm256_permutevar8x32_epi32(reinterpret_cast<__m256i>(a), idx)
  #else
    #define VECREG_GI __m256i
    #define VLOAD_GI VLOAD_I
    #define VECREG __m256
    // __m256 _mm256_i32gather_ps (float const* base_addr, __m256i vindex, const int scale)
    #define VGATHER _mm256_i32gather_ps
    // __m256 _mm256_load_ps (float const * mem_addr)
    #define VLOAD _mm256_loadu_ps
    
    #define VSTORE _mm256_storeu_ps
    #define VBLEND _mm256_blendv_ps
    // __m256 _mm256_permutevar8x32_ps (__m256 a, __m256i idx)
    #define VPERM _mm256_permutevar8x32_ps
  #endif
#endif

#ifndef ARRAY_SIZE
#define ARRAY_SIZE (1024*1024)
#endif
#define TEST_SIZE (8*ARRAY_SIZE)

#ifdef USE_AVX512
  #define INPUT_DIR "avx512-input/"
#else
  #define INPUT_DIR "avx2-input/"
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


#ifdef USE_AVX512
void init_para_perm(int perm[VECTOR_LEN][16], const char* fn) {
    printf("Parameter PERM file: %s\n", fn);
    FILE* f=fopen(fn, "r");
    for(int i=0;i<VECTOR_LEN;++i) {
#ifdef USE_DOUBLE
        for(int j=0;j<16;j+=2) {
#else
        for(int j=0;j<16;++j) {
#endif
            fscanf(f, "%d", &perm[i][j]);
        }
    }
    fclose(f);
}

void init_para_select(SELECT_T select[15][15], const char* fn) {
    printf("Parameter SELECT file: %s\n", fn);
    FILE* f = fopen(fn, "r");
    for(int n=0;n<VECTOR_LEN-1;++n) {
        bool flag = false;
        for(int i=0;i<15;++i) {
            select[n][i] = 0;
            for(int j=0;j<VECTOR_LEN;++j) {
                int x;
                fscanf(f,"%d", &x);
                if(x<0) { flag=true; break; }
                if(x>0) { select[n][i] = select[n][i] | (1<<j); }
            }
            if (flag) break;
        }
    }
    fclose(f);
}
#else
void init_para_perm(int perm[VECTOR_LEN][8], const char* fn) {
    printf("Parameter PERM file: %s\n", fn);
    FILE* f=fopen(fn, "r");
    for(int i=0;i<VECTOR_LEN;++i) {
        for(int j=0;j<8;++j) {
            fscanf(f, "%d", &perm[i][j]);
        }
    }
    fclose(f);
}
void init_para_select(SELECT_T select[7][7], const char* fn) {
    printf("Parameter SELECT file: %s\n", fn);
    FILE* f = fopen(fn, "r");
#ifdef USE_DOUBLE
    for(int n=0;n<3;++n) {
        bool flag = false;
        for(int i=0;i<7;++i) {
            uint64_t* __sel = reinterpret_cast<uint64_t*>(&select[n][i]);
            for(int j=0;j<VECTOR_LEN;++j) {
                int x;
                fscanf(f,"%d", &x);
                if(x<0) { flag=true; break; }
                __sel[j] = (x==0) ? 0 : 0xffffffffffffffff;
            }
            if (flag) break;
        }
    }
#else
    for(int n=0;n<7;++n) {
        bool flag = false;
        for(int i=0;i<7;++i) {
            uint32_t* __sel = reinterpret_cast<uint32_t*>(&select[n][i]);
            for(int j=0;j<VECTOR_LEN;++j) {
                int x;
                fscanf(f,"%d", &x);
                if(x<0) { flag=true; break; }
                __sel[j] = (x==0) ? 0 : 0xffffffff;
            }
            if (flag) break;
        }
    }
#endif
    fclose(f);
}
#endif

void check(FLOAT* CREF, FLOAT*COPT, int nnz) {
    int num = 0;
    for(int i=0;i<nnz;++i) {
        if (CREF[i]!=COPT[i]) {
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

void print_info() {
#ifdef USE_AVX512
    printf("INFO: USE_AVX512 is set\n");
#else
    printf("INFO: using AVX2 mode\n");
#endif
#ifdef USE_DOUBLE
    printf("INFO: USE_DOUBLE is set\n");
#else
    printf("INFO: using single floating-point\n");
#endif
}
