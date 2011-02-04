/*
 * dataorg.h
 *
 *  Created on: 07/04/2010
 *      Author: aislan
 */

#ifndef DATAORG_H_
#define DATAORG_H_

#include <string.h>
#include <malloc.h>

#pragma css task input(NUMBER_OF_RECORDS, CENTERS) \
				 input(assigned_centers[NUMBER_OF_RECORDS]) \
				 inout(records_center_count[CENTERS])

void records_center_count_task(int NUMBER_OF_RECORDS, int CENTERS, int *records_center_count, int *assigned_centers);

#pragma css task input(CENTERS) \
				 input(records_center_count[CENTERS]) \
				 inout(records_center_mempos[CENTERS], records_center_mempos_tmp[CENTERS])

void records_center_index_task(int CENTERS, int *records_center_count, int *records_center_mempos, int *records_center_mempos_tmp);

#pragma css task input(NUMBER_OF_RECORDS, TOTAL_RECORDS, DIMENSIONS, CENTERS) \
				 input(records[NUMBER_OF_RECORDS*DIMENSIONS], assigned_centers[NUMBER_OF_RECORDS]) \
				 inout(records_center_mempos_tmp[CENTERS], records_center[TOTAL_RECORDS*DIMENSIONS]) \
				 reduction (records_center_mempos_tmp[CENTERS], records_center[TOTAL_RECORDS*DIMENSIONS])

void memcpy_task(int NUMBER_OF_RECORDS, int TOTAL_RECORDS, int DIMENSIONS, int CENTERS, float *records, int *assigned_centers, int *records_center_mempos_tmp, float *records_center);

#pragma css task input(NUMBER_OF_COPY_RECORDS, NUMBER_OF_RECORDS, DIMENSIONS, CENTER, CENTERS, OFFSET) \
				 input(records[NUMBER_OF_RECORDS*DIMENSIONS], assigned_centers[NUMBER_OF_RECORDS]) \
				 input(records_center_mempos_tmp[CENTERS]) \
				 output(records_center[NUMBER_OF_COPY_RECORDS*DIMENSIONS])

void memcpy_task2(int NUMBER_OF_COPY_RECORDS, int NUMBER_OF_RECORDS, int DIMENSIONS, int CENTER, int CENTERS, int OFFSET, float *records, int *assigned_centers, int *records_center_mempos_tmp, float *records_center);





#pragma css task input(NUMBER_OF_RECORDS, TOTAL_NUMBER_OF_RECORDS, CENTERS) \
				 input(assigned_centers[NUMBER_OF_RECORDS]) \
                 input(records_center_mempos_tmp[CENTERS]) \
				 output(assigned_memcpy[NUMBER_OF_RECORDS], records_center_hist[CENTERS])
//				 reduction(records_center_mempos_tmp[CENTERS])

void generate_memcpy_index_task(int NUMBER_OF_RECORDS, int TOTAL_NUMBER_OF_RECORDS, int CENTERS, int *assigned_centers, int *records_center_mempos_tmp, int *assigned_memcpy, int *records_center_hist);

#pragma css task input(NUMBER_OF_RECORDS, CENTERS) \
				 input(assigned_centers[NUMBER_OF_RECORDS]) \
                 inout(records_center_mempos_tmp[CENTERS]) \
				 output(assigned_memcpy[NUMBER_OF_RECORDS])

void generate_memcpy_index_task2(int NUMBER_OF_RECORDS, int CENTERS, int *assigned_centers, int *records_center_mempos_tmp, int *assigned_memcpy);

#pragma css task input(CENTERS) \
				 input(records_center_hist[CENTERS]) \
                 inout(records_center_hist_update[CENTERS])

void update_hist_task(int CENTERS, int *records_center_hist, int *records_center_hist_update);


#pragma css task input(NUMBER_OF_RECORDS, TOTAL_NUMBER_OF_RECORDS, CENTERS, DIMENSIONS) \
				 input(records[NUMBER_OF_RECORDS*DIMENSIONS], assigned_centers[NUMBER_OF_RECORDS]) \
				 input(assigned_memcpy[NUMBER_OF_RECORDS], records_center_hist[CENTERS]) \
				 inout(records_center[TOTAL_NUMBER_OF_RECORDS*DIMENSIONS]) \
				 reduction(records_center[TOTAL_NUMBER_OF_RECORDS*DIMENSIONS])

void memcpy_task3(int NUMBER_OF_RECORDS, int TOTAL_NUMBER_OF_RECORDS, int CENTERS, int DIMENSIONS, float *records, int *assigned_memcpy, int *assigned_centers, float *records_center, int *records_center_hist);

#endif /* DATAORG_H_ */
