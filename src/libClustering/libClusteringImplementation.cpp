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

#include "libClusteringImplementation.hpp"

#include "ClusteringAlgorithmsFactory.hpp"

#include <cerrno>
#include <cstring>

#include <fstream>
using std::ofstream;
using std::ios_base;

/**
 * Empty constructor
 */
libClusteringImplementation::libClusteringImplementation(void)
{
}

/**
 * Initialization of the clustering algorithm
 *
 * \param AlgorithmName Name of the clustering algorithm to be used
 * \param AlgorithmParameters Map with the parameters needed by the algorithm
 *
 * \return True if the library has been correctly initialized, false otherwise
 */
bool libClusteringImplementation::InitClustering(string              AlgorithmName,
                                                 map<string, string> AlgorithmParameters)
{
  ClusteringAlgorithmsFactory* AlgorithmsFactory;

  AlgorithmsFactory = ClusteringAlgorithmsFactory::GetInstance();

  Algorithm = AlgorithmsFactory->GetClusteringAlgorithm(AlgorithmName,
                                                        AlgorithmParameters);

  if (Algorithm == NULL)
  {
    SetErrorMessage(AlgorithmsFactory->GetLastError());
    return false;
  }

  if (Algorithm->GetError())
  {
    SetErrorMessage(Algorithm->GetLastError());
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
bool libClusteringImplementation::ExecuteClustering(const vector<const Point*>& Data,
                                                    Partition&                  DataPartition)
{
  if (Algorithm == NULL)
  {
    SetErrorMessage("clustering algorithm not initialized");
    return false;
  }

  if (!Algorithm->Run(Data, DataPartition))
  {
    SetErrorMessage(Algorithm->GetLastError());
    return false;
  }
  
  return true;
}

/**
 * Return true if the clustering algorithm to be used is based on MPI
 * 
 * \return True if the clustering algorithm is based on MPI
 */
bool libClusteringImplementation::UsingADistributedAlgorithm(void)
{
  if (Algorithm == NULL)
  {
    return false;
  }
  
  return Algorithm->IsDistributed();
}

/**
 * Returns the name of the clustering algorithm to be used
 * 
 * \return String with the name and parameters of the clustering algorithm  used
 */ 
string libClusteringImplementation::GetClusteringAlgorithmName(void)
{
  if (Algorithm == NULL)
  {
    return string("No algorithm initialized");
  }

  return Algorithm->GetClusteringAlgorithmName();
}