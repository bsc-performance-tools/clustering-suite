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


#ifndef _CLUSTERING_STATISTICS_HPP_
#define _CLUSTERING_STATISTICS_HPP_

#include "trace_clustering_types.h"

#include "ClusteringStatistics.hpp"
#include "CPUBurst.hpp"

#include <iostream>
using std::ostream;

#include <sstream>
using std::ostringstream;

#include <map>
using std::map;

#include <set>
using std::set;

/******************************************************************************
 * CLASS 'MetricContainer'
 *****************************************************************************/

///
/// Container to easily manipulate the statistics of a given metric
///
class MetricContainer
{
  private:
    size_t Individuals;
    double Mean;
    double M2;
    double StdDev_2;

  public:
    MetricContainer();

    void Update(double NewValue);

    size_t GetIndividuals(void) { return Individuals; };
    double GetMean(void)        { return Mean; };
    double GetM2(void)          { return M2; };
    double GetStdDev_2(void)    { return StdDev_2; };
};

/******************************************************************************
 * CLASS 'StatisticsContainer'
 *****************************************************************************/
///
/// Container to easily manipulate the statistics of a given cluster
///
class StatisticsContainer
{
  private:
    cluster_id_t OriginalClusterID;

    size_t       Individuals;
    double       TotalDuration;
    double       DurationMean;
    double       DurationM2;
    double       DurationStdDev_2;

    vector<MetricContainer> ClusteringParameters;
    vector<MetricContainer> ExtrapolationMetrics;

  public:

    StatisticsContainer(void);

    StatisticsContainer(cluster_id_t OriginalClusterID,
                        size_t       ClusteringParametersCount,
                        size_t       ExtrapolationMetricsCount);

    void NewBurst (CPUBurst* Burst);

    cluster_id_t GetOriginalClusterID(void) const { return OriginalClusterID; };
    size_t       GetIndividuals(void)       const { return Individuals; };
    double       GetTotalDuration(void)     const { return TotalDuration; };
    double       GetDurationMean(void)      const { return DurationMean; };
    double       GetDurationM2(void)        const { return DurationM2; };
    double       GetDurationStdDev_2(void)  const { return DurationStdDev_2; };

    double       GetClusteringParameterMean(size_t i);
    double       GetExtrapolationMetricMean(size_t i);

    bool operator<(const StatisticsContainer& other) const;

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

    size_t                      NumberOfClusters;
    bool                        HasNoise;

    double                      TotalBurstsDuration;
    StatisticsContainer         NoiseStatistics;
    vector<StatisticsContainer> StatisticsPerCluster;

    map<cluster_id_t, size_t>   IDsPosition;

    vector<string>              ClusteringParametersNames;
    vector<bool>                ClusteringParametersPrecision;
    vector<string>              ExtrapolationMetricsNames;
    vector<bool>                ExtrapolationMetricsPrecision;

    map<cluster_id_t, cluster_id_t> TranslationMap;
    bool                            Translated;

  public:

    ClusteringStatistics() {} ;

    ClusteringStatistics(set<cluster_id_t>& IDs,
                         vector<string> ClusteringParametersNames     = vector<string>(0),
                         vector<bool>   ClusteringParametersPrecision = vector<bool>(0),
                         vector<string> ExtrapolationMetricsNames     = vector<string>(0),
                         vector<bool>   ExtrapolationMetricsPrecision = vector<bool>(0));

    void InitStatistics(set<cluster_id_t>& IDs,
                        vector<string> ClusteringParametersNames     = vector<string>(0),
                        vector<bool>   ClusteringParametersPrecision = vector<bool>(0),
                        vector<string> ExtrapolationMetricsNames     = vector<string>(0),
                        vector<bool>   ExtrapolationMetricsPrecision = vector<bool>(0));

    bool ComputeStatistics(const vector<CPUBurst*>& Bursts, const vector<cluster_id_t>& IDs);

    template <typename T>
    bool ComputeStatistics(T begin, T end, size_t size, const vector<cluster_id_t>& IDs);

    void TranslatedIDs(vector<cluster_id_t>& NewIDs);

    map<cluster_id_t, percentage_t> GetPercentageDurations(void);
    map<cluster_id_t, double>       GetDurationSums(void);
    map<cluster_id_t, size_t>       GetIndividuals(void);
    map<cluster_id_t, cluster_id_t> GetTranslationMap(void) { return TranslationMap; };


    bool Flush(ostream& str);

  private:

    bool NewIndividual(CPUBurst* Burst, cluster_id_t ID);

};

template <typename T>
bool ClusteringStatistics::ComputeStatistics(T                           begin,
                                             T                           end,
                                             size_t                      size,
                                             const vector<cluster_id_t>& IDs)
{
  /* DEBUG
  cout << "Total number of bursts = " << Bursts.size() << endl; */

  if (size != IDs.size())
  {
    ostringstream Messages;

    Messages << "number of bursts (" << size << ") ";
    Messages << "different from number of IDs (" << IDs.size() << ") ";
    Messages << "when computing statistics";

    SetError(true);
    SetErrorMessage(Messages.str());
    return false;
  }

  T Burst  = begin;
  size_t i = 0;

  while (Burst != end)
  {
    if (HasNoise && IDs[i] == NOISE_CLUSTERID)
    {
      NoiseStatistics.NewBurst((*Burst));
    }
    else
    {
      map<cluster_id_t, size_t>::iterator CurrentIDPosition;

      CurrentIDPosition = IDsPosition.find(IDs[i]);

      if (CurrentIDPosition == IDsPosition.end())
      {
        ostringstream ErrorMessage;
        ErrorMessage << "incorrect cluster ID (" << IDs[i] << ") when computing statistics";

        SetError(true);
        SetErrorMessage(ErrorMessage.str());
        return false;
      }
      else
      {
        /* DEBUG
        cout << "Updating Statistics for Cluster " << IDs[i];
        cout << " (" << CurrentIDPosition->second << ")" << endl; */
        StatisticsPerCluster[CurrentIDPosition->second].NewBurst((*Burst));
      }
    }

    TotalBurstsDuration += (*Burst)->GetDuration();
    ++Burst;
    ++i;
  }

  return true;
}

#endif // _CLUSTERSTATISTICS_HPP_
