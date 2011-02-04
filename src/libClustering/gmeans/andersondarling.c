/*
 * andersondarling.c
 *
 *  Created on: 11-ago-2009
 *      Author: aislan
 */

#include "andersondarling.h"

// comparing function for sort function
int compare_floats (const float *a, const float *b) {
	if (*a > *b)
		return 1;
	else
		return -1;

	return 0;
}


//sorting function
void sort(float *x, int length) {
	qsort(x, length, sizeof (float), compare_floats);
}

//anderson&darling calculation main function
void andersondarling(float *x_temp, int n, float *a2, float sum, float sum2) {

	int i = 0;


	float mi;
	float sig;

	std_mi_task(sum, sum2, n, &mi, &sig);

/**
 *
 * Phase 5, data sort for the A&D test
 *
 */

#ifdef PSORT

	psort(x_temp, n);

#else

	sort(x_temp, n);

#endif

/**
 *
 * Phase 6, calculation of the A&D formula
 *
 */

	float s = 0;

#pragma css wait on (mi, sig)

#ifdef AD

	float *f1is = memalign (128, n * sizeof(float));
	float *f2is = memalign (128, n * sizeof(float));

	int block_records = BLOCK_SIZE;

	for (i = 0; i < n; i += block_records) {
		int number_of_records = (i + block_records > n ? n - i : block_records);
		andersondarling_f12i_task (&x_temp[i], number_of_records, mi, sig, &f1is[i], &f2is[n-i-number_of_records]);

	}

#pragma css barrier

	for (i = 0; i < n; i += block_records) {
		int number_of_records = (i + block_records > n ? n - i : block_records);
		andersondarling_si_task (number_of_records, i, n, &f1is[i], &f2is[i], &s);
	}

#pragma css wait on (s)

	*a2 = -n-s;

	free(f1is);
	free(f2is);

#else

	double f1i = 0;
	double f2i = 0;
	double si = 0;


	for (i = 0; i < n; i++) {
		f1i = normcdfd((x_temp[i]-mi)/sig);
		f2i = (1 - normcdfd((x_temp[n-i-1]-mi)/sig));

		si = (2*(i+1)-1)*(log(f1i)+log(f2i))/n;
		s += si;
	}

	*a2 = -n-s;

#endif

}

void std_mi_task(float sum, float sum2, int n, float *mean, float *std) {
	float mi = sum/n;
	float sig = sqrt((sum2/n)-(mi*mi));

	*mean = mi;
	*std = sig;
}


void andersondarling_f12i_task(float *x, int n, float mi, float sig, float *f1i, float *f2i) {
	int i = 0;

	for (i = 0; i < n; i++) {
/*		float res = normcdf((x[i]-mi)/sig); //Have to change due the imprecision of the float for large datasets...
		f1i[i] = res;
		f2i[n-i-1] = (1 - res);*/
		double res = normcdfd((x[i]-mi)/sig);
		f1i[i] = (float) res;
		f2i[n-i-1] = (float) (1 - res);
	}
}

void andersondarling_si_task(int n, int offset, int ntot, float *f1i, float *f2i, float *s) {
	int i = 0;

	float si = 0;

	for (i = 0; i < n; i++) {
		si += (2*(i+offset+1)-1)*(log(f1i[i])+log(f2i[i]))/ntot;
	}
#pragma css mutex lock (s)
	*s += si;
#pragma css mutex unlock (s)
}
