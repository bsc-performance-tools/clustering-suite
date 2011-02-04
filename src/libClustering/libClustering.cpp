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


#include <libClusteringImplementation.hpp>
#include "libClustering.hpp"

#include <stdio.h>

/**
 * Empty constructor
 */
libClustering::libClustering(void)
{
  Implementation  = new libClusteringImplementation();
  Error = Warning = false;
}

/**
 * Initialization of the clustering algorithm
 *
 * \param AlgorithmName Name of the clustering algorithm to be used
 * \param AlgorithmParameters Map with the parameters needed by the algorithm
 *
 * \return True if the library has been correctly initialized, false otherwise
 */
bool libClustering::InitClustering(string              AlgorithmName,
                                   map<string, string> AlgorithmParameters)
{
  if (!Implementation->InitClustering(AlgorithmName,
                                      AlgorithmParameters))
  {
    ErrorMessage = Implementation->GetLastError();
    return false;
  }

  return true;
}

/**
 * Actual execution of the clustering algorithm
 *
 * \param Data Input vector containing the geometrical points to cluster. See class Point
 * \param Partition Output vector containing the assigned cluster ids for each point in the input vector
 *
 * \return True if the clustering algorithm was correctly applied, false otherwise
 */
bool libClustering::ExecuteClustering(const vector<const Point*>& Data,
                                      Partition&                  DataPartition)
{
  if (!Implementation->ExecuteClustering(Data, DataPartition))
  {
    ErrorMessage = Implementation->GetLastError();
    return false;
  }
  
  return true;
}

/**
 * Return true if the clustering algorithm to be used is based on MPI
 * 
 * \return True if the clustering algorithm is based on MPI
 */
bool libClustering::UsingADistributedAlgorithm(void)
{
  return Implementation->UsingADistributedAlgorithm();
}

/**
 * Returns the name of the clustering algorithm to be used
 * 
 * \return String with the name and parameters of the clustering algorithm  used
 */ 
string libClustering::GetClusteringAlgorithmName(void)
{
  return Implementation->GetClusteringAlgorithmName();
}

/**
 * Returns the string containing the last error message
 * \return Last error message
 */
string libClustering::GetErrorMessage(void)
{
  return ErrorMessage;
}

/**
 * Returns the string containing the last warning message
 * \return Last warning message
 */
string libClustering::GetWarningMessage(void)
{
  return WarningMessage;
}