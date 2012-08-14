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

  $Id::                                           $:  Id
  $Rev::                                          $:  Revision of last commit
  $Author::                                       $:  Author of last commit
  $Date::                                         $:  Date of last commit

\* -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=- */

#ifndef _PARTITION_HPP_
#define _PARTITION_HPP_

#include "clustering_types.h"

#include <vector>
using std::vector;

#include <set>
using std::set;

class Partition
{
  private:
    set<cluster_id_t>    _IDs;
    vector<cluster_id_t> _ClusterAssignmentVector;


  public:

    Partition(void);

    void clear(void);

    vector<cluster_id_t>& GetAssignmentVector(void);
    void SetAssignmentVector(vector<cluster_id_t>& ClusterAssignmentVector);

    set<cluster_id_t>& GetIDs(void);
    void               SetIDs(set<cluster_id_t>& IDs);
    cluster_id_t       GetMaxID(void);

    void MergeIDs(set<cluster_id_t>& OriginalIDs, cluster_id_t DefinitiveID);

    void UpdateIDs(void);

    size_t NumberOfClusters(void) const;
    bool   HasNoise(void) const;

};

#endif // _PARTITION_HPP_
