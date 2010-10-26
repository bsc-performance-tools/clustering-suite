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

  $URL:: https://svn.bsc.#$:  File
  $Rev:: 20               $:  Revision of last commit
  $Author:: jgonzale      $:  Author of last commit
  $Date:: 2010-03-09 17:1#$:  Date of last commit

\* -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=- */

#ifndef _CLUSTERING_TYPES_H_
#define _CLUSTERING_TYPES_H_

#include <stdlib.h>
#include <stddef.h>
#include <types.h>
#include <string>
using std::string;

#include <limits>
using std::numeric_limits;

typedef enum { UndefinedAlgorithm, DBSCAN_Algorithm } clustering_algorithm_t;

typedef INT32 cluster_id_t;

typedef double percentage_t;

static const string UNDEFINED = "Undefined";
static const INT32  NOT_FOUND = -1;

static const double MAX_DOUBLE = numeric_limits<double>::max();
static const double MIN_DOUBLE = numeric_limits<double>::min();

static const cluster_id_t UNCLASSIFIED    = -5; /* Just to uniform generate GNUplots when extracting data */
static const cluster_id_t NOISE_CLUSTERID = 0;
static const cluster_id_t MIN_CLUSTERID   = NOISE_CLUSTERID+1;

#endif
