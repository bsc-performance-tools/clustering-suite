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

#ifndef _OPTICS_HPP_
#define _OPTICS_HPP_

#include "ClusteringAlgorithm.hpp"
#include "clustering_types.h"

/* Forward declarations */
class Classifier;
class Partition;

#include <ANN/ANN.h>

#include <sstream>
using std::ostringstream;

#include <iostream>
using std::ostream;

#include <vector>
using std::vector;
#include <list>
using std::list;

using std::pair;

class OPTICS: public ClusteringAlgorithm
{

  typedef size_t point_idx;
  
  private:
    double              Eps;
    INT32               MinPoints;

    /*
    vector<INT32>       ClusterIdTranslation;
    ClusterInformation* NoiseClusterInfo;
    ClusterInformation* ThresholdFilteredClusterInfo; */

    ANNpointArray       ANNDataPoints;
    ANNkd_tree*         SpatialIndex;

  public:
    static const string NAME;
    
    static const string EPSILON_STRING;
    static const string MIN_POINTS_STRING;

    /*
    DBSCAN(double _Eps, INT32 _MinPoints, double FilterThreshold = 0.0)
    :ClusteringAlgorithm(FilterThreshold),
     Eps(_Eps),
     MinPoints(_MinPoints)
    {};
    */
    OPTICS(map<string, string> ClusteringParameters);

    ~OPTICS(void) {};

    double GetEpsilon(void) const     { return Eps; };
    void   SetEpsilon(double Epsilon) { Eps = Epsilon; };

    INT32  GetMinPoints(void) const      { return MinPoints; };
    void   SetMinPoints(INT32 MinPoints) { this->MinPoints = MinPoints; };
    
    bool Run(const vector<const Point*>& Data,
             Partition&                  DataPartition,
             bool                        SimpleRun);

    /*
    Classifier*
    GetClassifier(void) { return new KDTreeClassifier(SpatialIndex, Eps); }; */
  
    string GetClusteringAlgorithmName(void) const;
    string GetClusteringAlgorithmNameFile(void) const;

    /*
    bool
    GetClustersInformation(vector<ClusterInformation*>& ClusterInfoVector);
    */

    /* bool GetDataPoints(vector<DataPoint*>& DataPoints); */

    bool HasNoise(void) { return true; };

  private:
    
    bool BuildKDTree(const vector<const Point*>& Data);

    ANNpoint ToANNPoint(const Point* const InputPoint);

};

#endif /* _OPTICS_HPP_ */

