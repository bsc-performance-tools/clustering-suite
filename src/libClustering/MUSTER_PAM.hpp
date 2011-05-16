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

#ifndef _MUSTER_PAM_HPP_
#define _MUSTER_PAM_HPP_

#include "clustering_types.h"

#include "ClusteringAlgorithm.hpp"

#include "Partition.hpp"

#include <kmedoids.h>

#include <sstream>
using std::ostringstream;

#include <iostream>
using std::ostream;

#include <vector>
using std::vector;

#include <list>
using std::list;
using std::pair;

class MUSTER_PAM: public ClusteringAlgorithm
{
    typedef size_t point_idx;

  private:
    INT32               MaxClusters;

  public:

    static const string NAME;
    static const string MAX_CLUSTERS_STRING;
    
    MUSTER_PAM();
    
    MUSTER_PAM(map <string, string> ClusteringParameters);
    ~MUSTER_PAM(void)
    {
    };
    
    bool Run (const vector <const Point*> &Data,
              Partition                   &DataPartition,
              bool                         SimpleRun);
    
    string GetClusteringAlgorithmName (void) const;

    string GetClusteringAlgorithmNameFile (void) const;

    bool ComputeParamsApproximation (const vector < const Point* > &Data,
                                     INT32                          ParametersCount, ...);
  private:

    void ProcessClusterAssignment(cluster::kmedoids &muster_algorithm,
                                  Partition         &DataPartition,
                                  size_t             DataSize);
};

#endif /* _MUSTER_PAM_HPP_ */
