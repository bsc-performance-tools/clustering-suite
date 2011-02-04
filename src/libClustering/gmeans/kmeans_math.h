/*
 * kmeans_math.h
 *
 *  Created on: 13-ago-2009
 *      Author: aislan
 */

#ifndef KMEANS_MATH_H_
#define KMEANS_MATH_H_

#include <math.h>

float projab(float *a, float *b, float powb, int dim);
double mean(float x[], int length);
double std_calc(float *x, int length);
double std_mean(float x[], double mi, int length);
float ndf(float x);
float normcdf(float x);
double ndfd(float x);
double normcdfd(float x);

#endif /* KMEANS_MATH_H_ */
