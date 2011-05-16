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

  $URL::                                          $:  File
  $Rev::                                          $:  Revision of last commit
  $Author::                                       $:  Author of last commit
  $Date::                                         $:  Date of last commit

\* -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=- */
#include "clustering_types.h"

#include <SystemMessages.hpp>
using cepba_tools::system_messages;

#include <sstream>
using std::ostringstream;

#include <cassert>

#include "ConvexHullClassifier.hpp"

bool ConvexHullClassifier::Classify(vector<const Point*>& Data,
                                    Partition&            DataPartition)
{
  cluster_id_t          CurrentClusterId;
  vector<cluster_id_t>& AssignmentVector = DataPartition.GetAssignmentVector();

  ostringstream Message;

  for (size_t i = 0; i < Data.size(); i++)
  {
    /* DEBUG 
    Message.str("");
    Message << "Point[" << i << "]";
    system_messages::information(Message.str()); */
      
    Classify(Data[i], CurrentClusterId);
    AssignmentVector.push_back(CurrentClusterId);

    /* DEBUG 
    Message.str("");
    Message << " ID = " << CurrentClusterId << endl;
    system_messages::information(Message.str()); */
  }

  DataPartition.NumberOfClusters(HullModels.size()+1);
  DataPartition.HasNoise(true);

  /* DEBUG
  Message.str("");
  Message << "Classification Partition Size = " << DataPartition.NumberOfClusters() << endl;
  system_messages::information(Message.str().c_str());
  */
  
  return true;
}

bool ConvexHullClassifier::Classify(const Point* QueryPoint, cluster_id_t& ID)
{
  ostringstream Message;
  ID = NOISE_CLUSTERID;

  /* To improve the search, first we just look using the inclusion */
  for (size_t i = 0; i < HullModels.size(); i++)
  {
    if (HullModels[i].IsInside(QueryPoint))
    {
      ID    = (cluster_id_t) i+1;
      return true;
    }
  }

  /* If point hasn't been classified, we try the proximity */
  for (size_t i = 0; i < HullModels.size(); i++)
  {
    if (HullModels[i].IsNear(QueryPoint, Eps))
    {
      ID    = (cluster_id_t) i+1;
      return true;
    }
  }
  
  /* DEBUG 
  Message << " ID = " << ID << endl;
  system_messages::information(Message.str().c_str());
  */
  
  return true;
}
