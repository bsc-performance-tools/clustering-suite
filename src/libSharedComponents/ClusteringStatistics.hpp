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


#ifndef _CLUSTERING_STATISTICS_HPP_
#define _CLUSTERING_STATISTICS_HPP_

#include "trace_clustering_types.h"

#include "ClusteringStatistics.hpp"
#include "CPUBurst.hpp"

#include <iostream>
using std::ostream;

#include <map>
using std::map;

/******************************************************************************
 * CLASS 'MetricContainer'
 *****************************************************************************/

/// 
/// Container to easily manipulate the statistics of a giving metric for all
/// clusters
///
class MetricContainer
{
  private:
    string         Name;
    bool           HighPrecision;

    vector<size_t> Individuals;
    vector<double> Mean;
    vector<double> M2;
    vector<double> StdDev_2;

  public:
    MetricContainer() {};
    MetricContainer(string Name,
                    bool   HighPrecision,
                    size_t NumberOfClusters);

    void Update(cluster_id_t Cluster, double NewValue);

    string GetMetricName(void)           { return Name; };
    bool   IsHighPrecision(void)         { return HighPrecision; };
    
    vector<size_t>& GetIndividuals(void) { return Individuals; };
    vector<double>& GetMeans(void)       { return Mean; };
    vector<double>& GetM2s(void)         { return M2; };
    vector<double>& GetStdDevs_2(void)   { return StdDev_2; };

    bool Flush(ostream&              str,
               bool                  HasNoise,
               vector<cluster_id_t>& SortedClusters);
};

/******************************************************************************
 * CLASS 'ClusteringStatistics'
 *****************************************************************************/

/// 
/// Container to manipulate the statistics of every cluster found during the
/// cluster analysis, and also needed to sort the results with respect to the
/// total bursts duration
///
class ClusteringStatistics: public Error 
{
  private:

    size_t                  NumberOfClusters;
    bool                    HasNoise;
    
    vector<size_t>          Individuals;
    vector<double>          TotalDuration;
    vector<double>          DurationMean;
    vector<double>          DurationM2;
    vector<double>          DurationStdDev_2;

    vector<MetricContainer> ClusteringMetrics;
    vector<MetricContainer> ExtrapolationMetrics;

    map<cluster_id_t, cluster_id_t>    TranslationMap; // To re-order in terms of duration
    
  public:

    ClusteringStatistics() {} ;
    
    ClusteringStatistics(size_t         NumberOfClusters,
                         bool           HasNoise,
                         vector<string> ClusteringParametersNames,
                         vector<bool>   ClusteringParametersPrecision,
                         vector<string> ExtrapolationMetricsNames,
                         vector<bool>   ExtrapolationMetricsPrecision);

    void InitStatistics(size_t         NumberOfClusters,
                        bool           HasNoise,
                        vector<string> ClusteringParametersNames,
                        vector<bool>   ClusteringParametersPrecision,
                        vector<string> ExtrapolationMetricsNames,
                        vector<bool>   ExtrapolationMetricsPrecision);

    bool ComputeStatistics(const vector<CPUBurst*>& Bursts, const vector<cluster_id_t>& IDs);
    
    void TranslatedIDs(vector<cluster_id_t>& NewIDs);

    bool Flush(ostream& str);

  private:

    bool NewIndividual(CPUBurst* Burst, cluster_id_t ID);

};

#endif // _CLUSTERSTATISTICS_HPP_
