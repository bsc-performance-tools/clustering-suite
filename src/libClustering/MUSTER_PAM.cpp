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

  $Id::                                           $:  Id
  $Rev::                                          $:  Revision of last commit
  $Author::                                       $:  Author of last commit
  $Date::                                         $:  Date of last commit

\* -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=- */

#include "SystemMessages.hpp"
using cepba_tools::system_messages;

#include "MUSTER_PAM.hpp"
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

using cluster::medoid_id;

const string MUSTER_PAM::NAME                = "MUSTER_PAM";
const string MUSTER_PAM::MAX_CLUSTERS_STRING = "max_clusters";


/*****************************************************************************
 * class MUSTER_PAM implementation                                        *
 ****************************************************************************/
MUSTER_PAM::MUSTER_PAM()
{
}

MUSTER_PAM::MUSTER_PAM(map<string, string> ClusteringParameters)
{
  map<string, string>::iterator ParametersIterator;

  /* maximum clusters */
  ParametersIterator = ClusteringParameters.find(MUSTER_PAM::MAX_CLUSTERS_STRING);
  if (ParametersIterator == ClusteringParameters.end())
  {
    string ErrorMessage;
    ErrorMessage = "parameter '" + MUSTER_PAM::MAX_CLUSTERS_STRING + "' not found in MUSTER_DBSCAN definition";
    
    SetErrorMessage(ErrorMessage);
    SetError(true);
    return;
  }
  else
  {
    char* err;
    MaxClusters = strtol(ParametersIterator->second.c_str(), &err, 0);

    if (*err)
    {
      string ErrorMessage;
      ErrorMessage = "incorrect value for MUSTER_PAM parameter '"+ MUSTER_PAM::MAX_CLUSTERS_STRING + "'";

      SetErrorMessage(ErrorMessage);
      SetError(true);
      return;
    }
  }

  return;
}


bool MUSTER_PAM::Run(const vector<const Point*>& Data,
                     Partition&                  DataPartition,
                     bool                        SimpleRun)
{
  cluster::kmedoids             muster_algorithm = cluster::kmedoids();
  cluster::dissimilarity_matrix DissimilarityMatrix;
  size_t                        dimensions;

  if (Data.empty())
  {
    SetError(true);
    SetErrorMessage("no data to apply clustering analysis");
    return false;
  }

  system_messages::information("Using MUSTER's PAM we can not provide algorithm progress information\n");
  
  /* Use the dimensions of the first point */
  dimensions = Data[0]->size();

  cluster::build_dissimilarity_matrix(Data, PointEuclideanDistance(), DissimilarityMatrix);

  muster_algorithm.xpam(DissimilarityMatrix, (size_t) MaxClusters, dimensions);

  /* Translate MUSTER partition to 'libClustering' partition */
  ProcessClusterAssignment (muster_algorithm, DataPartition, Data.size());

  return true;
}


string MUSTER_PAM::GetClusteringAlgorithmName(void) const
{
  ostringstream Result;
  Result << "MUSTER_XPAM (MaxClusters=" << MaxClusters << ")";
  return Result.str();
}


string MUSTER_PAM::GetClusteringAlgorithmNameFile(void) const
{
  ostringstream Result;
  Result << "MUSTER_XPAM_MaxClusters_" << MaxClusters;
  return Result.str();
}


bool MUSTER_PAM::ComputeParamsApproximation(const vector<const Point*>& Data,
INT32 ParametersCount, ...)
{
  return true;
}

void MUSTER_PAM::ProcessClusterAssignment(cluster::kmedoids &muster_algorithm,
                                          Partition         &DataPartition,
                                          size_t             DataSize)
{
  vector<cluster_id_t>& ClusterAssignmentVector = DataPartition.GetAssignmentVector();
  set<cluster_id_t>&    DifferentIDs            = DataPartition.GetIDs();

  /*
  map<medoid_id, cluster_id_t>           ClusterTranslation;
  map<medoid_id, cluster_id_t>::iterator ClusterTranslationQuery;
  cluster_id_t                           CurrentClusterId = MIN_CLUSTERID;
  */
  
  ClusterAssignmentVector.clear();
  DifferentIDs.clear();

  for (size_t i = 0; i < DataSize; i++)
  {
    ClusterAssignmentVector.push_back(muster_algorithm.cluster_ids[i]);
    DifferentIDs.insert(muster_algorithm.cluster_ids[i]);
  }

  /* Add one more cluster, to avoid the non-existent NOISE cluster */
  /* Not needed any more
  DataPartition.NumberOfClusters (DifferentIDs.size());
  DataPartition.HasNoise(false); */

  /* DEBUG 
  for (ClusterTranslationQuery  = ClusterTranslation.begin();
       ClusterTranslationQuery != ClusterTranslation.end();
       ++ClusterTranslationQuery)
  {
    cout << "medoid_id = " << ClusterTranslationQuery->first;
    cout << " cluster_id = " << ClusterTranslationQuery->second;
    cout << endl;
  }
  */
}
