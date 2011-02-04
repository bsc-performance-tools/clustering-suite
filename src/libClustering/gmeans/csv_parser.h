/*
 * csv_parser.h
 *
 *  Created on: 12-ago-2009
 *      Author: aislan
 */

#ifndef CSV_PARSER_H_
#define CSV_PARSER_H_


#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAXFLDS 200     /* maximum possible number of fields */
#define MAXFLDSIZE 32   /* longest possible field + 1 = 31 byte field */

void parse( char *record, char *delim, char arr[][MAXFLDSIZE],int *fldcnt);
void getDataset(char *filepath, float *records, int NUMREC, int DIM);

#endif /* CSV_PARSER_H_ */
