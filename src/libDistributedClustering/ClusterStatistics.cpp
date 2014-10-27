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

#include "ClusterStatistics.hpp"
#include <iostream>
#include <sstream>
#include <limits>


/******************************************************************************
 * CLASS 'MetricStats'
 *****************************************************************************/

/**
 * Aggregates the information of an external MetricStats object to the current
 * one in case their ID is the same
 *
 * \param Other The external MetricStats object we want to aggregate to the
 *              current one
 *
 * \return A reference to the current object with the required aggragetation
 */
const MetricStats& MetricStats::Aggregate(const MetricStats& Other)
{
  if (_MetricID.compare(Other.MetricID()) == 0)
  {
    _Aggregate   += Other.Aggregate();
    _Individuals += Other.Individuals();
    _Mean         = _Aggregate/_Individuals;
    _StdDev_2    += Other.StdDev_2();
  }

  return *this;
}

/**
 * Operator to serialize the current class to a stream
 *
 * \param Other The external ClusterStatistics object we want to aggregate to
 *              the current one
 * \return A reference to the current object with the required aggragetation
 */
std::ostream & operator<<(std::ostream &os, const MetricStats& MS)
{
  os << MS.MetricID() << ClusterStatistics::DELIMITER;
  os << MS.Aggregate() << ClusterStatistics::DELIMITER;
  os << MS.Individuals() << ClusterStatistics::DELIMITER;
  os << MS.Mean() << ClusterStatistics::DELIMITER;
  os << MS.StdDev_2();

  return os;
}

/******************************************************************************
 * CLASS 'ClusterStatistics'
 *****************************************************************************/

/**
 * Deserialization from a string constructor
 */
ClusterStatistics::ClusterStatistics(string ClusterStatisticsStr)
{
  std::istringstream CSSStr(ClusterStatisticsStr);
  std::stringstream Conversion;
  string _DurationStdDev_2Str;

  unsigned int ExtrapolationMetricsCount;
  char delim;

  string Field;

  getline( CSSStr, Field, ',' );
  std::stringstream IDConv(Field);
  IDConv >> _ID;

  getline( CSSStr, Field, ',' );
  std::stringstream IndividualsConv(Field);
  IndividualsConv >> _Individuals;

  getline( CSSStr, Field, ',' );
  std::stringstream TotalDurationConv(Field);
  TotalDurationConv >> _TotalDuration;


  getline( CSSStr, Field, ',' );
  std::stringstream DurationMeanConv(Field);
  DurationMeanConv >> _DurationMean;

  getline( CSSStr, Field, ',' );
  // std::istringstream Conversion.str(Field);
  if (Field.compare("-nan") == 0 ||
      Field.compare("nan") == 0)
  {
    _DurationStdDev_2 = std::numeric_limits<double>::quiet_NaN();
  }
  else
  {
    std::istringstream DurationStdDev_2Conv(Field);
    DurationStdDev_2Conv >> _DurationStdDev_2;
  }

  getline( CSSStr, Field, ',' );
  std::istringstream ExtrapCountConv(Field);
  ExtrapCountConv >> ExtrapolationMetricsCount;

  /*
  Conversion >> _DurationMean;
  CSSStr >> _ID                  >> delim;
  CSSStr >> _Individuals         >> delim;
  CSSStr >> _TotalDuration       >> delim;
  CSSStr >> _DurationMean        >> delim;
  CSSStr >> _DurationStdDev_2Str >> delim;

  std::cout << "_DurationStdDev_2Str: " << _DurationStdDev_2Str << std::endl;

  if (_DurationStdDev_2Str.compare("-nan") == 0 ||
      _DurationStdDev_2Str.compare("nan") == 0)
  {
    _DurationStdDev_2 = std::numeric_limits<double>::quiet_NaN();
  }
  else
  {
    std::istringstream doubleConv(_DurationStdDev_2Str);
    doubleConv >> _DurationStdDev_2;
  }

  CSSStr >> ExtrapolationMetricsCount;
  */

  if (ExtrapolationMetricsCount > 0)
  {

    for (size_t i = 0; i < ExtrapolationMetricsCount; i++)
    {
      string MetricID;
      double Aggregate;
      size_t Individuals;
      string MeanStr;
      double Mean;
      string StdDev_2Str;
      double StdDev_2;

      /* Empty */
      getline( CSSStr, MetricID, ',' );
      // getline( CSSStr, Field, ',' );
      // MetricID = Field;

      getline( CSSStr, Field, ',' );
      std::stringstream AggregateConv(Field);
      AggregateConv >> Aggregate;

      getline( CSSStr, Field, ',' );
      std::stringstream IndividualsConv(Field);
      IndividualsConv >> Individuals;

      getline( CSSStr, Field, ',' );
      if (Field.compare("-nan") == 0 ||
          Field.compare("nan") == 0)
      {
        Mean = std::numeric_limits<double>::quiet_NaN();
      }
      else
      {
        std::istringstream MeanConv(Field);
        MeanConv >> Mean;
      }

      getline( CSSStr, Field, ',' );
      if (Field.compare("-nan") == 0 ||
          Field.compare("nan") == 0)
      {
        StdDev_2 = std::numeric_limits<double>::quiet_NaN();
      }
      else
      {
        std::istringstream StdDev_2Conv(Field);
        StdDev_2Conv >> StdDev_2;
      }

      AddMetric(MetricID, Aggregate, Individuals, Mean, StdDev_2);
    }
  }
}

/**
/**
 * Aggregates the information of an external ClusterStatistics object to the
 * current one in case their ID is the same
 *
 * \param Other The external ClusterStatistics object we want to aggregate to
 *              the current one
 * \return A reference to the current object with the required aggragetation
 */
const ClusterStatistics& ClusterStatistics::Aggregate(const ClusterStatistics& Other)
{
  if (_ID == Other.ID())
  {
    _TotalDuration    += Other.TotalDuration();
    _DurationMean      = (_TotalDuration + Other.TotalDuration())/(_Individuals + Other.Individuals());
    _DurationStdDev_2 += Other.DurationStdDev_2();
    _Individuals      += Other.Individuals();

    for (vector<MetricStats>::size_type i = 0; i < _Metrics.size(); i++)
    {
      for (ClusterStatistics::size_type j = 0; j < Other.size(); j++)
      {
        _Metrics[i].Aggregate(Other[j]);
      }
    }
  }
  return *this;
}

std::ostream & operator<<(std::ostream &os, const ClusterStatistics& CS)
{
  os << CS.ID()               << ClusterStatistics::DELIMITER;

  os.precision(6);
  os << std::fixed;

  os << CS.Individuals()      << ClusterStatistics::DELIMITER;
  os << CS.TotalDuration()    << ClusterStatistics::DELIMITER;
  os << CS.DurationMean()     << ClusterStatistics::DELIMITER;
  os << CS.DurationStdDev_2() << ClusterStatistics::DELIMITER;

  os << CS.size();

  for (ClusterStatistics::size_type i = 0; i < CS.size(); i++)
  {
    os << ClusterStatistics::DELIMITER;
    os << CS[i];
  }

  return os;
}
