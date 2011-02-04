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
#include "clustering_types.h"

#include <SystemMessages.hpp>
using cepba_tools::system_messages;

#include <cassert>

#include "NearestNeighbourClassifier.hpp"

NearestNeighbourClassifier::NearestNeighbourClassifier(
  vector<Point*>        Data,
  vector<cluster_id_t>& Partition,
  double                Eps)
{
  assert(Data.size() > 0);

  ANNpointArray       ANNDataPoints;
  size_t Dimensions = Data[0]->size();

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
  
  return;
}

bool
NearestNeighbourClassifier::Classify(vector<Point*>& Data,
                                     bool            AddData)
{
  if (Data.size() == 0)
  {
    return true;
  }
  
  for (size_t i = 0; i < Data.size(); i++)
  {
    ANNpoint     QueryPoint     = ToANNPoint(Data[i]);
    ANNidxArray  ResultPoint    = new ANNidx[1];
    ANNdistArray ResultDistance = new ANNdist[1];
    
    /* Query for the nearest point to the current */
    SpatialIndex->annkSearch(QueryPoint, 1, ResultPoint, ResultDistance);
    
    if (ResultDistance[0] < pow(Eps, 2.0))
    {
      /* Current point must be classified in the same cluster */
    }
    else
    {
      /* Otherwise, current point is Noise! */
    }
  }
}

bool
NearestNeighbourClassifier::Classify(Point* QueryPoint, cluster_id_t& ID)
{
  ANNpoint     ANNQueryPoint;
  ANNidxArray  ResultPoint    = new ANNidx[1];
  ANNdistArray ResultDistance = new ANNdist[1];
  
  if (QueryPoint == NULL)
  {
    SetError(true);
    SetErrorMessage("Unknown point to classify");
    return false;
  }
  
  ANNQueryPoint = ToANNPoint(QueryPoint);
  
  /* Query for the nearest point to the current */
  SpatialIndex->annkSearch(ANNQueryPoint, 1, ResultPoint, ResultDistance);
  
  if (ResultDistance[0] < pow(Eps, 2.0))
  {
    /* Current point must be classified in the same cluster as Neighbour */
    ID = Partition[ResultPoint[0]];
  }
  else
  {
    /* Otherwise, current point is Noise! */
    ID = NOISE_CLUSTERID;
  }
  
  return true;
}

ANNpoint
NearestNeighbourClassifier::ToANNPoint(Point* InputPoint)
{
  ANNpoint Result = annAllocPt(InputPoint->size());

  for (size_t i = 0; i < InputPoint->size(); i++)
  {
    Result[i] = (*InputPoint)[i];
  }

  return Result;
}

