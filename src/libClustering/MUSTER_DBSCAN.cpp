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

#include "MUSTER_DBSCAN.hpp"
#include "Point.hpp"
#include "Partition.hpp"

#include <density.h>

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

const string MUSTER_DBSCAN::NAME              = "MUSTER_DBSCAN";
const string MUSTER_DBSCAN::EPSILON_STRING    = "epsilon";
const string MUSTER_DBSCAN::MIN_POINTS_STRING = "min_points";

/*****************************************************************************
 * class MUSTER_DBSCAN implementation                                        *
 ****************************************************************************/
MUSTER_DBSCAN::MUSTER_DBSCAN()
{
}

MUSTER_DBSCAN::MUSTER_DBSCAN(map<string, string> ClusteringParameters)
{
  map<string, string>::iterator ParametersIterator;

  /* Epsilon */
  ParametersIterator = ClusteringParameters.find(MUSTER_DBSCAN::EPSILON_STRING);
  if (ParametersIterator == ClusteringParameters.end())
  {
    string ErrorMessage;
    ErrorMessage = "parameter '" + MUSTER_DBSCAN::EPSILON_STRING + "' not found in MUSTER_DBSCAN definition";
    
    SetErrorMessage(ErrorMessage);
    SetError(true);
    return;
  }
  else
  {
    char* err;
    Eps = strtod(ParametersIterator->second.c_str(), &err);

    if (*err)
    {
      string ErrorMessage;
      ErrorMessage = "incorrect value for MUSTER_DBSCAN parameter '"+ MUSTER_DBSCAN::EPSILON_STRING + "'";

      SetErrorMessage(ErrorMessage);
      SetError(true);
      return;
    }
  }

  /* MinPoints */
  ParametersIterator = ClusteringParameters.find(MUSTER_DBSCAN::MIN_POINTS_STRING);
  if (ParametersIterator == ClusteringParameters.end())
  {
    string ErrorMessage;
    ErrorMessage = "parameter '" + MUSTER_DBSCAN::MIN_POINTS_STRING + "' not found in MUSTER_DBSCAN definition";
    
    SetErrorMessage(ErrorMessage);
    SetError(true);
    return;
  }
  else
  {
    char* err;
    MinPoints = strtol(ParametersIterator->second.c_str(), &err, 0);

    if (*err)
    {
      string ErrorMessage;
      ErrorMessage = "incorrect value for MUSTER_DBSCAN parameter '"+ MUSTER_DBSCAN::MIN_POINTS_STRING + "'";

      SetErrorMessage(ErrorMessage);
      SetError(true);
      return;
    }
  }

  return;
}


bool MUSTER_DBSCAN::Run(const vector<const Point*>& Data,
                        Partition&                  DataPartition,
                        bool                        SimpleRun)
{
  if (Data.empty())
  {
    SetError(true);
    SetErrorMessage("no data to apply clustering analysis");
    return false;
  }

  system_messages::information("Using MUSTER's DBSCAN we can not provide algorithm progress information\n");
  
  cluster::density muster_algorithm = cluster::density();
  muster_algorithm.dbscan(Data, PointEuclideanDistance(), Eps, MinPoints);

  cout << "Number of clusters found = " << muster_algorithm.num_clusters() << endl;

  /* Translate MUSTER partition to 'libClustering' partition */
  vector<cluster_id_t>& ClusterAssignmentVector = DataPartition.GetAssignmentVector();
  set<cluster_id_t>&    DifferentIDs            = DataPartition.GetIDs();

  ClusterAssignmentVector.clear();

  for (size_t i = 0; i < Data.size(); i++)
  {
    /* We had to substract 1 to each cluster id to control homogeneity with internal DBSCAN numbering */
    ClusterAssignmentVector.push_back(muster_algorithm.cluster_ids[i]);
    DifferentIDs.insert(muster_algorithm.cluster_ids[i]);
  }

  /* Not necessary any more
  DataPartition.NumberOfClusters(DifferentIDs.size()-1);
  DataPartition.HasNoise(true); */

  return true;
}


string MUSTER_DBSCAN::GetClusteringAlgorithmName(void) const
{
  ostringstream Result;
  Result << "MUSTER_DBSCAN (Eps=" << Eps << ", MinPoints=" << MinPoints << ")";
  return Result.str();
}


string MUSTER_DBSCAN::GetClusteringAlgorithmNameFile(void) const
{
  ostringstream Result;
  Result << "MUSTER_DBSCAN_Eps_" << Eps << "_MinPoints_" << MinPoints;
  return Result.str();
}


bool MUSTER_DBSCAN::ComputeParamsApproximation(const vector<const Point*>& Data,
INT32 ParametersCount, ...)
{
  return true;
}
