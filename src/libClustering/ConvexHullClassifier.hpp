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

#ifndef _CONVEX_HULL_CLASSIFIER_HPP_
#define _CONVEX_HULL_CLASSIFIER_HPP_

#include <Error.hpp>
using cepba_tools::Error;
#include <SystemMessages.hpp>
using cepba_tools::system_messages;

#include "Classifier.hpp"
#include "ConvexHullModel.hpp"

#include <iostream>

#include <sstream>
using std::ostringstream;

/* Forward declarations */

class ConvexHullClassifier: public Classifier
{
  protected:

    double                   Eps;
    int                      MinPoints;
    vector<ConvexHullModel>& HullModels;

  public:

    ConvexHullClassifier(vector<ConvexHullModel>& _HullModels,
                         double                   _Eps,
                         int                      _MinPoints):
    HullModels(_HullModels),
    Eps(_Eps),
    MinPoints(_MinPoints)
    {
      ostringstream Message;
      Message << "Classifier - Total Models = " << _HullModels.size();
      Message << " Epsilon = " << _Eps;
      Message << " MinPoints = " << _MinPoints << std::endl;
      system_messages::information(Message.str());
    };

    bool Classify(vector<const Point*>& Data,
                  Partition&            DataPartition);

    template <typename T>
    bool Classify(T begin, T end, size_t size, Partition& DataPartition);

    bool Classify(const Point* QueryPoint, cluster_id_t& ID);

  private:

};

template <typename T>
bool ConvexHullClassifier::Classify(T          begin,
                                    T          end,
                                    size_t     size,
                                    Partition& DataPartition)
{
  T PointsIt;

  cluster_id_t          CurrentClusterId;
  vector<cluster_id_t>& AssignmentVector = DataPartition.GetAssignmentVector();
  set<cluster_id_t>&    DifferentIDs     = DataPartition.GetIDs();

  ostringstream Message;

  for (PointsIt = begin; PointsIt != end; ++PointsIt)
  {
    /* DEBUG
    Message.str("");
    Message << "Point[" << i << "]";
    system_messages::information(Message.str()); */

    Classify((Point*) (*PointsIt), CurrentClusterId);
    AssignmentVector.push_back(CurrentClusterId);

    DifferentIDs.insert(CurrentClusterId);

    /* DEBUG
    Message.str("");
    Message << " ID = " << CurrentClusterId << endl;
    system_messages::information(Message.str()); */
  }

  // DataPartition.NumberOfClusters(HullModels.size()+1);
  // DataPartition.HasNoise(true);

  /* DEBUG
  Message.str("");
  Message << "Classification Partition Size = " << DataPartition.NumberOfClusters() << endl;
  system_messages::information(Message.str().c_str());
  */

  return true;
}

#endif /* _NEARESTNEIGHBOURCLASSIFIER_HPP_ */
