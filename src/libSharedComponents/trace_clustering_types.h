/*****************************************************************************\
 *                        ANALYSIS PERFORMANCE TOOLS                         *
 *                             ClusteringSuite                               *
 *   Infrastructure and tools to apply clustering analysis to Paraver and    *
 *                              Dimemas traces                               *
 *                                                                           *
 *****************************************************************************
 *     ___     This library is free software; you can redistribute it and/or *
 *    /  __         modify it under the terms of the GNU LGPL as published   *
 *   /  /  _____    by the Free Software Foundation; either version 2.1      *
 *  /  /  /     \   of the License, or (at your option) any later version.   *
 * (  (  ( B S C )                                                           *
 *  \  \  \_____/   This library is distributed in hope that it will be      *
 *   \  \__         useful but WITHOUT ANY WARRANTY; without even the        *
 *    \___          implied warranty of MERCHANTABILITY or FITNESS FOR A     *
 *                  PARTICULAR PURPOSE. See the GNU LGPL for more details.   *
 *                                                                           *
 * You should have received a copy of the GNU Lesser General Public License  *
 * along with this library; if not, write to the Free Software Foundation,   *
 * Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA          *
 * The GNU LEsser General Public License is contained in the file COPYING.   *
 *                                 ---------                                 *
 *   Barcelona Supercomputing Center - Centro Nacional de Supercomputacion   *
\*****************************************************************************/

/* -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=- *\

  $URL:: https://svn.bsc.es/repos/ptools/prv2dim/                          $:

  $Rev:: 478                        $:  Revision of last commit
  $Author:: jgonzale                $:  Author of last commit
  $Date:: 2010-10-28 13:58:59 +0200 $:  Date of last commit

\* -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=- */

#ifndef _EXTRACTION_TYPES_H_
#define _EXTRACTION_TYPES_H_

#include <clustering_types.h>

typedef UINT64 duration_t;

typedef enum { NoSampling, Space, Time } sampling_type_t;

typedef enum { UndefinedParameter, SingleEventParameter, DerivedEventParameter} parameter_type_t;

typedef enum { UndefinedOperation, Add, Substract, Multiply, Divide} derived_event_op_t;

typedef enum { UndefinedInputFile, ParaverTrace, DimemasTrace, ClusteringCSV } input_file_t;

typedef enum { CompleteBurst = 0, MissingDataBurst, DurationFilteredBurst, RangeFilteredBurst } burst_type_t;

typedef UINT32 event_type_t;
typedef UINT64 event_value_t;

typedef UINT64 timestamp_t;

typedef UINT32 task_id_t;
typedef UINT32 thread_id_t;
typedef size_t line_t;

typedef double percentage_t;

/* Pseudo-event type for durations */
static const event_type_t DURATION_EVENT_TYPE   = 1;

/* Default values, should not be used */
static const double DEFAULT_DBSCAN_EPS          = 0.01;
static const INT32  DEFAULT_DBSCAN_MINPOINTS    = 4;
static const UINT64 DEFAULT_DURATION_FILTER     = 0;
static const double DEFAULT_FILTER_THRESHOLD    = 3.0;

/* Filtered cluster ids for Paraver Output */
static const cluster_id_t MISSING_DATA_CLUSTERID       = UNCLASSIFIED+1;
static const cluster_id_t DURATION_FILTERED_CLUSTERID  = UNCLASSIFIED+2;
static const cluster_id_t RANGE_FILTERED_CLUSTERID     = UNCLASSIFIED+3;
static const cluster_id_t THRESHOLD_FILTERED_CLUSTERID = UNCLASSIFIED+4;

/* Paraver offset (add the previous cluster ids + reserved 0 value (Paraver End) */
static const INT32 PARAVER_OFFSET = 5;

/* # Instance, TaskId, ThreadId, Begin_Time, End_Time, Duration, Line */
static const INT32 CSV_HEADING_FIELDS = 7;
static const INT32 CSV_DURATION_FIELD = 6;

#endif
