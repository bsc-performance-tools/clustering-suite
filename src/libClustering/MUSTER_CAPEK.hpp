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

  $URL:: https://svn.bsc.es/repos/ptools/prv2dim/                          $:

  $Rev:: 478                        $:  Revision of last commit
  $Author:: jgonzale                $:  Author of last commit
  $Date:: 2010-10-28 13:58:59 +0200 $:  Date of last commit

\* -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=- */

#ifndef _MUSTER_CAPEK_HPP_
#define _MUSTER_CAPEK_HPP_

#include "ClusteringAlgorithm.hpp"
#include "clustering_types.h"

#include <Point.hpp>

#include "Partition.hpp"

#include <par_kmedoids.h>

#include <sstream>
using std::ostringstream;

#include <iostream>
using std::ostream;

#include <vector>
using std::vector;

#include <list>
using std::list;
using std::pair;


/* Internal points for CAPEK */
class CAPEK_Point: public Point
{
  public:
    CAPEK_Point (void) {};
    
    CAPEK_Point(Point OriginalPoint);
    /* Serialization methods required by muster */
    /// Returns the size of a packed point
    int packed_size(MPI_Comm comm) const;
  
    /// Packs a point into an MPI packed buffer
    void pack(void *buf, int bufsize, int *position, MPI_Comm comm) const;

    /// Unpacks a point from an MPI packed buffer
    static Point unpack(void *buf, int bufsize, int *position, MPI_Comm comm);
};

class MUSTER_CAPEK: public ClusteringAlgorithm
{
    typedef size_t point_idx;

  private:
    INT32 k;

  public:

    static const string NAME;
    static const string K_STRING;
    
    MUSTER_CAPEK();
    
    MUSTER_CAPEK(map < string, string > ClusteringParameters);
    ~MUSTER_CAPEK(void)
    {
    };
    
    bool Run (const vector <const Point*> &Data,
              Partition                   &DataPartition,
              bool                         SimpleRun);
    
    string GetClusteringAlgorithmName (void) const;

    string GetClusteringAlgorithmNameFile (void) const;

    bool   IsDistributed (void) { return true; };

    bool ComputeParamsApproximation (const vector < const Point* > &Data,
                                     INT32                          ParametersCount, ...);
  private:

    void ProcessClusterAssignment(cluster::par_kmedoids &muster_algorithm,
                                  Partition            &DataPartition,
                                  size_t                DataSize);
};

#endif /* _MUSTER_DBSCAN_HPP_ */
