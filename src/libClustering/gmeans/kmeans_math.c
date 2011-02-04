/*
 * kmeans_math.c
 *
 *  Created on: 13-ago-2009
 *      Author: aislan
 */

#include "kmeans_math.h"

float projab(float *a, float *b, float powb, int dim) {
	float c = 0;

	float abdot = 0;
//	float powb = 0;

	int i = 0;

	for (i = 0; i < dim; i++) {
		abdot += a[i]*b[i];
//		powb += b[i]*b[i];
	}
	c = abdot / powb;

	return c;

//#endif

}

// Calculate the mean
double mean(float *x, int length) {
	double sum = 0;
	int i;
	for (i = 0; i < length; i++) {
		sum += x[i];
	}
	double mean = sum / length;
	return mean;
}

//Calculate the standard deviation
double std_calc(float *x, int length) {
	double mi = mean(x,length);
	double sum = 0;
	int i = 0;
	for (i = 0; i < length; i++) {
		sum += pow(x[i] - mi, 2);
	}
	double sig = sqrt(sum/length);
	return sig;
}

//Calculate the standard deviation
double std_mean(float *x, double mi, int length) {
	double sum = 0;
	int i = 0;
	for (i = 0; i < length; i++) {
		sum += pow(x[i] - mi, 2);
	}
	double sig = sqrt(sum/length);
	return sig;
}

// standard normal density function
float ndf(float x) {
	float result = 0.398942280401433 * exp(-x * x / 2);  // 1/sqrt(2*PI)=0.398942280401433
	return result;
}

// standard normal comulated density function
float normcdf(float x) {
	float result;
	if (x < -7.)
		result = ndf(x) / sqrt(1. + x * x);
	else if (x > 7.) {
		result = 1. - normcdf(-x);
/*
                x = -x;
                float tempx =  ndf(x) / sqrt(1. + x * x);
                result = 1. - tempx;
 */	}
	else {
		result = 0.2316419;
		static float a[5] = { 0.31938153, -0.356563782, 1.781477937, -1.821255978, 1.330274429 };
		result = 1. / (1 + result * fabs(x));
		result = 1 - ndf(x) * (result * (a[0] + result * (a[1] + result * (a[2] + result * (a[3] + result * a[4])))));
		if (x <= 0.)
			result = 1. - result;
	}
	return result;
}

// standard normal density function
double ndfd(float x) {
	double result = 0.398942280401433 * exp(-x * x / 2);  // 1/sqrt(2*PI)=0.398942280401433
	return result;
}

// standard normal comulated density function
double normcdfd(float x) {
	double result;
	if (x < -7.)
		result = ndfd(x) / sqrt(1. + x * x);
	else if (x > 7.) {
		result = 1. - normcdfd(-x);
/*
                x = -x;
                float tempx =  ndf(x) / sqrt(1. + x * x);
                result = 1. - tempx;
 */	}
	else {
		result = 0.2316419;
		static double a[5] = { 0.31938153, -0.356563782, 1.781477937, -1.821255978, 1.330274429 };
		result = 1. / (1 + result * fabs(x));
		result = 1 - ndfd(x) * (result * (a[0] + result * (a[1] + result * (a[2] + result * (a[3] + result * a[4])))));
		if (x <= 0.)
			result = 1. - result;
	}
	return result;
}



/*
// standard normal density function
double ndf(float x) {
	double result = 0.398942280401433 * exp(-x * x / 2);  // 1/sqrt(2*PI)=0.398942280401433
	return result;
}

// standard normal comulated density function
double normcdf(float x) {
	double result;
	if (x < -7.)
		result = ndf(x) / sqrt(1. + x * x);
	else if (x > 7.)
		result = 1. - normcdf(-x);
	else {
		result = 0.2316419;
		static double a[5] = { 0.31938153, -0.356563782, 1.781477937, -1.821255978, 1.330274429 };
		result = 1. / (1 + result * fabs(x));
		result = 1 - ndf(x) * (result * (a[0] + result * (a[1] + result * (a[2] + result * (a[3] + result * a[4])))));
		if (x <= 0.)
			result = 1. - result;
	}
	return result;
}
*/

//Calculate the standard deviation
/*double std(double *x, int length) {
	double mi = mean(x,length);
	double sum = 0;
	int i = 0;
	for (i = 0; i < length; i++) {
		sum += pow(x[i] - mi, 2);
	}
	double sig = sqrt(sum/length);
	return sig;
}

// Calculate the mean
double mean(double *x, int length) {
	double sum = 0;
	int i;
	for (i = 0; i < length; i++) {
		sum += x[i];
	}
	double mean = sum / length;
	return mean;
}
// standard normal density function
/*double ndf(double x) {
	return 0.398942280401433 * exp(-x * x / 2);  // 1/sqrt(2*PI)=0.398942280401433
}
// standard normal comulated density function
double normcdf(double x) {
	double result;
	if (x < -7.)
		result = ndf(x) / sqrt(1. + x * x);
	else if (x > 7.)
		result = 1. - normcdf(-x);
	else {
		result = 0.2316419;
		static double a[5] = { 0.31938153, -0.356563782, 1.781477937, -1.821255978, 1.330274429 };
		result = 1. / (1 + result * fabs(x));
		result = 1 - ndf(x) * (result * (a[0] + result * (a[1] + result * (a[2] + result * (a[3] + result * a[4])))));
		if (x <= 0.)
			result = 1. - result;
	}
	return result;
}*/



