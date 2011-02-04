/*
 * main.c
 *
 *  Created on: 13-ago-2009
 *      Author: aislan
 */

#include "gmeans.h"

/******************************************************************************
 *                                                                            *
 * Main function.                                                             *
 *                                                                            *
 ******************************************************************************/
int gmeans_c(int argc, char **argv) {
	/* Algorithm parameters */
	int NUMBER_OF_RECORDS, DIMENSIONS, CENTERS, NEW_CENTERS;
	int CSS_NUM_SPUS;
	int CSS_NUM_CPUS;
	/* Auxiliary variables */
	int i, j;//, k;
//	int MAX_RECORDS_CLUSTER;
	/* Other computations */
	int *records_center_count;
	int *records_center_mempos;
	int *records_center_mempos_tmp;
	int *records_center_hist;
//	int *cluster_records_counter;
	float *records_center;
	/* Data arrays */
	float *records;
	float *centers;
	float *new_centers;
	int *assigned_centers;//, *assigned_new_centers;
	int *assigned_memcpy;

	/*gmeans vars */
	double adcv;
	int split = 0;
	int nosplit = 0;
	int *splitlist;


	printf("***********************************************\n");
	printf("***********************************************\n");
	printf("******                                   ******\n");
	printf("******          SMPSs G-Means V4.0       ******\n");
	printf("******  BARCELONA SUPERCOMPUTING CENTER  ******\n");
	printf("******                                   ******\n");
	printf("******     Created by:                   ******\n");
	printf("******         Aislan Gomide Foina       ******\n");
	printf("******                                   ******\n");
	printf("***********************************************\n");
	printf("***********************************************\n\n");

	if (argc < 4) {
		printf("%s <data_points> <dim> <inicial_num_centers> (csv dataset path) (A&D cv)\n", argv[0]);
		printf("eg: %s 200000 60 10\n", argv[0]);
		return 1;
	}

/**
 *
 *  Initialization of the variables
 *
 */


	char *env_num_spus, *env_num_cpus;

#ifdef CELL

	env_num_spus = getenv("CSS_NUM_SPUS");
	if (env_num_spus != NULL)
		CSS_NUM_SPUS = atoi(env_num_spus);
	else
		CSS_NUM_SPUS = 8;

#else

	CSS_NUM_SPUS = 1;

#endif

#ifdef StarSs

	env_num_cpus = getenv("CSS_NUM_CPUS");
	if (env_num_cpus != NULL)
		CSS_NUM_CPUS = atoi(env_num_cpus);
	else
		CSS_NUM_CPUS = 1;

#endif

#ifdef smpSs

	CSS_NUM_CPUS = 1;

	env_num_cpus = getenv("CSS_NUM_CPUS");
	if (env_num_cpus != NULL)
		CSS_NUM_SPUS = atoi(env_num_cpus);
	else
		CSS_NUM_SPUS = 1;

#else

	CSS_NUM_CPUS = 1;

#endif

	NUMBER_OF_RECORDS = atoi(argv[1]);
	DIMENSIONS = atoi(argv[2]);
	CENTERS = atoi(argv[3]);
	NEW_CENTERS = 0;

	/* Check number of centers (to avoid uchar overflow) */
	if (CENTERS > 256) {
		printf("Current program only supports 256 centers\n.");
		return 2;
	}

#ifndef smpSs

	int block_size = DIMENSIONS * sizeof(float);
	int block_records = 1;
	while (block_size < BLOCK_SIZE) {
		block_size *= 2;
		block_records *= 2;
	}

	int localstore_usage = block_size + block_records * sizeof(int) + 2 * (CENTERS
			* DIMENSIONS * sizeof(float)) + (CENTERS + 1) * sizeof(int);

	printf("Record array SPU chunk size is %d\n", block_size);
	printf("Number of records per SPU chunk is %d\n", block_records);
	printf("Predicted SPU localstore usage is %d bytes\n\n", localstore_usage);


#else
	//	int block_records = (NUMBER_OF_RECORDS+CSS_NUM_SPUS)/CSS_NUM_SPUS;
	int block_records = BLOCK_SIZE; //1900;//BLOCK_SIZE;
	printf("Number of CPUs = %d\n\n", CSS_NUM_SPUS);
#endif


	maintime_int(0);

	// Calculate the critical value for adnserson&darling test
	get_ad_cv(1 - ALPHA, &adcv);

	// alocate alligned memory for the variables
	records = memalign(128, ((NUMBER_OF_RECORDS * DIMENSIONS) + 4)* sizeof(float));
	centers = memalign(128, MAX_CENTERS * DIMENSIONS * sizeof(float));
	assigned_centers = memalign(128, NUMBER_OF_RECORDS * sizeof(int));
	assigned_memcpy = memalign(128, NUMBER_OF_RECORDS * sizeof(int));

	memset(records, 0, ((NUMBER_OF_RECORDS * DIMENSIONS) + 4)* sizeof(float));
	memset(centers, 0, MAX_CENTERS * DIMENSIONS * sizeof(float));
	memset(assigned_centers, 0, NUMBER_OF_RECORDS * sizeof(int));

	// Alocated memory for the temporary variables for kmeans_ad
	records_center = memalign(128, ((NUMBER_OF_RECORDS * DIMENSIONS) + 4) * sizeof(float));
	records_center_count = memalign(128, MAX_CENTERS * sizeof(int));
	records_center_mempos = memalign(128, MAX_CENTERS * sizeof(int));
	records_center_mempos_tmp = memalign(128, MAX_CENTERS * sizeof(int));

	int num_hist = (NUMBER_OF_RECORDS / block_records) + 2;
	records_center_hist = memalign(128, num_hist * MAX_CENTERS * sizeof(int));

//	cluster_records_counter = memalign(128, MAX_CENTERS * sizeof(int));
	new_centers = memalign(128, MAX_CENTERS * DIMENSIONS * sizeof(float));

	splitlist = malloc(MAX_CENTERS * sizeof(int));

	/* Make record data */
	if (argc < 5) {

		for (i = 0; i < NUMBER_OF_RECORDS; i++)
			for (j = 0; j < DIMENSIONS; j++)
				records[i * DIMENSIONS + j] = rand() / 100000.0f + j;
	}
	// Or load from a file if a path is passed as parameter
	else if (argc >= 5) {
		getDataset(argv[4], records, NUMBER_OF_RECORDS, DIMENSIONS);
		printf("Data loaded from %s\n", argv[4]);

		// In case a Critical Value for A&D is passed as parameter
		if (argc == 6) {
			adcv = atoi(argv[5]);
		}

	}

	/* Initialize the centers as the first records of the dataset */
	for (i = 0; i < CENTERS; i++)
		memcpy(&centers[i * DIMENSIONS], &records[i * DIMENSIONS], DIMENSIONS * sizeof(float));

#pragma css start

	// Measure the start time
	maintime_int(1);

	while (CENTERS < 256) {

#ifdef MAIN_DEBUG
		printf("\n\n######### LOOP... TOTAL CENTERS = %d  NEW_CENTERS = %d #########\n\n", CENTERS, NEW_CENTERS);
#endif

		// Reset counters
		NEW_CENTERS = 0;
		split = 0;
		nosplit = 0;

		clear_int(records_center_count, MAX_CENTERS);


/**
 *
 * 		1. Big-Kmeans... Whole dataset
 *
 */

		big_kmeans(DIMENSIONS, CENTERS, NUMBER_OF_RECORDS, records, centers, assigned_centers, CSS_NUM_CPUS, CSS_NUM_SPUS, block_records, records_center_count);

//#pragma css barrier


/**
 *
 * 2. Data organization phase
 *
 */


// Order records in order according to the assigned center

#ifdef PDATAORG

#pragma css wait on (records_center_count)

		records_center_index_task(CENTERS, records_center_count, records_center_mempos, records_center_mempos_tmp);

		int chunk_of_records = 0;

		int k = 1;


		for (j = 0; j < NUMBER_OF_RECORDS; j += block_records) {
			chunk_of_records = (j + block_records > NUMBER_OF_RECORDS ? NUMBER_OF_RECORDS - j : block_records);
			generate_memcpy_index_task(chunk_of_records, NUMBER_OF_RECORDS, CENTERS, &assigned_centers[j], records_center_mempos_tmp, &assigned_memcpy[j], &records_center_hist[k*CENTERS]);
			k++;
		}

		for (j = 2; j < k; j++) {
			update_hist_task(CENTERS, &records_center_hist[(j-1)*CENTERS],&records_center_hist[j*CENTERS]);
		}

		k = 0;
		clear_int(records_center_hist, CENTERS);

		for (j = 0; j < NUMBER_OF_RECORDS; j += block_records) {
			chunk_of_records = (j + block_records > NUMBER_OF_RECORDS ? NUMBER_OF_RECORDS - j : block_records);
			memcpy_task3(chunk_of_records, NUMBER_OF_RECORDS, CENTERS, DIMENSIONS, &records[j*DIMENSIONS], &assigned_memcpy[j], &assigned_centers[j], records_center, &records_center_hist[k*CENTERS]);
			k++;
		}


#else

		memset(records_center_count, 0, CENTERS * sizeof(int));
		memset(records_center_mempos, 0, CENTERS * sizeof(int));
		memset(records_center_mempos_tmp, 0, CENTERS * sizeof(int));
//		memset(records_center, 0, NUMBER_OF_RECORDS * DIMENSIONS * sizeof(float));


		for (i = 0; i < NUMBER_OF_RECORDS; i++) {
			records_center_count[assigned_centers[i]]++;
		}
		for (i = 0; i < CENTERS; i++) {
			for (j = 0; j < i; j++) {
				records_center_mempos[i] += records_center_count[j];
				records_center_mempos_tmp[i] += records_center_count[j];
			}
		}
		for (i = 0; i < NUMBER_OF_RECORDS; i++) {
			memcpy(&records_center[records_center_mempos_tmp[assigned_centers[i]]*DIMENSIONS], &records[i*DIMENSIONS], DIMENSIONS * sizeof(float));
			records_center_mempos_tmp[assigned_centers[i]]++;
		}
#endif
		for (i = 0;  i < CENTERS;  i++) {

#ifdef MAIN_DEBUG
			printf("\n***** Center %d ******\n\n",i);
#endif

/**
 *
 * Phases 3, 4, 5 and 6 inside kmeans_ad.
 *
 */

// Kmeans with anderson_darling test. Parallelized.

#pragma css wait on (records_center)

			kmeans_ad(DIMENSIONS, records_center_count[i], &records_center[records_center_mempos[i]*DIMENSIONS], &centers[i*DIMENSIONS], &new_centers[i*DIMENSIONS], &splitlist[i], adcv, CSS_NUM_SPUS, block_records);

		}

//#pragma css barrier

// Check for new centers, merge them with the old ones or, in case of no splits, stops the algorithm.

		for (i = 0;  i < CENTERS;  i++) {

			if (splitlist[i]) {

#pragma css wait on (new_centers)

				copy_float(&centers[CENTERS*DIMENSIONS+NEW_CENTERS*DIMENSIONS], &new_centers[i*DIMENSIONS], DIMENSIONS);
//				memcpy(&centers[CENTERS*DIMENSIONS+NEW_CENTERS*DIMENSIONS], &new_centers[i*DIMENSIONS], DIMENSIONS * sizeof(float));
				NEW_CENTERS++;
				split++;
			}
			else {
				nosplit++;
			}
		}

		printf("\n######### LOOP REPORT #########\n\n");

		printf("\n SPLIT: %d  NOSPLIT: %d    NEW TOTAL: %d\n\n", split, nosplit,(CENTERS+NEW_CENTERS));

		printf("\n######### END LOOP REPORT #########\n\n");

		CENTERS += NEW_CENTERS;

		if (!split)
			break;

	}

	printf("\n######### FINAL REPORT #########\n\n");

	maintime_int(1);

	for (i = 0;  i < CENTERS;  i++) {
		pVector("CNTf",i,&centers[i*DIMENSIONS],DIMENSIONS);
		printf("\n");
	}

	printf("\n######### END FINAL REPORT #########\n\n");

#ifdef LOG
	// Save data into a file
	FILE *fp;

	fp = fopen("gmeans.log","w");
    if ( fp == NULL ) {
           printf("Cannot open gmeans log file\n");
    }
    else {
    	fprintf(fp,"Points: \n");
    	for (i = 0; i < NUMBER_OF_RECORDS; i++) {
    		for (j = 0; j < DIMENSIONS; j++) {
        		fprintf(fp, "%f\t",records[i*DIMENSIONS+j]);
    		}
    		fprintf(fp,"\n");
    	}
    	fprintf(fp,"\nCenters: \n");
    	for (i = 0; i < CENTERS; i++) {
    		for (j = 0; j < DIMENSIONS; j++) {
        		fprintf(fp, "%f\t",centers[i*DIMENSIONS+j]);
    		}
    		fprintf(fp,"\n");
    	}
    }
    fclose (fp);


    fp = fopen("gmeans.csv","w");
    if ( fp == NULL ) {
           printf("Cannot open gmeans csv file\n");
    }
    else {
    	for (i = 0; i < NUMBER_OF_RECORDS; i++) {
    		for (j = 0; j < DIMENSIONS; j++) {
        		fprintf(fp, "%f,",records[i*DIMENSIONS+j]);
    		}
    		fprintf(fp,"%d\n", assigned_centers[i]);
    	}
    }
    fclose (fp);
#endif

#pragma css finish

	free(centers);
	free(records);
	free(assigned_centers);
	free(new_centers);
	free(splitlist);
	free(records_center);
	free(records_center_count);
	free(records_center_mempos);
	free(records_center_mempos_tmp);
//	free(cluster_records_counter);

	return 0;
}
