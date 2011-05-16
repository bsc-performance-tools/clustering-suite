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
    vector<ConvexHullModel>& HullModels;

  public:
    
    ConvexHullClassifier(vector<ConvexHullModel>& _HullModels,
                         double                   _Eps):
    HullModels(_HullModels),
    Eps(_Eps)
    {
      ostringstream Message;
      Message << "Classifier - Total Models = " << _HullModels.size();
      Message << " Epsilon = " << _Eps << std::endl;
      system_messages::information(Message.str());
    };
    
    bool Classify(vector<const Point*>& Data,
                  Partition&            DataPartition);
  
    bool Classify(const Point* QueryPoint, cluster_id_t& ID);
    
  private:
  
};

#endif /* _NEARESTNEIGHBOURCLASSIFIER_HPP_ */
