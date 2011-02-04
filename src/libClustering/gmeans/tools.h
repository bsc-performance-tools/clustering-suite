/*
 * tools.h
 *
 *  Created on: Jun 7, 2010
 *      Author: aislan
 */

#ifndef TOOLS_H_
#define TOOLS_H_

#include <sys/time.h>
#include <malloc.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

double maintime_int(int print);

void pVector(char *label,int count, float *vec, int DIM);

#pragma css task input (SIZE) output (array[SIZE])
void clear_float (float *array, int SIZE);

#pragma css task input (SIZE) output (array[SIZE])
void clear_int (int *array, int SIZE);

#pragma css task input (SIZE, orig[SIZE]) output (dst[SIZE])
void copy_int (int *dst, int *orig, int SIZE);

#pragma css task input (SIZE, orig[SIZE]) output (dst[SIZE])
void copy_float (float *dst, float *orig, int SIZE);

#endif /* TOOLS_H_ */
