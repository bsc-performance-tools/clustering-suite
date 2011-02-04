/*
 * tools.c
 *
 *  Created on: Jun 7, 2010
 *      Author: aislan
 */

#include "tools.h"

double maintime_int(int print) {

        double elapsed_seconds = 0;
        static struct timeval t1; /* var for previous time stamp */
        static struct timeval t2; /* var of current time stamp */

        if (gettimeofday(&t2, NULL) == -1) {
                perror("gettimeofday");
                exit(9);
        }

        if (print) {
                elapsed_seconds = (t2.tv_sec - t1.tv_sec) + (t2.tv_usec - t1.tv_usec) * 1e-6;
                printf("Time spent [%.2fs] \n", elapsed_seconds);
        }

        t1 = t2;
        return elapsed_seconds;
}

void pVector(char *label,int count, float *vec, int DIM) {
        int i;
        if (count != -1) {
                printf("Vec[%s%d] = (",label,count);
        }
        else {
                printf("Vec[%s] = (",label);
        }
        for (i=0; i<DIM-1; i++)
                printf("%9.3f, ",vec[i]);
        printf("%9.3f) ",vec[i]);
}

void clear_int (int *array, int SIZE){
	memset(array, 0, SIZE * sizeof(int));
}

void clear_float (float *array, int SIZE){
	memset(array, 0, SIZE * sizeof(float));
}

void copy_int (int *dst, int *orig, int SIZE){
	memcpy(dst, orig, SIZE * sizeof(int));
}

void copy_float (float *dst, float *orig, int SIZE){
	memcpy(dst, orig, SIZE * sizeof(float));
}
