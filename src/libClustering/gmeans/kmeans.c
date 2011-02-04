#include "kmeans.h"


/******************************************************************************
 *                                                                            *
 * The big K-means, it runs the K-means for all data.						  *
 *                                                                            *
 ******************************************************************************/

int big_kmeans (int DIMENSIONS, int CENTERS, int NUMBER_OF_RECORDS, float *records, float *centers, int *assigned_centers, int CSS_NUM_CPUS, int CSS_NUM_SPUS, int block_records, int *cluster_records_counter) {

	/* Algorithm parameters */
	int number_of_records_cpu, block_records_cpu;
	/* Auxiliary variables */
	int k, i, j;
	int iteration, changed;
	/* Other computations */
	int sizeofdistance;
	/* Data arrays */
	float *newcenters;
	int *newcenters_histograms;

	block_records_cpu = BLOCK_SIZE;

	/* Print precalculated information */

#ifdef KMEANS_DEBUG
	printf("K-MEANS called\n");
	printf("Number of data points is %d\n", NUMBER_OF_RECORDS);
	printf("Number of centers is %d\n", CENTERS);
	printf("Number of dimensions %d\n\n", DIMENSIONS);
#endif

	/* Allocate space for data */
	newcenters = memalign(128, CENTERS * DIMENSIONS * sizeof(float));
	newcenters_histograms = memalign(128, (CENTERS + 1) * sizeof(int));

	/* Clear assigned centers */
	memset(assigned_centers, -1, NUMBER_OF_RECORDS * sizeof(int));

	changed = 0;
	iteration = 0;
	sizeofdistance = DIMENSIONS * 3 + 1;

	/* Main loop */
	do {

		clear_float (newcenters, CENTERS*DIMENSIONS);
		clear_int (newcenters_histograms, CENTERS+1);

		for (i = 0; i < NUMBER_OF_RECORDS; i += block_records_cpu) {
			number_of_records_cpu = (i + block_records_cpu > NUMBER_OF_RECORDS ? NUMBER_OF_RECORDS - i : block_records_cpu);

			kmeans_cpu_task(DIMENSIONS, CENTERS, number_of_records_cpu, &records[i
					* DIMENSIONS], centers, &assigned_centers[i],
					newcenters, newcenters_histograms, CSS_NUM_SPUS, block_records);
		}

		iteration++;

#pragma css wait on (newcenters_histograms)

		changed = newcenters_histograms[CENTERS];
		if ((float) changed / NUMBER_OF_RECORDS < TERMINATION_THRESHOLD) {
			break;
		}
		if (iteration >= MAX_ITERATIONS) {
			break;
		}

		recalculate_centers_task(DIMENSIONS, CENTERS, centers,
				newcenters, newcenters_histograms);

	} while (1);

#ifdef KMEANS_DEBUG
	printf("\nKmeans Number of ITERATIONS = #%d\n", iteration);
#endif

	for(i = 0; i < CENTERS; i++) {
		cluster_records_counter[i] = newcenters_histograms[i];
	}

	free(newcenters);
	free(newcenters_histograms);

	return 0;
}


/******************************************************************************
 *                                                                            *
 * K-means used by the Kmeans_ad due its optimization for two centers only    *
 *                                                                            *
 ******************************************************************************/

int twocenters_kmeans (int DIMENSIONS, int CENTERS, int NUMBER_OF_RECORDS, float *records, float *centers, int *assigned_centers, int CSS_NUM_SPUS, int block_records) {

	/* Algorithm parameters */
	int number_of_records;
	/* Auxiliary variables */
	int k, i, j;
	int iteration, changed;
	/* Other computations */
	int sizeofdistance;
	/* Data arrays */
	float *newcenters;
	int *newcenters_histograms;

	/* Print precalculated information */

#ifdef KMEANS_DEBUG
	printf("K-MEANS called\n");
	printf("Number of data points is %d\n", NUMBER_OF_RECORDS);
	printf("Number of centers is %d\n", CENTERS);
	printf("Number of dimensions %d\n\n", DIMENSIONS);
#endif

	/* Allocate space for data */
	newcenters = memalign(128, CENTERS * DIMENSIONS * sizeof(float));
	newcenters_histograms = memalign(128, (CENTERS + 1) * sizeof(int));

	/* Clear assigned centers */
	memset(assigned_centers, -1, NUMBER_OF_RECORDS * sizeof(int));

	changed = 0;
	iteration = 0;
	sizeofdistance = DIMENSIONS * 3 + 1;

	/* Main loop */
	do {

		clear_float (newcenters, CENTERS*DIMENSIONS);
		clear_int (newcenters_histograms, CENTERS+1);

		for (i = 0; i < NUMBER_OF_RECORDS; i += block_records) {
			number_of_records = (i + block_records > NUMBER_OF_RECORDS ? NUMBER_OF_RECORDS - i : block_records);

			kmeans_calculate2(DIMENSIONS, CENTERS, number_of_records, &records[i
					* DIMENSIONS], centers, &assigned_centers[i],
					newcenters, newcenters_histograms);
		}

		iteration++;

#pragma css wait on (newcenters_histograms)

		changed = newcenters_histograms[CENTERS];
		if ((float) changed / NUMBER_OF_RECORDS < TERMINATION_THRESHOLD) {
			break;
		}
		if (iteration >= MAX_ITERATIONS_2KM) {
			break;
		}

		recalculate_centers_task(DIMENSIONS, CENTERS, centers,
				newcenters, newcenters_histograms);

	} while (1);

#ifdef KMEANS_DEBUG
	printf("\nKmeans Number of ITERATIONS = #%d\n", iteration);
#endif

	free(newcenters);
	free(newcenters_histograms);

	return 0;
}

/******************************************************************************
 *                                                                            *
 * Guassian test. Kmeans with two centers followed by the vector projection   *
 * and by the anderson and darling test.									  *
 *                                                                            *
 ******************************************************************************/

void kmeans_ad (int DIMENSIONS, int NUMBER_OF_RECORDS, float *records, float *orig_center, float *new_center, int *split, float AD_CV, int CSS_NUM_SPUS, int block_records) {

	int CENTERS = 2;
	int i, j;
	int *assigned_centers;

	if (NUMBER_OF_RECORDS < 2) {

#ifdef KMEANSAD_DEBUG
		printf("\n\n######### no split... NUM_REC < 2 #########\n\n");
#endif
		*split = 0;
		return;
	}
	float *centers;

	// Alocacao de 2 centros a mais para nao gerar erro na rotina de leitura do KMeans.
	centers = memalign(128, 4 * DIMENSIONS * sizeof(float));

	clear_float(&centers[2*DIMENSIONS], 2*DIMENSIONS);

	for (i = 0; i < 2; i++) {
		copy_float(&centers[i * DIMENSIONS], &records[i * DIMENSIONS], DIMENSIONS);
	}

	assigned_centers = memalign(128, NUMBER_OF_RECORDS * sizeof(int));

/**
 *
 * Phase 3, two-centers kmeans
 *
 */


	twocenters_kmeans(DIMENSIONS, CENTERS, NUMBER_OF_RECORDS, records, centers, assigned_centers, CSS_NUM_SPUS, block_records);


/**
 *
 * Phase 4, vector projection
 *
 */

	// Calculation of vector vec
	float vec[DIMENSIONS];
	float powvec = 0;
	for (j = 0; j < DIMENSIONS; j++) {
		vec[j] = centers[0*DIMENSIONS + j] - centers[1*DIMENSIONS + j];
		powvec += vec[j]*vec[j];
	}

	// projection in the vector c1 - c2
	float *p_records;
	p_records = memalign(128, NUMBER_OF_RECORDS * sizeof(float));

	float sum = 0;
	float sum2 = 0;

#ifdef PROJAB

	block_records = BLOCK_SIZE;

	for (i = 0; i < NUMBER_OF_RECORDS; i += block_records) {
				int number_of_records = (i + block_records > NUMBER_OF_RECORDS ? NUMBER_OF_RECORDS - i : block_records);
				projab_cpu_task (DIMENSIONS, number_of_records, &records[i * DIMENSIONS], vec, powvec, &p_records[i], &sum, &sum2);
	}

#pragma css barrier

#else

	for (j = 0; j < NUMBER_OF_RECORDS; j++) {
		p_records[j] = projab(&records[j*DIMENSIONS], vec, powvec, DIMENSIONS);
		sum += p_records[j];
		sum2 += p_records[j]*p_records[j];
	}

#endif


/**
 *
 * Phases 5 and 6 inside the andersondarling()
 *
 */

	float ad;
	andersondarling(p_records, NUMBER_OF_RECORDS, &ad, sum, sum2);

	if (ad >= AD_CV) {
#ifdef KMEANSAD_DEBUG
		printf("\n\n######### SPLIT! AD = %f, CV = %f #########\n\n", ad, AD_CV);
#endif
//		copy_float(orig_center, &centers[0*DIMENSIONS], DIMENSIONS);
//		copy_float(new_center, &centers[1*DIMENSIONS], DIMENSIONS);
		memcpy(orig_center, &centers[0*DIMENSIONS], DIMENSIONS * sizeof(float));
		memcpy(new_center, &centers[1*DIMENSIONS], DIMENSIONS * sizeof(float));

		*split = 1;
	}
	else {
#ifdef KMEANSAD_DEBUG
		printf("\n\n######### no split... AD = %f, CV = %f #########\n\n", ad, AD_CV);
#endif
		*split = 0;
	}

	free(centers);
	free(assigned_centers);
	free(p_records);

}


/******************************************************************************
 *                                                                            *
 * Collect new centroid data from current centroids and the given chunk of    *
 * records.  The output data will be combined toghether from the other tasks. *
 *                                                                            *
 ******************************************************************************/

void kmeans_calculate(int DIMENSIONS, int CENTERS, int number_of_records,
		float *records, float *centers, int *assigned_center,
		float *newcenters, int *newcenters_histogram) {

	int i, j;
	int min;

	float *newcenters_local;
	int *newcenters_histogram_local;
	newcenters_local = memalign(128, CENTERS * DIMENSIONS * sizeof(float));
	newcenters_histogram_local = memalign(128, (CENTERS + 1) * sizeof(int));
	memset(newcenters_local, 0, CENTERS * DIMENSIONS * sizeof(float));
	memset(newcenters_histogram_local, 0, (CENTERS + 1) * sizeof(int));

	for (i = 0; i < number_of_records; i++) {
		/* Find this record's nearest centroid */
		min = compare_to_centers_cpu(&records[i * DIMENSIONS], DIMENSIONS, CENTERS, centers);
		if (assigned_center[i] != min) {
			newcenters_histogram_local[CENTERS]++;
			assigned_center[i] = min;
		}
		/* Update centroid's cluster information */
		newcenters_histogram_local[min]++;
		for (j = 0; j < DIMENSIONS; j++)
			newcenters_local[min * DIMENSIONS + j] += records[i * DIMENSIONS + j];
	}

#pragma css mutex lock (newcenters_histogram)
	for (i = 0; i < CENTERS + 1; i++)
		newcenters_histogram[i] += newcenters_histogram_local[i];
#pragma css mutex unlock (newcenters_histogram)
#pragma css mutex lock (newcenters)
	for (i = 0; i < CENTERS * DIMENSIONS; i++)
		newcenters[i] += newcenters_local[i];
#pragma css mutex unlock (newcenters)

	free(newcenters_local);
	free(newcenters_histogram_local);

}

/******************************************************************************
 *                                                                            *
 * Collect new centroid data from current centroids and the given chunk of    *
 * records.  The output data will be combined toghether from the other tasks. *
 * Same as kmeans_calculate, but optimized for 2 centers only.				  *
 *                                                                            *
 ******************************************************************************/

void kmeans_calculate2(int DIMENSIONS, int CENTERS, int number_of_records,
		float *records, float *centers, int *assigned_center,
		float *newcenters, int *newcenters_histogram) {

	int i, j;
	int min;

	float *newcenters_local;
	int *newcenters_histogram_local;
	newcenters_local = memalign(128, CENTERS * DIMENSIONS * sizeof(float));
	newcenters_histogram_local = memalign(128, (CENTERS + 1) * sizeof(int));
	memset(newcenters_local, 0, CENTERS * DIMENSIONS * sizeof(float));
	memset(newcenters_histogram_local, 0, (CENTERS + 1) * sizeof(int));

	for (i = 0; i < number_of_records; i++) {
		/* Find this record's nearest centroid */
		min = compare_to_2centers_cpu(&records[i * DIMENSIONS], DIMENSIONS, centers);
		if (assigned_center[i] != min) {
			newcenters_histogram_local[CENTERS]++;
			assigned_center[i] = min;
		}
		/* Update centroid's cluster information */
		newcenters_histogram_local[min]++;
		for (j = 0; j < DIMENSIONS; j++)
			newcenters_local[min * DIMENSIONS + j] += records[i * DIMENSIONS + j];
	}

#pragma css mutex lock (newcenters_histogram)
	for (i = 0; i < CENTERS + 1; i++)
		newcenters_histogram[i] += newcenters_histogram_local[i];
#pragma css mutex unlock (newcenters_histogram)
#pragma css mutex lock (newcenters)
	for (i = 0; i < CENTERS * DIMENSIONS; i++)
		newcenters[i] += newcenters_local[i];
#pragma css mutex unlock (newcenters)

	free(newcenters_local);
	free(newcenters_histogram_local);

}

/******************************************************************************
 *                                                                            *
 * Combine results produced by the tasks and calculate new centroids for the  *
 * next clustering iteration.                                                 *
 *                                                                            *
 ******************************************************************************/
void recalculate_centers_task(int DIMENSIONS, int CENTERS,
		float *centers, float *newcenters, int *newcenters_histograms) {

	int i, j, k;

	/* Re-average centroids directly on the centroids array */
	for (i = 0; i < CENTERS; i++)
		for (j = 0; j < DIMENSIONS; j++) {
			if (newcenters_histograms[i] > 0)
				centers[i * DIMENSIONS + j] = newcenters[i * DIMENSIONS + j] / newcenters_histograms[i];
			else
				centers[i * DIMENSIONS + j] = -1;
		}
}

/******************************************************************************
 *                                                                            *
 * Find the nearest centroid for a record.  Returns the center identifier     *
 * (index).                                                                   *
 *                                                                            *
 * This is a straightforward adaptation of the original function              *
 *                                                                            *
 ******************************************************************************/

int compare_to_centers_cpu(float *record, int dimension, int number_of_centers,
		float *centers) {

	short k;
	short i, j;
	short min_id = 0;
	float min = 1e18;
	short blocksize = 12;

	short loopcount = 0;
	short extra = dimension;

	float *recordsPointer = record;

	// Creation of 4 groups of variables to do 4 calculations in one loop.

	float *center1 = centers;
	float distance = 0.0;

	int offsetX = dimension;
	float *center2 = centers + offsetX;
	float Cdistance = 0.0;

	offsetX += dimension;
	float *center3 = centers + offsetX;
	float Ddistance = 0.0;

	offsetX += dimension;
	float *center4 = centers + offsetX;
	float Edistance = 0.0;


	//for each 4 centers.
	//Aislan: Removed the -4 sub in the condition of the loop.

	for (k = 0; k < number_of_centers; k += 4) {

		int index = 0;
		distance = 0;
		Cdistance = 0;
		Ddistance = 0;
		Edistance = 0;
		index = loopcount * blocksize; // A bugfix (maybe ?)
		for (i = 0; i < extra; i++) { // TODO: SIMD it.
			float distance1 = recordsPointer[index + i] - center1[index + i];
			distance += distance1 * distance1;
			float Cdistance1 = recordsPointer[index + i] - center2[index + i];
			Cdistance += Cdistance1 * Cdistance1;
			float Ddistance1 = recordsPointer[index + i] - center3[index + i];
			Ddistance += Ddistance1 * Ddistance1;
			float Edistance1 = recordsPointer[index + i] - center4[index + i];
			Edistance += Edistance1 * Edistance1;

		}

		/* this is made spu_sel by the compiler */
		// Aislan: Added the validation in case of the k % 4 > 0, for the invalid memory positions.
		// TODO: Improve it!!!

		if (__builtin_expect((min > distance), 0)) {
			min_id = k;
			min = distance;
		}
		if (__builtin_expect((min > Cdistance) && (k + 1 < number_of_centers),
				0)) {
			min_id = k + 1;
			min = Cdistance;
		}
		if (__builtin_expect((min > Ddistance) && (k + 2 < number_of_centers),
				0)) {
			min_id = k + 2;
			min = Ddistance;
		}
		if (__builtin_expect((min > Edistance) && (k + 3 < number_of_centers),
				0)) {
			min_id = k + 3;
			min = Edistance;
		}

		center1 += dimension * 4;
		center2 += dimension * 4;
		center3 += dimension * 4;
		center4 += dimension * 4;
	} //for each center mod 4

	return min_id;
}

/**
 *
 * Same as above, but improved only for 2 centers instead of 4 centers. Used in the phase 3, two-centers kmeans.
 *
 */

int compare_to_2centers_cpu(float *record, int dimension, float *centers) {

	short i, j;
	short min_id = 0;
	float min = 1e18;
	short blocksize = 12;

	short loopcount = 0;
	short extra = dimension;

	float *recordsPointer = record;

	// Creation of 4 groups of variables to do 4 calculations in one loop.

	float *center1 = centers;
	float distance = 0.0;
	float distance_3 = 0.0;
	float distance_4 = 0.0;
	float distance_5 = 0.0;

	float *center2 = centers + dimension;
	float Cdistance = 0.0;
	float Cdistance_3 = 0.0;
	float Cdistance_4 = 0.0;
	float Cdistance_5 = 0.0;

	//for 2 centers.
	//Aislan: Removed the -4 sub in the condition of the loop.

	distance_3 = 0.0;
	distance_4 = 0.0;
	distance_5 = 0.0;

	Cdistance_3 = 0.0;
	Cdistance_4 = 0.0;
	Cdistance_5 = 0.0;

	int index = 0;

	distance = 0;
	Cdistance = 0;

	index = loopcount * blocksize; // A bugfix (maybe ?)
	for (i = 0; i < extra; i++) { // TODO: SIMD it.
		float distance1 = recordsPointer[index + i] - center1[index + i];
		distance += distance1 * distance1;
		float Cdistance1 = recordsPointer[index + i] - center2[index + i];
		Cdistance += Cdistance1 * Cdistance1;
	}

	/* this is made spu_sel by the compiler */
	// Aislan: Added the validation in case of the k % 4 > 0, for the invalid memory positions.
	// TODO: Improve it!!!

	if (min > distance) {
		min_id = 0;
		min = distance;
	}
	//		if (__builtin_expect((min > Cdistance) && (k + 1 < number_of_centers), 0)) {
	if (min > Cdistance) {
		min_id = 1;
		min = Cdistance;
	}

	return min_id;
}

void kmeans_cpu_task (int DIMENSIONS, int CENTERS, int NUMBER_OF_RECORDS, float *records, float *centers, int *assigned_centers, float *newcenters, int *newcenters_histograms, int CSS_NUM_SPUS, int block_records) {
	int number_of_records = 0;
	int i = 0;

	for (i = 0; i < NUMBER_OF_RECORDS; i += block_records) {
		number_of_records = (i + block_records > NUMBER_OF_RECORDS ? NUMBER_OF_RECORDS - i : block_records);

		kmeans_calculate(DIMENSIONS, CENTERS, number_of_records, &records[i
				* DIMENSIONS], centers, &assigned_centers[i],
				newcenters, newcenters_histograms);
	}
}

void projab_cpu_task (int DIMENSIONS, int NUMBER_OF_RECORDS, float *records, float *vec, float powvec, float *p_records, float *sum, float *sum2) {
	int j;

	float tempsum = 0;
	float tempsum2 = 0;

	float *tempp_records;

	tempp_records = malloc(NUMBER_OF_RECORDS * sizeof(float));

	for (j = 0; j < NUMBER_OF_RECORDS; j++) {
		tempp_records[j] = projab(&records[j*DIMENSIONS], vec, powvec, DIMENSIONS);
		tempsum += tempp_records[j];
		tempsum2 += tempp_records[j]*tempp_records[j];
	}
	memcpy(p_records,tempp_records,NUMBER_OF_RECORDS * sizeof(float));
#pragma css mutex lock (sum)
	*sum += tempsum;
#pragma css mutex unlock (sum)
#pragma css mutex lock (sum2)
	*sum2 += tempsum2;
#pragma css mutex unlock (sum2)

	free(tempp_records);
}

