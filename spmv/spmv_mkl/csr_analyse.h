#ifndef CSR_ANALYSE
#define CSR_ANALYSE
typedef unsigned char MASK;
#define DetectionRecord_SIZE 8
#define DetectionRecordSize DetectionRecord_SIZE
#define CACHELINE 64
typedef struct DetectionRecord{
	MASK scatter_mask;
	long long scatter_index[DetectionRecord_SIZE];
	long long shift_index[DetectionRecord_SIZE];
}DetectionRecord , *DetectionRecordPtr;


	DetectionRecordPtr analyse( const int *row_ptr , const int data_num , const int row_num ) ;
#endif
