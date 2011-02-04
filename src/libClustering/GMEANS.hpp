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

#ifndef _GMEANS_HPP_
#define _GMEANS_HPP_

#include "ClusteringAlgorithm.hpp"
#include "clustering_types.h"

/* Gmeans C code declaration */
extern "C"
{

#include "gmeans/gmeans.h"
}


/* Forward declarations */
class Partition;

#include <sstream>
using std::ostringstream;

#include <iostream>
using std::ostream;

#include <vector>
using std::vector;

#include <list>
using std::list;
using std::pair;
class GMEANS:public ClusteringAlgorithm
{
  typedef size_t point_idx;
  private:

    UINT32 InitialPoints;
    double CriticalValue;
    INT32  MaxClusters;
    
  public:

    static const string NAME;
    
    static const string INITIAL_POINTS_STRING;
    static const string CRITICAL_VALUE_STRING;
    static const string MAX_CLUSTERS_STRING;
    GMEANS ();
    GMEANS (map < string, string > ClusteringParameters);
    ~GMEANS (void) {};
    
    bool Run (const vector <const Point *> &Data,
              Partition                    &DataPartition,
              bool                          SimpleRun);
    string GetClusteringAlgorithmName (void) const;
    string GetClusteringAlgorithmNameFile (void) const;
    bool
      ComputeParamsApproximation (const vector <const Point *> &Data,
                                  INT32                         ParametersCount, ...);
  private:
};
#endif                                            /* _DBSCAN_HPP_ */
