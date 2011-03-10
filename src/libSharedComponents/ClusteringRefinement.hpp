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

  $URL:: https://svn.bsc.es/repos/ClusteringSuite/trunk/src/libClustering/#$:

  $Rev:: 5                          $:  Revision of last commit
  $Author:: jgonzale                $:  Author of last commit
  $Date:: 2011-02-04 12:39:47 +0100#$:  Date of last commit

\* -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=- */
#ifndef _CLUSTERINGREFINEMENT_HPP_
#define _CLUSTERINGREFINEMENT_HPP_

#include <Error.hpp>
using cepba_tools::Error;

#include <libClustering.hpp>
#include "Partition.hpp"
#include "TraceData.hpp"

class ClusteringRefinement: public Error
{
  private:
    libClustering*    ClusteringCore;
    vector<Partition> PartitionsHistory;
    TraceData*        Data;

};

#endif // _CLUSTERINGREFINEMENT_HPP_
