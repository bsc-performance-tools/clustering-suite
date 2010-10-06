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

#include <SystemMessages.hpp>

#include "GMEANS.hpp"

#include "Point.hpp"
#include "Partition.hpp"

#include <cstring>
#include <cmath>
#include <cassert>
#include <cerrno>

#include <cstdarg>

#include <algorithm>
using std::sort;

#include <iostream>
using std::cout;
using std::endl;

using std::make_pair;

const string GMEANS::INITIAL_POINTS = "initial_points";
const string GMEANS::CRITICAL_VALUE = "critical_value";
const string GMEANS::MAX_CLUSTERS = "max_clusters";

/*****************************************************************************
 * class GMEANS implementation                                               *
 ****************************************************************************/

GMEANS::GMEANS(map<string, string> ClusteringParameters)
{
  map<string, string>::iterator ParametersIterator;

  /* Initial Points */
  ParametersIterator = ClusteringParameters.find(GMEANS::INITIAL_POINTS);
  if (ParametersIterator == ClusteringParameters.end())
  {
    string ErrorMessage;
    ErrorMessage = "parameter '" + GMEANS::INITIAL_POINTS + "' not found in G-Means definition";
    
    SetErrorMessage(ErrorMessage);
    SetError(true);
    return;
  }
  else
  {
    char* err;
    InitialPoints = strtol(ParametersIterator->second.c_str(), &err, 0);

    if (*err)
    {
      string ErrorMessage;
      ErrorMessage = "incorrect value for G-Means parameter '"+ GMEANS::INITIAL_POINTS + "'";

      SetErrorMessage(ErrorMessage);
      SetError(true);
      return;
    }
  }

  /* Critical Values */
  ParametersIterator = ClusteringParameters.find(GMEANS::CRITICAL_VALUE);
  if (ParametersIterator == ClusteringParameters.end())
  {
    string ErrorMessage;
    ErrorMessage = "parameter '" + GMEANS::CRITICAL_VALUE + "' not found in G-Means definition";
    
    SetErrorMessage(ErrorMessage);
    SetError(true);
    return;
  }
  else
  {
    char* err;
    CriticalValue = strtod(ParametersIterator->second.c_str(), &err);

    if (*err)
    {
      string ErrorMessage;
      ErrorMessage = "incorrect value for G-Means parameter '"+ GMEANS::CRITICAL_VALUE + "'";

      SetErrorMessage(ErrorMessage);
      SetError(true);
      return;
    }
  }

  /* Max Clusters */
  ParametersIterator = ClusteringParameters.find(GMEANS::MAX_CLUSTERS);
  if (ParametersIterator == ClusteringParameters.end())
  {
    MaxClusters = -1;
  }
  else
  {
    char* err;
    MaxClusters = strtol(ParametersIterator->second.c_str(), &err, 0);

    if (*err)
    {
      string ErrorMessage;
      ErrorMessage = "incorrect value for G-Means parameter '"+ GMEANS::MAX_CLUSTERS + "'";

      SetErrorMessage(ErrorMessage);
      SetError(true);
      return;
    }
  }
  
  return;
}

bool GMEANS::Run(const vector<const Point*>& Data,
                 Partition&                  DataPartition,
                 bool                        SimpleRun)
{
  
  /* Number of resulting clusters must be defined for the partition
  DataPartition.SetNumberOfClusters (ResultingClusters); */

  return true;
}

string GMEANS::GetClusteringAlgorithmName(void) const
{
  ostringstream Result;
  Result << "G-Means (InitialPoints =" << InitialPoints << ", CriticalValue=" << CriticalValue << ")";

  return Result.str();
}

string GMEANS::GetClusteringAlgorithmNameFile(void) const
{
  ostringstream Result;
  Result << "GMEANS_InitPoints_" << InitialPoints << "_CriticalValue_" << CriticalValue;

  return Result.str();
}

bool GMEANS::ComputeParamsApproximation(const vector<const Point*>& Data,
                                        INT32                       ParametersCount, ...)
{
  return true;
}
