#include "csr_analyse.h"
#include <stdio.h>
DetectionRecordPtr analyse( const int *row_ptr , const int data_num , const int row_num ) {
	const int detection_record_num = data_num / DetectionRecord_SIZE;
	DetectionRecordPtr detection_record_ptr = (DetectionRecordPtr)_mm_malloc( detection_record_num * sizeof(DetectionRecord) ,CACHELINE);
	int cur_row = 0 ;
	int remainder_data_num_cur_row = row_ptr[1] - row_ptr[0];
	for ( int detection_record_index = 0  ; detection_record_index < detection_record_num ; detection_record_index++ ) {

		detection_record_ptr[detection_record_index].scatter_mask = 0x1;
		detection_record_ptr[detection_record_index].scatter_index[0] = cur_row ;
//		detection_record_ptr[detection_record_index].sca
		if(remainder_data_num_cur_row > DetectionRecord_SIZE) {

			remainder_data_num_cur_row -= DetectionRecord_SIZE;

		} else {
//	int row_ptr[] = {0,3,4,5,12};
		        for(;row_ptr[cur_row]==row_ptr[cur_row+1];cur_row++);
			printf("%d\n",cur_row);
			int detection_record_fill_num = remainder_data_num_cur_row;
			int old_remainder_data_num_cur_row = 0 ;

			for(;detection_record_fill_num < DetectionRecord_SIZE;) {

				detection_record_ptr[detection_record_index].scatter_mask |= (1<<detection_record_fill_num);
				cur_row++;
				printf("cur row %d %d\n",cur_row,detection_record_fill_num);
				detection_record_ptr[detection_record_index].scatter_index[detection_record_fill_num] = cur_row;
				detection_record_ptr[detection_record_index].shift_index[detection_record_fill_num] = old_remainder_data_num_cur_row;

				old_remainder_data_num_cur_row = detection_record_fill_num;
				for(;row_ptr[cur_row]==row_ptr[cur_row+1];cur_row++);
				remainder_data_num_cur_row = row_ptr[cur_row+1] - row_ptr[cur_row];
				detection_record_fill_num += remainder_data_num_cur_row;

			}
//			detection_record_ptr[detection_record_index].scatter_index[DetectionRecord_SIZE-1] = cur_row;
//			detection_record_ptr[detection_record_index].scatter_mask |= (1<<DetectionRecord_SIZE);
			remainder_data_num_cur_row = detection_record_fill_num - DetectionRecord_SIZE;
//			detection_record_ptr[detection_record_index].shift_index[DetectionRecord_SIZE-1] = old_remainder_data_num_cur_row;

		}
	}
	return detection_record_ptr;
}
