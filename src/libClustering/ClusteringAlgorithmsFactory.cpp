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

#include "ClusteringAlgorithmsFactory.hpp"
#include "DBSCAN.hpp"

ClusteringAlgorithmsFactory*
ClusteringAlgorithmsFactory::Instance = NULL;

ClusteringAlgorithmsFactory* ClusteringAlgorithmsFactory::GetInstance()
{
  if (Instance == NULL)
    Instance = new ClusteringAlgorithmsFactory();

  return Instance;
}

ClusteringAlgorithm*
ClusteringAlgorithmsFactory::GetClusteringAlgorithm(string                AlgorithmDefinition,
                                                    map<string, string>&  ClusteringParameters)
{
  ClusteringAlgorithm* Algorithm;
  DBSCAN             * dbScan_alg;
  
  if (AlgorithmDefinition.compare("DBSCAN") == 0)
  {
    // dbScan_alg = new DBSCAN(ClusteringParameters);
    // Algorithm = (ClusteringAlgorithm*) dbScan_alg;
    return (ClusteringAlgorithm*) new DBSCAN(ClusteringParameters);
  }
  else if (AlgorithmDefinition.compare("GMEANS") == 0)
  {
    
  }
  else
  {
    Algorithm = NULL;
    Error = true;
    ErrorMessage = "unknown clustering algorithm '"+AlgorithmDefinition+"'";
  }

  return Algorithm;
}
