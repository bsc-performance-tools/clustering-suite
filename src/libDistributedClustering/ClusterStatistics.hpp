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

  $Id:: libDistributedClustering.hpp 76 2012-08-1#$:  Id
  $Rev:: 76                                       $:  Revision of last commit
  $Author:: jgonzale                              $:  Author of last commit
  $Date:: 2012-08-14 17:30:38 +0200 (Tue, 14 Aug #$:  Date of last commit

\* -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=- */

#ifndef _CLUSTER_STATISTICS_HPP
#define _CLUSTER_STATISTICS_HPP

#include "clustering_types.h"

#include <string>
using std::string;
#include <vector>
using std::vector;
#include <iostream>

class MetricStats
{
  private:

    string _MetricID;

    double _Aggregate;
    size_t _Individuals;
    double _Mean;
    double _StdDev_2;

  public:

    MetricStats(string MetricID, double Aggregate, size_t Individuals, double Mean, double StdDev_2)
      :_MetricID(MetricID), _Aggregate(Aggregate), _Individuals(Individuals), _Mean(Mean), _StdDev_2(StdDev_2)
    {};

    string MetricID(void)    const { return _MetricID; };
    double Aggregate(void)   const { return _Aggregate; };
    size_t Individuals(void) const { return _Individuals; };
    // double Mean(void)        const { return _Mean; };
    double Mean(void)        const { return _Aggregate/_Individuals; };
    double StdDev_2(void)    const { return _StdDev_2; };

    const MetricStats& Aggregate(const MetricStats& Other);

  private:

    friend std::ostream & operator<<(std::ostream &os, const MetricStats& MS);
};

class ClusterStatistics
{
  private:

    cluster_id_t        _ID;

    size_t              _Individuals;
    double              _TotalDuration;
    double              _DurationMean;
    double              _DurationStdDev_2;

    vector<MetricStats> _Metrics;

  public:

    static const char DELIMITER = ',';

    ClusterStatistics(cluster_id_t ID,
                      size_t       Individuals,
                      double       TotalDuration,
                      double       DurationMean,
                      double       DurationStdDev_2)
    :_ID(ID), _Individuals(Individuals),_TotalDuration(TotalDuration), _DurationMean(DurationMean), _DurationStdDev_2(DurationStdDev_2)
    {};

    ClusterStatistics(string ClusterStatisticsStr);

    void AddMetric (string MetricID, double Aggregate, size_t Individuals, double Mean, double StdDev_2)
    {
      _Metrics.push_back(MetricStats(MetricID, Aggregate, Individuals, Mean, StdDev_2));
    }

    cluster_id_t ID(void)                const {return _ID; };
    size_t       Individuals(void)       const { return _Individuals; };
    double       TotalDuration(void)     const { return _TotalDuration; };
    double       DurationMean(void)      const { return _DurationMean; };
    double       DurationStdDev_2 (void) const { return _DurationStdDev_2; };

    // vector<MetricStats>& Metrics          { return _Metrics; };

    typedef vector<MetricStats>::size_type size_type;

    size_type size(void) const { return _Metrics.size(); };

    MetricStats& operator[](unsigned int i)
    {
      return _Metrics[i];
    }

    const MetricStats& operator[] (unsigned int i) const
    {
      return _Metrics[i];
    }


    const ClusterStatistics& Aggregate(const ClusterStatistics& Other);

    friend std::ostream & operator<<(std::ostream &os, const ClusterStatistics& CS);
};

#endif
