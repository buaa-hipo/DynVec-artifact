#ifndef CSR_SPMV_SIMD
#define CSR_SPMV_SIMD
#define DTYPE double
	void csr_spmv_simd( const DTYPE * data_ptr , const int * column_ptr , const double * x, double * y ,DetectionRecordPtr detection_record_ptr , const int detection_record_num);

#endif
