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

  $URL::                                                                   $:

  $Rev::                            $:  Revision of last commit
  $Author::                         $:  Author of last commit
  $Date::                           $:  Date of last commit

\* -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=- */

#include "SystemMessages.hpp"

#include "GMEANS.hpp"

#include "Point.hpp"
#include "Partition.hpp"

#include <cstring>
#include <cmath>
#include <cassert>
#include <cerrno>

#include <cstdarg>

#include <algorithm>
using std::sort;

#include <iostream>
using std::cout;
using std::endl;

using std::make_pair;

const string GMEANS::NAME                  = "GMEANS";
const string GMEANS::CRITICAL_VALUE_STRING = "critical_value";
const string GMEANS::MAX_CLUSTERS_STRING   = "max_clusters";

/*****************************************************************************
 * class GMEANS implementation                                               *
 ****************************************************************************/
GMEANS::GMEANS()
{
}


GMEANS::GMEANS(map<string, string> ClusteringParameters)
{
  map<string, string>::iterator ParametersIterator;

  /* Critical Values */
  ParametersIterator = ClusteringParameters.find(GMEANS::CRITICAL_VALUE_STRING);
  if (ParametersIterator == ClusteringParameters.end())
  {
    string ErrorMessage;
    ErrorMessage = "parameter '" + GMEANS::CRITICAL_VALUE_STRING + "' not found in G-Means definition";

    SetErrorMessage(ErrorMessage);
    SetError(true);
    return;
  }
  else
  {
    char* err;
    CriticalValue = strtod(ParametersIterator->second.c_str(), &err);

    if (*err)
    {
      string ErrorMessage;
      ErrorMessage = "incorrect value for G-Means parameter '" + GMEANS::CRITICAL_VALUE_STRING + "'";

      SetErrorMessage(ErrorMessage);
      SetError(true);
      return;
    }
  }

  /* Max Clusters */
  ParametersIterator = ClusteringParameters.find(GMEANS::MAX_CLUSTERS_STRING);
  if (ParametersIterator == ClusteringParameters.end())
  {
    MaxClusters = 30;
  }
  else
  {
    char* err;
    MaxClusters = strtol(ParametersIterator->second.c_str(), &err, 0);

    if (MaxClusters > 256)
      MaxClusters = 256;

    if (*err)
    {
      string ErrorMessage;
      ErrorMessage = "incorrect value for G-Means parameter '" + GMEANS::MAX_CLUSTERS_STRING + "'";

      SetErrorMessage(ErrorMessage);
      SetError(true);
      return;
    }
  }

  return;
}


bool GMEANS::Run(const vector<const Point*>& Data,
                 Partition& DataPartition,
                 bool SimpleRun)
{

  //    CriticalValue = 20.0;
  //    MaxClusters = 60;

  int NUMBER_OF_RECORDS, DIMENSIONS, CENTERS, NEW_CENTERS;
  int CSS_NUM_SPUS = 1;
  int CSS_NUM_CPUS = 1;
  /* Auxiliary variables */
  int i, j;                                       //, k;
  //  int MAX_RECORDS_CLUSTER;
  
  /* Other computations */
  int *records_center_count;
  int *records_center_mempos;
  int *records_center_mempos_tmp;
  int *records_center_hist;
  // int *cluster_records_counter;
  float *records_center;
  
  /* Data arrays */
  float *records;
  float *centers;
  float *new_centers;
  int *assigned_centers;                          //, *assigned_new_centers;
  int *assigned_memcpy;

  /*gmeans vars */
  double adcv;
  int split = 0;
  int nosplit = 0;
  int *splitlist;

  NUMBER_OF_RECORDS = Data.size();                //100;//atoi(argv[1]);
  //	printf("******** %d ******\n", NUMBER_OF_RECORDS);

  Point *pt;
  pt = (Point *) Data[0];
  Point p = *pt;
  DIMENSIONS = p.size();                          //3;//atoi(argv[2]);
  // printf("******** %d ******\n", DIMENSIONS);

  CENTERS     = 2;                                    //atoi(argv[3]);
  NEW_CENTERS = 0;

  /* Check number of centers (to avoid uchar overflow) 
  if (CENTERS > 256)
  {
    printf("Current program only supports 256 centers\n.");
    return 2;
  }
  */

  int block_records = BLOCK_SIZE;

  maintime_int(0);

  // Calculate the critical value for adnserson&darling test
  get_ad_cv(1 - ALPHA, &adcv);

  // alocate alligned memory for the variables
  /*
  records          = (float *) memalign(128, ((NUMBER_OF_RECORDS * DIMENSIONS) + 4)* sizeof(float));
  centers          = (float *) memalign(128, MAX_CENTERS * DIMENSIONS * sizeof(float));
  assigned_centers = (int *)   memalign(128, NUMBER_OF_RECORDS * sizeof(int));
  assigned_memcpy  = (int *)   memalign(128, NUMBER_OF_RECORDS * sizeof(int));
  */

  posix_memalign((void**) &records,          128, ((NUMBER_OF_RECORDS * DIMENSIONS) + 4)* sizeof(float));
  posix_memalign((void**) &centers,          128, MAX_CENTERS * DIMENSIONS * sizeof(float));
  posix_memalign((void**) &assigned_centers, 128, NUMBER_OF_RECORDS * sizeof(int));
  posix_memalign((void**) &assigned_memcpy,  128, NUMBER_OF_RECORDS * sizeof(int));

  memset(records, 0, ((NUMBER_OF_RECORDS * DIMENSIONS) + 4) * sizeof(float));
  memset(centers, 0, MAX_CENTERS * DIMENSIONS * sizeof(float));
  memset(assigned_centers, 0, NUMBER_OF_RECORDS * sizeof(int));

  //Alocated memory for the temporary variables for kmeans_ad
  /*
  records_center            = (float *) memalign(128, ((NUMBER_OF_RECORDS * DIMENSIONS) + 4) * sizeof(float));
  records_center_count      = (int *)   memalign(128, MAX_CENTERS * sizeof(int));
  records_center_mempos     = (int *)   memalign(128, MAX_CENTERS * sizeof(int));
  records_center_mempos_tmp = (int *)   memalign(128, MAX_CENTERS * sizeof(int));
  */

  posix_memalign((void**) &records_center,            128, ((NUMBER_OF_RECORDS * DIMENSIONS) + 4) * sizeof(float));
  posix_memalign((void**) &records_center_count,      128, MAX_CENTERS * sizeof(int));
  posix_memalign((void**) &records_center_mempos,     128, MAX_CENTERS * sizeof(int));
  posix_memalign((void**) &records_center_mempos_tmp, 128, MAX_CENTERS * sizeof(int));  
  
  int num_hist = (NUMBER_OF_RECORDS / block_records) + 2;
  records_center_hist = (int *) memalign(128, num_hist * MAX_CENTERS * sizeof(int));

  //	cluster_records_counter = memalign(128, MAX_CENTERS * sizeof(int));
  new_centers = (float *) memalign(128, MAX_CENTERS * DIMENSIONS * sizeof(float));

  splitlist = (int *) malloc(MAX_CENTERS * sizeof(int));

  for (i = 0; i < NUMBER_OF_RECORDS; i++)
  {
    Point* InputPoint = (Point *) Data[i];
    for (j = 0; j < DIMENSIONS; j++)
    {
                                                  //rand() / 100000.0f + j;
      records[i * DIMENSIONS + j] = (float) (*InputPoint)[j];
    }
  }

  adcv = CriticalValue;

  /* Initialize the centers as the first records of the dataset */
  for (i = 0; i < CENTERS; i++)
    memcpy(&centers[i * DIMENSIONS], &records[i * DIMENSIONS], DIMENSIONS
      * sizeof(float));

  maintime_int(0);

  while (CENTERS < MaxClusters)
  {

    // Reset counters
    NEW_CENTERS = 0;
    split = 0;
    nosplit = 0;

    clear_int(records_center_count, MAX_CENTERS);

    /**
     *
     * 		1. Big-Kmeans... Whole dataset
     *
     */

    big_kmeans(DIMENSIONS, CENTERS, NUMBER_OF_RECORDS, records, centers,
      assigned_centers, CSS_NUM_CPUS, CSS_NUM_SPUS, block_records,
      records_center_count);

    //#pragma css barrier

    /**
     *
     * 2. Data organization phase
     *
     */

    // Order records in order according to the assigned center

    memset(records_center_count, 0, CENTERS * sizeof(int));
    memset(records_center_mempos, 0, CENTERS * sizeof(int));
    memset(records_center_mempos_tmp, 0, CENTERS * sizeof(int));
    //		memset(records_center, 0, NUMBER_OF_RECORDS * DIMENSIONS * sizeof(float));

    for (i = 0; i < NUMBER_OF_RECORDS; i++)
    {
      records_center_count[assigned_centers[i]]++;
    }
    for (i = 0; i < CENTERS; i++)
    {
      for (j = 0; j < i; j++)
      {
        records_center_mempos[i] += records_center_count[j];
        records_center_mempos_tmp[i] += records_center_count[j];
      }
    }
    for (i = 0; i < NUMBER_OF_RECORDS; i++)
    {
      memcpy(&records_center[records_center_mempos_tmp[assigned_centers[i]]* DIMENSIONS],
             &records[i * DIMENSIONS],
             DIMENSIONS * sizeof(float));
      records_center_mempos_tmp[assigned_centers[i]]++;
    }

    for (i = 0; i < CENTERS; i++)
    {

      /**
       *
       * Phases 3, 4, 5 and 6 inside kmeans_ad.
       *
       */

      // Kmeans with anderson_darling test. Parallelized.

      kmeans_ad(DIMENSIONS, records_center_count[i],
        &records_center[records_center_mempos[i] * DIMENSIONS],
        &centers[i * DIMENSIONS], &new_centers[i * DIMENSIONS],
        &splitlist[i], adcv, CSS_NUM_SPUS, block_records);

    }
    // Check for new centers, merge them with the old ones or, in case of no splits, stops the algorithm.

    for (i = 0; i < CENTERS; i++)
    {

      if (splitlist[i])
      {

        copy_float(&centers[CENTERS * DIMENSIONS + NEW_CENTERS
          * DIMENSIONS], &new_centers[i * DIMENSIONS], DIMENSIONS);
        //				memcpy(&centers[CENTERS*DIMENSIONS+NEW_CENTERS*DIMENSIONS], &new_centers[i*DIMENSIONS], DIMENSIONS * sizeof(float));
        NEW_CENTERS++;
        split++;
      }
      else
      {
        nosplit++;
      }
    }

    printf("\n######### LOOP REPORT #########\n\n");

    printf("\n SPLIT: %d  NOSPLIT: %d    NEW TOTAL: %d\n\n", split,
      nosplit, (CENTERS + NEW_CENTERS));

    printf("\n######### END LOOP REPORT #########\n\n");

    CENTERS += NEW_CENTERS;

    if (!split)
      break;

  }
  maintime_int(1);
  printf("\n######### FINAL REPORT #########\n\n");

  for (i = 0; i < CENTERS; i++)
  {
    pVector( (char*) "CNTf", i, &centers[i * DIMENSIONS], DIMENSIONS);
    printf("\n");
  }

  printf("\n######### END FINAL REPORT #########\n\n");

  /* Number of resulting clusters must be defined for the partition */

  vector<cluster_id_t>& ClusterAssignmentVector = DataPartition.GetAssignmentVector();
  set<cluster_id_t>&    DifferentIDs            = DataPartition.GetIDs();

  for(i = 0; i < NUMBER_OF_RECORDS; i++)
  {
                                                  //[i] = UNCLASSIFIED;
    ClusterAssignmentVector.push_back(assigned_centers[i]); // Cluster 0 is always NOISE
    DifferentIDs.insert((cluster_id_t) assigned_centers[i]);
  }

  /* Add one more cluster, to avoid the non-existent NOISE cluster */
  DataPartition.NumberOfClusters (DifferentIDs.size());
  DataPartition.HasNoise(false);

  free(centers);
  free(records);
  free(assigned_centers);
  free(new_centers);
  free(splitlist);
  free(records_center);
  free(records_center_count);
  free(records_center_mempos);
  free(records_center_mempos_tmp);

  return true;
}


string GMEANS::GetClusteringAlgorithmName(void) const
{
  ostringstream Result;
  Result << "G-Means ( CriticalValue = " << CriticalValue;
  Result << ", MaxClusters = " << MaxClusters << ")";

  return Result.str();
}


string GMEANS::GetClusteringAlgorithmNameFile(void) const
{
  ostringstream Result;
  Result << "GMEANS_CriticalValue_" << CriticalValue;
  Result << "_MaxClusters_" << MaxClusters;

  return Result.str();
}


bool GMEANS::ComputeParamsApproximation(const vector<const Point*>& Data,
INT32 ParametersCount, ...)
{
  return true;
}
