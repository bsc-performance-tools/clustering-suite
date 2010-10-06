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

  $URL:: https://svn.bsc.#$:  File
  $Rev:: 20               $:  Revision of last commit
  $Author:: jgonzale      $:  Author of last commit
  $Date:: 2010-03-09 17:1#$:  Date of last commit

\* -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=- */

#ifndef _DBSCAN_HPP_
#define _DBSCAN_HPP_

#include "ClusteringAlgorithm.hpp"
#include "clustering_types.h"
//#include "KDTreeClassifier.hpp"

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

class DBSCAN: public ClusteringAlgorithm
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

    static const string EPSILON_STRING;
    static const string MIN_POINTS_STRING;

    /*
    DBSCAN(double _Eps, INT32 _MinPoints, double FilterThreshold = 0.0)
    :ClusteringAlgorithm(FilterThreshold),
     Eps(_Eps),
     MinPoints(_MinPoints)
    {};
    */
    DBSCAN(map<string, string> ClusteringParameters);

    ~DBSCAN(void) {};

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


    bool
    ComputeParamsApproximation(const vector<const Point*>& Data,
                               INT32                       ParametersCount, ...);

    bool
    ComputeParamsApproximation(const vector<const Point*>& Data,
                               INT32                       MinPoints,
                               vector<double>&             Distances);

  private:
    
    bool BuildKDTree(const vector<const Point*>& Data);

    bool
    ExpandCluster(const vector<const Point*>& Data,
                  point_idx                   CurrentPoint,
                  vector<cluster_id_t>&       Partition,
                  cluster_id_t           CurrentClusterId);


    void EpsilonRangeQuery(const Point*     QueryPoint,
                           list<point_idx>& SeedList);

    ANNpoint ToANNPoint(const Point* const InputPoint);

    /* Parameters approximation methods */
    bool
    ComputeKNeighbourhoods(const vector<const Point*>& Data,
                           vector<INT32>              *KNeighbours,
                           string                      KNeighbourFileBaseName);

    bool
    ComputeKNeighbourDistances(const vector<const Point*>& Data,
                               INT32                       k,
                               vector<double>&             Distances);

    double
    ComputeKNeighbourDistance(const Point* QueryPoint, INT32 k);

};

#endif /* _DBSCAN_HPP_ */

