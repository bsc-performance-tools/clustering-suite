/*
 * dataorg.c
 *
 *  Created on: 07/04/2010
 *      Author: aislan
 */

#include "dataorg.h"

void records_center_count_task(int NUMBER_OF_RECORDS, int CENTERS, int *records_center_count, int *assigned_centers) {
	int i = 0;
	memset(records_center_count, 0, CENTERS * sizeof(int));

	for (i = 0; i < NUMBER_OF_RECORDS; i++) {
		records_center_count[assigned_centers[i]]++;
	}
}

void records_center_index_task(int CENTERS, int *records_center_count, int *records_center_mempos, int *records_center_mempos_tmp) {
	int i, j = 0;

	memset(records_center_mempos, 0, CENTERS * sizeof(int));

	for (i = 0; i < CENTERS; i++) {
		for (j = 0; j < i; j++) {
			records_center_mempos[i] += records_center_count[j];
		}
	}
	memcpy(records_center_mempos_tmp, records_center_mempos, CENTERS*sizeof(int));

}

void generate_memcpy_index_task(int NUMBER_OF_RECORDS, int TOTAL_NUMBER_OF_RECORDS, int CENTERS, int *assigned_centers, int *records_center_mempos_tmp, int *assigned_memcpy, int *records_center_hist) {
	int i = 0;

//	memset(records_center_hist, 0, CENTERS * sizeof(int));

	memcpy(records_center_hist, records_center_mempos_tmp, CENTERS * sizeof(int));

	for (i = 0; i < NUMBER_OF_RECORDS; i++) {
		int center = assigned_centers[i];
		assigned_memcpy[i] = records_center_hist[center];

//#pragma css mutex lock (&records_center_mempos_tmp[center])
//		records_center_mempos_local[center]++;
		records_center_hist[center]++;
//#pragma css mutex unlock (&records_center_mempos_tmp[center])
	}
	for (i = 0; i < CENTERS; i++)
		records_center_hist[i] = records_center_hist[i] - records_center_mempos_tmp[i];
}

void generate_memcpy_index_task2(int NUMBER_OF_RECORDS, int CENTERS, int *assigned_centers, int *records_center_mempos_tmp, int *assigned_memcpy) {
	int i = 0;

	for (i = 0; i < NUMBER_OF_RECORDS; i++) {
		assigned_memcpy[i] = records_center_mempos_tmp[assigned_centers[i]];
		records_center_mempos_tmp[assigned_centers[i]]++;
	}
}

void update_hist_task(int CENTERS, int *records_center_hist, int *records_center_hist_update) {
	int i;
	for (i = 0; i < CENTERS; i++) {
		records_center_hist_update[i] = records_center_hist[i] + records_center_hist_update[i];
	}
}



void memcpy_task3(int NUMBER_OF_RECORDS, int TOTAL_NUMBER_OF_RECORDS, int CENTERS, int DIMENSIONS, float *records, int *assigned_memcpy, int *assigned_centers, float *records_center, int *records_center_hist) {
	int i = 0;

	for (i = 0; i < NUMBER_OF_RECORDS; i++) {
		int pos = assigned_memcpy[i];
		int offset = records_center_hist[assigned_centers[i]];
		memcpy(&records_center[(pos+offset)*DIMENSIONS], &records[i*DIMENSIONS], DIMENSIONS * sizeof(float));

	}
}


void memcpy_task2(int NUMBER_OF_COPY_RECORDS, int NUMBER_OF_RECORDS, int DIMENSIONS, int CENTER, int CENTERS, int OFFSET, float *records, int *assigned_centers, int *records_center_mempos_tmp, float *records_center) {
	int i = 0;
	int offset_counter = 0;
	int copy_counter = 0;

	for (i = 0; i < NUMBER_OF_RECORDS; i++) {
		if(assigned_centers[i] == CENTER) {
			if(offset_counter < OFFSET)
				offset_counter++;
			else {
				memcpy(&records_center[copy_counter*DIMENSIONS], &records[i*DIMENSIONS], DIMENSIONS * sizeof(float));
				copy_counter++;
				if(copy_counter == NUMBER_OF_COPY_RECORDS)
					break;
			}
		}
	}
}

void memcpy_task(int NUMBER_OF_RECORDS, int TOTAL_RECORDS, int DIMENSIONS, int CENTERS, float *records, int *assigned_centers, int *records_center_mempos_tmp, float *records_center) {
	int i = 0;

	for (i = 0; i < NUMBER_OF_RECORDS; i++) {

		int assign = assigned_centers[i];
#pragma css mutex lock (&records_center_mempos_tmp[assigned_centers[i]])
		int pos = records_center_mempos_tmp[assign];
		records_center_mempos_tmp[assign]++;
#pragma css mutex unlock (&records_center_mempos_tmp[assigned_centers[i]])

		memcpy(&records_center[pos*DIMENSIONS], &records[i*DIMENSIONS], DIMENSIONS * sizeof(float));
	}
}

