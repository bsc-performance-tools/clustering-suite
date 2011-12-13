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

#ifndef _DBSCAN_HPP_
#define _DBSCAN_HPP_

#include "ClusteringAlgorithm.hpp"
#include "clustering_types.h"
//#include "KDTreeClassifier.hpp"

/* Forward declarations */
#include "Classifier.hpp"
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

    ANNpointArray        ANNDataPoints;
    ANNkd_tree*          SpatialIndex;

    size_t               NoisePoints;

    vector<cluster_id_t> IDs;
    set<cluster_id_t>    IDsUsed;

  public:
    static const string NAME;

    static const string EPSILON_STRING;
    static const string MIN_POINTS_STRING;

    DBSCAN(map<string, string> ClusteringParameters);

    ~DBSCAN(void) {};

    double GetEpsilon(void) const     { return Eps; };
    void   SetEpsilon(double Epsilon) { Eps = Epsilon; };

    INT32  GetMinPoints(void) const      { return MinPoints; };
    void   SetMinPoints(INT32 MinPoints) { this->MinPoints = MinPoints; };

    bool Run(const vector<const Point*>& Data,
             Partition&                  DataPartition,
             bool                        SimpleRun);

    string GetClusteringAlgorithmName(void) const;
    string GetClusteringAlgorithmNameFile(void) const;

    bool HasNoise(void) { return true; };

    static const string PARAMETER_K_BEGIN;
    static const string PARAMETER_K_END;

    bool ParametersApproximation(const vector<const Point*>& Data,
                                 map<string, string>&        Parameters,
                                 string                      OutputFileNamePrefix);

    bool ComputeNeighbourhood(const vector<const Point*>& Data,
                              size_t                      K,
                              vector<double>&             Distances);

    /* Classifier Methods */
    bool GetClassifier(Classifier& NewClassifier);

    bool Classify(const vector<const Point*> &Data,
                  Partition                  &DataPartition);

    bool Classify(const Point* Point, cluster_id_t& ID);

  private:

    bool BuildKDTree(const vector<const Point*>& Data);

    bool ExpandCluster(const vector<const Point*>& Data,
                       point_idx                   CurrentPoint,
                       vector<cluster_id_t>&       Partition,
                       cluster_id_t                CurrentClusterId);


    void EpsilonRangeQuery(const Point*     QueryPoint,
                           list<point_idx>& SeedList);

    ANNpoint ToANNPoint(const Point* const InputPoint);

    /* Parameters approximation methods */
    bool ComputeKNeighbourhoods(const vector<const Point*>& Data,
                                INT32                       k_begin,
                                INT32                       k_end,
                                string                      OutputFileNamePrefix);

    /*
    bool
    ComputeKNeighbourDistances(const vector<const Point*>& Data,
                               INT32                       k,
                               vector<double>&             Distances);
    */

    void ComputeNeighboursDistance(const  Point*            QueryPoint,
                                   size_t                   k_begin,
                                   size_t                   k_end,
                                   vector<vector<double> >& ResultingDistances);

};

#endif /* _DBSCAN_HPP_ */

