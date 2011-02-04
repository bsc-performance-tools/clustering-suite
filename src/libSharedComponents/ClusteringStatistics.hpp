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

#include <map>
using std::map;

/******************************************************************************
 * CLASS 'ExtrapolationDataContainer'
 *****************************************************************************/

/// 
/// Container to easily manipulate the statistics of the extrapolation metrics
///
class ExtrapolationDataContainer
{
  private:
    vector<size_t> Individuals;
    vector<double> Mean;
    vector<double> M2;
    vector<double> StdDev_2;

  public:
    ExtrapolationDataContainer() {};
    ExtrapolationDataContainer(size_t ExtrapolationMetrics);
    
    void Update(const map<size_t, double>& ExtrapolationData);
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

    vector<size_t>                     Individuals;
    vector<double>                     TotalDuration;
    vector<double>                     DurationMean;
    vector<double>                     DurationM2;
    vector<double>                     DurationStdDev_2;
    vector<ExtrapolationDataContainer> Extrapolation;
    
  public:

    ClusteringStatistics() {} ;
    
    ClusteringStatistics(size_t NumberOfClusters, size_t ExtrapolationMetrics);

    void InitStatistics(size_t NumberOfClusters, size_t ExtrapolationMetrics);

    bool ComputeStatistics(const vector<CPUBurst*>& Bursts, const vector<cluster_id_t>& IDs);
    
    void TranslatedIDs(vector<cluster_id_t>& NewIDs);

  private:

    bool NewIndividual(CPUBurst* Burst, cluster_id_t ID);

};

#endif // _CLUSTERSTATISTICS_HPP_
