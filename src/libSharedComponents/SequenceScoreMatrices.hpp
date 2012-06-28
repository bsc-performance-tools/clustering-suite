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

  $Id:: ClusteringRefinementAggregative.cpp 25 20#$:  Id
  $Rev:: 25                                       $:  Revision of last commit
  $Author:: jgonzale                              $:  Author of last commit
  $Date:: 2011-07-13 14:59:54 +0200 (Wed, 13 Jul #$:  Date of last commit

\* -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=- */

#ifndef _SEQUENCESCOREMATRICS_HPP_
#define _SEQUENCESCOREMATRICS_HPP_

#if 0
// using namespace seqan;

// FRAGMENT(user-defined-matrix)
// Extend SeqAn by a user-define scoring matrix.
namespace seqan {

// We have to create a new specialization of the ScoringMatrix_ class
// for amino acids.  For this, we first create a new tag.
struct UserDefinedMatrix {};

// Then, we specialize the class ScoringMatrix_.
template <>
struct ScoringMatrixData_<int, cluster_id_t, UserDefinedMatrix>
{
  static int differentClusters;

  static inline void initialize_matrix(int ClustersCount)
  {
    differentClusters = ClustersCount;
  }

  static inline int const * getData() {

    int _data[differenClusters*differentClusters];

    for (size_t i = 0; i < differentClusters; i++)
    {
      for (size_t j = 0; j < differentClusters; j++)
      {
        _data[i*differentClusters + j] = -1;
      }
    }

    return _data;
  }
};

}  // namespace seqan

#endif

#endif
