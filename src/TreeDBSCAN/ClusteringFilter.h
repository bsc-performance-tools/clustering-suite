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

  $Id:: libDistributedClustering.cpp 51 2011-11-2#$:  Id
  $Rev:: 51                                       $:  Revision of last commit
  $Author:: jgonzale                              $:  Author of last commit
  $Date:: 2011-11-24 15:47:29 +0100 (Thu, 24 Nov #$:  Date of last commit

\* -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=- */

#ifndef __CLUSTERING_FILTER_H__
#define __CLUSTERING_FILTER_H__

#include <mrnet/NetworkTopology.h>
#include <HullModel.hpp>

#include <vector>
using std::vector;

using namespace MRN;


#define TOP_FILTER(top_info)    (top_info.get_NumSiblings() == 0)
#define BOTTOM_FILTER(top_info) (top_info.get_NumChildren() == 0)
#define FILTER_ID(top_info)     (top_info.get_Rank())

extern "C" {

void Init(const TopologyLocalInfo &top_info);

void MergeAlltoAll(vector<HullModel*> &ClustersHulls,
                   vector<HullModel*> &MergedModel,
                   double              Epsilon,
                   int                 MinPoints);

}

#endif /* __CLUSTERING_FILTER_H__ */
