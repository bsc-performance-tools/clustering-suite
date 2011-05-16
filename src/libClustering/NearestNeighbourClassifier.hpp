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

#ifndef _NEARESTNEIGHBOURCLASSIFIER_HPP_
#define _NEARESTNEIGHBOURCLASSIFIER_HPP_

#include <Error.hpp>
using cepba_tools::Error;

#include "Classifier.hpp"

#include <ANN/ANN.h>

/* Forward declarations */

class NearestNeighbourClassifier: public Classifier
{
  protected:

    double               Eps;
    ANNkd_tree*          SpatialIndex;
    vector<cluster_id_t> Partition;

  public:
    
    NearestNeighbourClassifier(ANNkd_tree*           _SpatialIndex,
                               vector<cluster_id_t>& _Partition,
                               double                _Eps):
    SpatialIndex(_SpatialIndex),
    Partition(_Partition),
    Eps(_Eps)
    {}

    NearestNeighbourClassifier(vector<Point*>        Data,
                               vector<cluster_id_t>& Partition,
                               double                Eps);
    
    bool Classify(vector<Point*>& Data,
                  bool            AddData = false);
  
    bool Classify(Point* QueryPoint, cluster_id_t& ID);
  private:
    
    ANNpoint ToANNPoint(Point* InputPoint);
  
};

#endif /* _NEARESTNEIGHBOURCLASSIFIER_HPP_ */
