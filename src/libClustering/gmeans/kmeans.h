/*
 * kmeans.h
 *
 *  Created on: 13-ago-2009
 *      Author: aislan
 */

#ifndef KMEANS_H_
#define KMEANS_H_

//#define smpSs	// use SMPSs compiler
//#define CELL

//#define AD 	//AD improvement
//#define PROJAB 	//vector projector improvement
//#define PDATAORG
//#define PSORT

#ifdef CELL
#include <spu_intrinsics.h>
#endif

#include <sys/time.h>
#include <malloc.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#include "gmeans.h"
#include "kmeans_math.h"
#include "andersondarling.h"
#include "tools.h"


int big_kmeans (int DIMENSIONS, int CENTERS, int NUMBER_OF_RECORDS, float *records, float *centers, int *assigned_centers, int CSS_NUM_CPUS, int CSS_NUM_SPUS, int block_records, int *cluster_records_counter);

int twocenters_kmeans (int DIMENSIONS, int CENTERS, int NUMBER_OF_RECORDS, float *records, float *centers, int *assigned_centers, int CSS_NUM_SPUS, int block_records);

/*#pragma css task input(DIMENSIONS, NUMBER_OF_RECORDS, AD_CV, CSS_NUM_SPUS, block_records) \
                 input(records[NUMBER_OF_RECORDS * DIMENSIONS]) \
                 inout(orig_center[DIMENSIONS], new_center[DIMENSIONS]) \
                 output(split)
#endif*/

void kmeans_ad (int DIMENSIONS, int NUMBER_OF_RECORDS, float *records, float *orig_center, float *new_center, int *split, float AD_CV, int CSS_NUM_SPUS, int block_records);

#pragma css task input(DIMENSIONS, CENTERS, number_of_records) \
                 input(records[number_of_records * DIMENSIONS], centers[CENTERS * DIMENSIONS]) \
                 inout(assigned_center[number_of_records]) \
                 inout(newcenters[CENTERS * DIMENSIONS], newcenters_histogram[CENTERS + 1]) \
				 reduction (newcenters[CENTERS * DIMENSIONS], newcenters_histogram[CENTERS + 1])
// target device (cell)

void kmeans_calculate(int DIMENSIONS, int CENTERS, int number_of_records,
		float *records, float *centers, int *assigned_center,
		float *newcenters, int *newcenters_histogram);

#pragma css task input(DIMENSIONS, CENTERS, number_of_records) \
                 input(records[number_of_records * DIMENSIONS], centers[CENTERS * DIMENSIONS]) \
                 inout(assigned_center[number_of_records]) \
                 inout(newcenters[CENTERS * DIMENSIONS], newcenters_histogram[CENTERS + 1]) \
                 reduction (newcenters[CENTERS * DIMENSIONS], newcenters_histogram[CENTERS + 1])
// target device (cell)

void kmeans_calculate2(int DIMENSIONS, int CENTERS, int number_of_records,
		float *records, float *centers, int *assigned_center,
		float *newcenters, int *newcenters_histogram);

#pragma css task input (DIMENSIONS, CENTERS) \
				 input (newcenters[CENTERS*DIMENSIONS], newcenters_histograms[CENTERS+1]) \
				 output(centers[CENTERS*DIMENSIONS])

void recalculate_centers_task(int DIMENSIONS, int CENTERS,
		float *centers, float *newcenters, int *newcenters_histograms);

int compare_to_centers_cpu(float *record, int dimension, int number_of_centers,float *centers);
int compare_to_2centers_cpu(float *record, int dimension, float *centers);

/*#pragma css task input(DIMENSIONS, CENTERS, NUMBER_OF_RECORDS) \
                 input(records[NUMBER_OF_RECORDS * DIMENSIONS], centers[CENTERS * DIMENSIONS]) \
                 input(CSS_NUM_SPUS, block_records) \
                 inout(assigned_centers[NUMBER_OF_RECORDS]) \
                 inout(newcenters[CSS_NUM_SPUS], newcenters_histograms[CSS_NUM_SPUS])
#endif*/
void kmeans_cpu_task (int DIMENSIONS, int CENTERS, int NUMBER_OF_RECORDS, float *records, float *centers, int *assigned_centers, float *newcenters, int *newcenters_histograms, int CSS_NUM_SPUS, int block_records);

#pragma css task input(DIMENSIONS, NUMBER_OF_RECORDS, powvec) \
				 input(records[NUMBER_OF_RECORDS * DIMENSIONS], vec[DIMENSIONS]) \
				 inout(p_records[NUMBER_OF_RECORDS], sum, sum2) \
				 reduction (sum, sum2)

void projab_cpu_task (int DIMENSIONS, int NUMBER_OF_RECORDS, float *records, float *vec, float powvec, float *p_records, float *sum, float *sum2);

#endif /* KMEANS_H_ */
