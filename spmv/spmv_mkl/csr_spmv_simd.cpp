#include "immintrin.h"
#include "csr_matrix.h"
#include "csr_analyse.h"
#include "csr_spmv_simd.h"
#include <stdio.h>
void csc_spmv(const csrSparseMatrixPtr matrix , const double * x , double * y ) {
    const int column_num = matrix->column_num;
    const int row_num = matrix->row_num;
    const int data_num = matrix->data_num;
    const int * row_ptr = matrix->row_ptr;
    const int * column_ptr = matrix->column_ptr;
    const double * data_ptr = matrix->data_ptr;
    for( int i = 0 ; i < matrix->row_num ; i++ ) {
        const int data_begin = row_ptr[i] ;
        const int data_end = row_ptr[i+1] ;
  //      double res = 0 ;
        y[i] = 0 ;
//        #pragma omp simd
        for( int j = data_begin ; j < data_end ; j++ ) {
            y[i] += data_ptr[ j ] *  x[ column_ptr[j] ];

        }
//	y[i] = res;
    }
}
inline __m512d hscan_avx512(__m512d scan512d, __m512d zero512d)
{
    register __m512d t0, t1;

    t0 = _mm512_permutex_pd(scan512d, 0xB1); //_mm512_swizzle_pd(scan512d, _MM_SWIZ_REG_CDAB);
    t1 = _mm512_permutex_pd(t0, 0x4E); //_mm512_swizzle_pd(t0, _MM_SWIZ_REG_BADC);
    t0 = _mm512_mask_blend_pd(0xAA, t1, t0);

    t1 = _mm512_mask_blend_pd(0x0F, zero512d, t0);
    t1 = _mm512_castsi512_pd(_mm512_permute4f128_epi32(_mm512_castpd_si512(t1), _MM_PERM_BADC));

    scan512d = _mm512_add_pd(scan512d, _mm512_mask_blend_pd(0x11, t0, t1));

    t0 = _mm512_permutex_pd(scan512d, 0x4E); //_mm512_swizzle_pd(scan512d, _MM_SWIZ_REG_BADC);

    t1 = _mm512_mask_blend_pd(0x0F, zero512d, t0);
    t1 = _mm512_castsi512_pd(_mm512_permute4f128_epi32(_mm512_castpd_si512(t1), _MM_PERM_BADC));

    scan512d = _mm512_add_pd(scan512d, _mm512_mask_blend_pd(0x33, t0, t1));

    t1 = _mm512_mask_blend_pd(0x0F, zero512d, scan512d);
    t1 = _mm512_castsi512_pd(_mm512_permute4f128_epi32(_mm512_castpd_si512(t1), _MM_PERM_BADC));
    scan512d = _mm512_add_pd(scan512d, t1);

    return scan512d;
}
#define PRINT_M512D(data) do { \
	double debug_data[8];\
	_mm512_store_pd(debug_data,data);printf(#data"\n");\
	for(int debug_i = 0 ; debug_i < 8 ; debug_i++)\
	printf( "%f ",debug_data[debug_i] );\
	printf("\n");} while(0) 

#define PRINT_M512I(data) do { \
	int debug_data[16];\
	_mm512_store_epi32(debug_data,data);printf(#data"\n");\
	for(int debug_i = 0 ; debug_i < 16 ; debug_i++)\
	printf( "%d ",debug_data[debug_i] );\
	printf("\n");} while(0) 

#define PRINT_M512LI(data) do { \
	long long debug_data[8];\
	_mm512_store_epi64(debug_data,data);printf(#data"\n");\
	for(int debug_i = 0 ; debug_i < 8 ; debug_i++)\
	printf( "%ld ",debug_data[debug_i] );\
	printf("\n");} while(0) 
#define PRINT8INT(data) do { \
	for(int debug_i = 0 ; debug_i < 8 ; debug_i++)\
		printf("%d",data[debug_i]);\
	}while(0)
#define PRINT8LONG(data) do { printf(#data"\n"); \
	for(int debug_i = 0 ; debug_i < 8 ; debug_i++)\
		printf("%ld ",data[debug_i]);\
	}while(0)


void csr_spmv_simd( const DTYPE * data_ptr , const int * column_ptr , const double * x, double * y ,DetectionRecordPtr detection_record_ptr , const int detection_record_num) {

//	const DTYPE * data_ptr = matrix->data_ptr;
//	const int * column_ptr = matrix->column_ptr;
    const __m512d __m512d_zero = _mm512_setzero_pd();
	__m512i __m512i_column;

	for (int detection_record_index = 0 ,detection_record_index_x = 0 ; detection_record_index < detection_record_num ; detection_record_index++ , detection_record_index_x += DetectionRecordSize ) {
			__m512i_column = (detection_record_index % 2) ?
			_mm512_permute4f128_epi32( __m512i_column, _MM_PERM_BADC) :
			_mm512_load_epi32(&column_ptr[detection_record_index_x]);
			PRINT8LONG(detection_record_ptr[detection_record_index].scatter_index);
			PRINT8LONG(detection_record_ptr[detection_record_index].shift_index);
			if(detection_record_ptr[detection_record_index].scatter_mask == 0x1) { //only reduce
				 const int y_index = detection_record_ptr[detection_record_index].scatter_index[0];
				 __m512d __m512d_x = _mm512_load_pd(&x[detection_record_index_x]);
//				 __m256i __m256i_column = _mm256_load_epi32(&column_ptr[detection_record_index_x]);
				        //x512d = _mm512_i32logather_pd(column_index512i, d_x, 8); 
				 __m512d __m512d_data = _mm512_i32logather_pd( __m512i_column, data_ptr, 8);

				 __m512d __m512d_data_x = _mm512_mul_pd(__m512d_data,__m512d_x);
				 y[y_index] += _mm512_reduce_add_pd( __m512d_data_x );
			} else {
 				 __m512d __m512d_x = _mm512_load_pd(&x[detection_record_index_x]);
				 __m512i __m512i_row = _mm512_load_epi64( &detection_record_ptr[detection_record_index].scatter_index);
				 __m512d __m512d_data = _mm512_i32logather_pd( __m512i_column,data_ptr,8);
				 __m512d __m512d_data_x =  _mm512_mul_pd(__m512d_x,__m512d_data);
//				 PRINT_M512D(__m512d_data_x);
				 __m512d_data_x = hscan_avx512(__m512d_data_x,__m512d_zero);
				 PRINT_M512D(__m512d_data_x);
				 __m512i __m512i_data_x_permute = _mm512_load_epi64(detection_record_ptr[detection_record_index].shift_index);
//				 __m512i_data_x_permute = _mm512_set_epi64(0,0,3,0,2,0,0,0 );
				 PRINT_M512LI( __m512i_data_x_permute );
				 PRINT_M512LI(__m512i_data_x_permute);
				 __m512d __m512d_data_x_permute = _mm512_mask_blend_pd(0x1, _mm512_permutexvar_pd(__m512i_data_x_permute ,__m512d_data_x) , __m512d_zero ); //
//				 
				 PRINT_M512D(__m512d_data_x_permute);

				 __m512d_data_x = _mm512_sub_pd( __m512d_data_x , __m512d_data_x_permute );

//				 PRINT8INT(detection_record_ptr[detection_record_index].scatter_mask)
				 PRINT_M512D(__m512d_data_x);
				 PRINT_M512LI(__m512i_row);
				 printf("%x\n",detection_record_ptr[detection_record_index].scatter_mask);
				 _mm512_mask_i64scatter_pd ( y , detection_record_ptr[detection_record_index].scatter_mask, __m512i_row , __m512d_data_x, 8);

			}

	}

}

