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

#include <SystemMessages.hpp>
using cepba_tools::system_messages;

#include "OPTICS.hpp"

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

#include <fstream>
using std::ofstream;

using std::make_pair;

const string OPTICS::NAME              = "DBSCAN";
const string OPTICS::EPSILON_STRING    = "epsilon";
const string OPTICS::MIN_POINTS_STRING = "min_points";

/*****************************************************************************
 * class OPTICS implementation                                               *
 ****************************************************************************/

OPTICS::OPTICS(map<string, string> ClusteringParameters)
{
  map<string, string>::iterator ParametersIterator;

  /* Epsilon */
  ParametersIterator = ClusteringParameters.find(OPTICS::EPSILON_STRING);
  if (ParametersIterator == ClusteringParameters.end())
  {
    string ErrorMessage;
    ErrorMessage = "parameter '" + OPTICS::EPSILON_STRING + "' not found in OPTICS definition";
    
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
      ErrorMessage = "incorrect value for OPTICS parameter '"+ OPTICS::EPSILON_STRING + "'";

      SetErrorMessage(ErrorMessage);
      SetError(true);
      return;
    }
  }

  /* MinPoints */
  ParametersIterator = ClusteringParameters.find(OPTICS::MIN_POINTS_STRING);
  if (ParametersIterator == ClusteringParameters.end())
  {
    string ErrorMessage;
    ErrorMessage = "parameter '" + OPTICS::MIN_POINTS_STRING + "' not found in OPTICS definition";
    
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
      ErrorMessage = "incorrect value for OPTICS parameter '"+ OPTICS::MIN_POINTS_STRING + "'";

      SetErrorMessage(ErrorMessage);
      SetError(true);
      return;
    }
  }
  
  return;
}

bool OPTICS::Run(const vector<const Point*>& Data,
                 Partition&                  DataPartition,
                 bool                        SimpleRun)
{
  cluster_id_t   ClusterId = MIN_CLUSTERID;
  Point*         CurrentPoint;
  INT32          CurrentPos, DimensionsCount;
  size_t         ResultingClusters = 0;

  INT64          DataSize, Index;

  /* TEMPORARY */
  SetError(true);
  SetErrorMessage("algorithm not implemented");
  return false;
  
  if (Data.size() == 0)
  {
    return true;
  }

  /* DEBUG */
  vector<cluster_id_t>& ClusterAssignmentVector = DataPartition.GetAssignmentVector();

  if (Data.size() != ClusterAssignmentVector.size())
  {
    ClusterAssignmentVector.clear();

    for (size_t i = 0; i < Data.size(); i++)
    {
      ClusterAssignmentVector.push_back(UNCLASSIFIED);
    }
  }
  
  /* Build KD-Tree */
  BuildKDTree(Data);

  system_messages::show_progress("Clustering points", 0, (int) Data.size());

  system_messages::show_progress_end("Clustering points", (int) Data.size());

  /* NOISE cluster has to be considered as a cluster, to mantain coherence across the namings */
  DataPartition.NumberOfClusters (ResultingClusters+1);
  DataPartition.HasNoise(true);

  return true;
}

string OPTICS::GetClusteringAlgorithmName(void) const
{
  ostringstream Result;
  Result << "OPTICS (Eps=" << Eps << ", MinPoints=" << MinPoints << ")";

  return Result.str();
}

string OPTICS::GetClusteringAlgorithmNameFile(void) const
{
  ostringstream Result;
  Result << "OPTICS_Eps_" << Eps << "_MinPoints_" << MinPoints;

  return Result.str();
}

bool OPTICS::BuildKDTree(const vector<const Point*>& Data)
{
  assert(Data.size() > 0);

  size_t Dimensions = Data[0]->size();
  
#ifdef DEBUG
/*  cout << "Current clustering has " << Dimensions << " dimensions" << endl; */
#endif
  
  system_messages::show_progress("Building data spatial index", 0, Data.size());
  
  ANNDataPoints = annAllocPts(Data.size(), Dimensions);

  for (size_t i = 0; i < Data.size(); i++)
  {
    system_messages::show_progress("Building data spatial index", i, Data.size());
    ANNDataPoints[i] = ToANNPoint(Data[i]);
  }

  system_messages::show_progress_end("Building data spatial index", Data.size());

  SpatialIndex = new ANNkd_tree(ANNDataPoints,
                                Data.size(),
                                Dimensions);
  
  return true;
}


ANNpoint OPTICS::ToANNPoint(const Point* InputPoint)
{
  ANNpoint Result = annAllocPt(InputPoint->size());

  for (size_t i = 0; i < InputPoint->size(); i++)
  {
    Result[i] = (*InputPoint)[i];
  }

  return Result;
}

