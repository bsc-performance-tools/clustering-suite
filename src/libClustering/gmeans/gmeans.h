/*
 * gmeans.h
 *
 *  Created on: Aug 25, 2009
 *      Author: aislan
 */

#ifndef GMEANS_H_
#define GMEANS_H_


#include "kmeans.h"
#include "kmeans_math.h"
#include "andersondarling.h"
#include "get_ad_cv.h"
#include "csv_parser.h"
#include "dataorg.h"
#include "tools.h"

//#define MAIN_DEBUG
//#define MAIN_TIME
//#define LOG
//#define PAUSE
//#define AMDAHL
//#define TRACE
//#define TRACE_KM
//#define TRACE_AD

#define ALPHA	 		0.00001

//#define REPORT	// Show report in the end of the code.
//#define KMEANS_DEBUG
#define KMEANSAD_DEBUG 1
//#define KMEANS_TIME
//#define PAUSE	// Stop during the iterations



/*
 * When the ratio of record movement across custers (led by centroids) hits
 * this low bound, the algorithms does not perform any more iterations.
 */

#define TERMINATION_THRESHOLD	0.01f

/*
 * Iteration hard limit.
 */

#define MAX_ITERATIONS			100
#define MAX_ITERATIONS_2KM		10
/*
 * Max number of Centers.
 */

#define MAX_CENTERS				512

/*
 * Size hint for the record block.
 */

#ifdef CellSs
#define BLOCK_SIZE             16384
#else
#define BLOCK_SIZE             8192
#endif


int gmeans_c(int argc, char **argv);

#endif /* GMEANS_H_ */
