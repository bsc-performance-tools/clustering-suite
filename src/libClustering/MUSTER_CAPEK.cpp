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

/* -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=- *

  $URL:: https://svn.bsc.es/repos/ptools/prv2dim/                          $:

  $Rev:: 478                        $:  Revision of last commit
  $Author:: jgonzale                $:  Author of last commit
  $Date:: 2010-10-28 13:58:59 +0200 $:  Date of last commit

\* -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=- */

#include "SystemMessages.hpp"
using cepba_tools::system_messages;

#include "MUSTER_CAPEK.hpp"
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

#include <sstream>
using std::ostringstream;

#include <set>
using std::set;

using std::make_pair;

using cluster::medoid_id;

const string MUSTER_CAPEK::NAME     = "MUSTER_CAPEK";
const string MUSTER_CAPEK::K_STRING = "k";

/*****************************************************************************
 * class CAPEK_Point implementation                                          *
 ****************************************************************************/

CAPEK_Point::CAPEK_Point(Point OriginalPoint):Point(OriginalPoint)
{
  
}

/// Returns the size of a packed point
int CAPEK_Point::packed_size(MPI_Comm comm) const
{
  int dimensions_size;
  int normalized_size;

  dimensions_size = mpi_packed_size(Dimensions.size(), MPI_DOUBLE, comm);
  normalized_size = mpi_packed_size(1, MPI_CHAR, comm);

  return (dimensions_size + normalized_size);
}

/// Packs a point into an MPI packed buffer
void CAPEK_Point::pack(void *buf, int bufsize, int *position, MPI_Comm comm) const
{
  double dimensions_buffer[Dimensions.size()];
  char   normalized_buffer = 0;
  
  for (size_t i = 0; i < Dimensions.size(); i++)
  {
    dimensions_buffer[i] = Dimensions[i];
  }

  if (Normalized)
  {
    normalized_buffer = 1;
  }
  
  MPI_Pack(const_cast<double*>(dimensions_buffer), Dimensions.size(), MPI_DOUBLE, buf, bufsize, position, comm);
  MPI_Pack(const_cast<char*>(&normalized_buffer), 1, MPI_CHAR, buf, bufsize, position, comm);
}

/// Unpacks a point from an MPI packed buffer
Point CAPEK_Point::unpack(void *buf, int bufsize, int *position, MPI_Comm comm)
{
  double dimensions_buffer[Point::PointDimensions];
  vector<double> dimensions_object;
  char    normalized_buffer;

  for (size_t i = 0; i < Point::PointDimensions; i++)
  {
    MPI_Unpack(buf, bufsize, position, &dimensions_buffer[i], 1, MPI_DOUBLE, comm);
  }

  for (size_t i = 0; i < Point::PointDimensions; i++)
  {
    dimensions_object.push_back(dimensions_buffer[i]);
  }

  Point NewPoint(dimensions_object);
  
  MPI_Unpack(buf, bufsize, position, &normalized_buffer, 1, MPI_CHAR, comm);

  if (normalized_buffer == 1)
  {
    NewPoint.SetNormalized (true);
  }
  else
  {
    NewPoint.SetNormalized (false);
  }

  return NewPoint;
}


/*****************************************************************************
 * class MUSTER_CAPEK implementation                                         *
 ****************************************************************************/
MUSTER_CAPEK::MUSTER_CAPEK()
{
}

MUSTER_CAPEK::MUSTER_CAPEK(map<string, string> ClusteringParameters)
{
  map<string, string>::iterator ParametersIterator;

  /* k */
  ParametersIterator = ClusteringParameters.find(MUSTER_CAPEK::K_STRING);
  if (ParametersIterator == ClusteringParameters.end())
  {
    string ErrorMessage;
    ErrorMessage = "parameter '" + MUSTER_CAPEK::K_STRING + "' not found in MUSTER_CAPEK definition";
    
    SetErrorMessage(ErrorMessage);
    SetError(true);
    return;
  }
  else
  {
    char* err;
    k = strtol(ParametersIterator->second.c_str(), &err, 0);

    if (*err)
    {
      string ErrorMessage;
      ErrorMessage = "incorrect value for MUSTER_CAPEK parameter '"+ MUSTER_CAPEK::K_STRING + "'";

      SetErrorMessage(ErrorMessage);
      SetError(true);
      return;
    }
  }

  return;
}


bool MUSTER_CAPEK::Run(const vector<const Point*>& Data,
                        Partition&                  DataPartition,
                        bool                        SimpleRun)
{
  vector<CAPEK_Point> LocalData;
  ostringstream Messages;
  
  cluster::par_kmedoids muster_algorithm = cluster::par_kmedoids();
  cluster::dissimilarity_matrix DissimilarityMatrix;
  
  if (Data.empty())
  {
    SetError(true);
    SetErrorMessage("no data to apply clustering analysis");
    return false;
  }

  system_messages::information("Using MUSTER's CAPEK we can not provide algorithm progress information\n");

  /* Use the dimensions of the first point */
  // dimensions = Data[0]->size();

  for (size_t i = 0; i < Data.size(); i++)
  {
    LocalData.push_back(CAPEK_Point(*Data[i]));
  }
  // DEBUG
  Messages << "Data Size = " << LocalData.size() << endl;
  system_messages::information(Messages.str().c_str());
  Messages.str("");

  cluster::build_dissimilarity_matrix(Data, PointEuclideanDistance(), DissimilarityMatrix);
  muster_algorithm.capek(LocalData, PointEuclideanDistance(), k);

  // cout << "Number of clusters found = " << muster_algorithm.num_clusters() << endl;

  /* Translate MUSTER partition to 'libClustering' partition */
  vector<cluster_id_t>& ClusterAssignmentVector = DataPartition.GetAssignmentVector();
  set<cluster_id_t>&    DifferentIDs            = DataPartition.GetIDs();

  ClusterAssignmentVector.clear();

  for (size_t i = 0; i < Data.size(); i++)
  {
    ClusterAssignmentVector.push_back(muster_algorithm.cluster_ids[i]);
    DifferentIDs.insert(muster_algorithm.cluster_ids[i]);
  }

  // DEBUG
  vector<size_t> ClusterSizes;

  muster_algorithm.get_sizes(ClusterSizes);

  for (size_t i = 0; i < ClusterSizes.size(); i++)
  {
    Messages << "Cluster " << i << " size = " << ClusterSizes[i] << endl;
    system_messages::information(Messages.str().c_str());
    Messages.str("");
  }
  
  Messages << "MEDOID IDS = [ ";
  for (size_t i = 0; i < muster_algorithm.medoid_ids.size(); i++)
  {
    Messages << muster_algorithm.medoid_ids[i] << " ";
  }
  Messages << "]" << endl;
  system_messages::information(Messages.str().c_str());
  
  Messages.str("");

  Messages << "DIFFERENT IDS = [ ";
  for (SetIterator  = DifferentIDs.begin();
       SetIterator != DifferentIDs.end();
       SetIterator++)
  {
    Messages << *SetIterator << " ";
  }
  Messages << "]" << endl;
  system_messages::information(Messages.str().c_str());

   /* Translate MUSTER partition to 'libClustering' partition */
  ProcessClusterAssignment (muster_algorithm, DataPartition, Data.size());

  /* NOISE cluster has to be considered as a cluster, to mantain coherence across the namings 
  DataPartition.NumberOfClusters (muster_algorithm.num_clusters()+1);
  DataPartition.HasNoise(false);
  */

  return true;
}


string MUSTER_CAPEK::GetClusteringAlgorithmName(void) const
{
  ostringstream Result;
  Result << "MUSTER_CAPEK (k =" << k << ")";
  return Result.str();
}


string MUSTER_CAPEK::GetClusteringAlgorithmNameFile(void) const
{
  ostringstream Result;
  Result << "MUSTER_CAPEK_k_" << k;
  return Result.str();
}


bool MUSTER_CAPEK::ComputeParamsApproximation(const vector<const Point*>& Data,
INT32 ParametersCount, ...)
{
  return true;
}

void MUSTER_CAPEK::ProcessClusterAssignment(cluster::par_kmedoids &muster_algorithm,
                                            Partition             &DataPartition,
                                            size_t                 DataSize)
{
  vector<cluster_id_t>& ClusterAssignmentVector = DataPartition.GetAssignmentVector();
  set<cluster_id_>&     DifferentIDs            = DataPartition.GetIDs();

  /*
  map<medoid_id, cluster_id_t>           ClusterTranslation;
  map<medoid_id, cluster_id_t>::iterator ClusterTranslationQuery;
  cluster_id_t                           CurrentClusterId = MIN_CLUSTERID;
  */
  
  ClusterAssignmentVector.clear();
  DifferentIDs.clear();

  for (size_t i = 0; i < DataSize; i++)
  {
    ClusterAssignmentVector = muster_algorithm.cluster_ids[i];
    DifferentIDs.insert(muster_algorithm.cluster_ids[i]);

  }

  /* Add one more cluster, to avoid the non-existent NOISE cluster */
  DataPartition.NumberOfClusters (DifferentIDs.size());
  DataPartition.HasNoise(false);

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

