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

  $URL::                                          $:  File
  $Rev::                                          $:  Revision of last commit
  $Author::                                       $:  Author of last commit
  $Date::                                         $:  Date of last commit

\* -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=- */

#include "ClusteringAlgorithmsFactory.hpp"
#include "DBSCAN.hpp"
#include "GMEANS.hpp"

#ifdef HAVE_MUSTER
#include "MUSTER_DBSCAN.hpp"
#include "MUSTER_PAM.hpp"
#include "MUSTER_XCLARA.hpp"
#include "MUSTER_CAPEK.hpp"
#endif

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
  
  if (AlgorithmDefinition.compare(DBSCAN::NAME) == 0)
  {
    return (ClusteringAlgorithm*) new DBSCAN(ClusteringParameters);
  }
  else if (AlgorithmDefinition.compare(GMEANS::NAME) == 0)
  {
    return (ClusteringAlgorithm*) new GMEANS(ClusteringParameters);
  }
#ifdef HAVE_MUSTER
  else if (AlgorithmDefinition.compare(MUSTER_DBSCAN::NAME) == 0)
  {
    return (ClusteringAlgorithm*) new MUSTER_DBSCAN(ClusteringParameters);
  }
  else if (AlgorithmDefinition.compare(MUSTER_PAM::NAME) == 0)
  {
    return (ClusteringAlgorithm*) new MUSTER_PAM(ClusteringParameters);
  }
  else if (AlgorithmDefinition.compare(MUSTER_XCLARA::NAME) == 0)
  {
    return (ClusteringAlgorithm*) new MUSTER_XCLARA(ClusteringParameters);
  }
  else if (AlgorithmDefinition.compare(MUSTER_CAPEK::NAME) == 0)
  {
    return (ClusteringAlgorithm*) new MUSTER_CAPEK(ClusteringParameters);
  }
#endif
  else
  {
    Algorithm = NULL;
    Error = true;
    ErrorMessage = "unknown clustering algorithm '"+AlgorithmDefinition+"'";
  }

  return Algorithm;
}
