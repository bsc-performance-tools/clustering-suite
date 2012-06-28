/*
 * andersondarling.h
 *
 *  Created on: 12-ago-2009
 *      Author: aislan
 */

#ifndef ANDERSONDARLING_H_
#define ANDERSONDARLING_H_

#include <math.h>
#include <malloc.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#include "gmeans.h"
#include "kmeans.h"
#include "kmeans_math.h"

#include "smp_superscalar_sort.h"

#define EPS 0.00000000000003

int compare_doubles(const void *a, const void *b);
int compare_floats (const void *a, const void*b);
void sort(float *x, int length);
void andersondarling(float *x, int n, float *a2, float sum, float sum2);

#pragma css task input(x[n], n, mi, sig) \
                 inout(f1i[n], f2i[n])
//target device (cell)

void andersondarling_f12i_task(float *x, int n, float mi, float sig, float *f1i, float *f2i);

#pragma css task input(n, offset, ntot) \
				 input(f1i[n], f2i[n]) \
				 inout(s) \
				 reduction (s)
//target device (cell)

void andersondarling_si_task(int n, int offset, int ntot, float *f1i, float *f2i, float *s);

#pragma css task input(sum, sum2, n) \
				 output(mean,std) \

void std_mi_task(float sum, float sum2, int n, float *mean, float *std);


#endif /* ANDERSONDARLING_H_ */
