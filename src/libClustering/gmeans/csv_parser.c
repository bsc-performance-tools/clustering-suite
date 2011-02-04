/*
 * csv_parser.c
 *
 *  Created on: 12-ago-2009
 *      Author: aislan
 */
#include "csv_parser.h"

void parse(char *record, char *delim, char arr[][MAXFLDSIZE], int *fldcnt) {
	char*p = strtok(record, delim);
	int fld = 0;

	while (p) {
		strcpy(arr[fld], p);
		fld++;
		p = strtok('\0', delim);
	}
	*fldcnt = fld;
}

void getDataset(char *filepath, float *records, int NUMREC, int DIM) {
	char tmp[1024] = { 0x0 };
	int fldcnt = 0;
	char arr[MAXFLDS][MAXFLDSIZE] = { 0x0 };
	int recordcnt = 0;
	FILE *in = fopen(filepath, "r"); /* open file on command line */

	if (in == NULL) {
		perror("File open error");
		exit(EXIT_FAILURE);
	}
	int i = 0;
	int j = 0;
	for (i = 0; i < NUMREC && (fgets(tmp, sizeof(tmp), in) != 0); i++) /* read a record */
	{
		recordcnt++;
		parse(tmp, ",", arr, &fldcnt); /* whack record into fields */

		for (j = 0; j < DIM; j++) {
			records[i * DIM + j] = atof(arr[j]);
		}
	}
	fclose(in);
}
