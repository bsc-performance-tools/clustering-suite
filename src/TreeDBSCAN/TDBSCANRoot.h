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

  $Id::                                    $:  Id
  $Rev::                                          $:  Revision of last commit
  $Author::                                       $:  Author of last commit
  $Date::                                         $:  Date of last commit

\* -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=- */

#ifndef __TDBSCAN_ROOT_H__
#define __TDBSCAN_ROOT_H__

#include <vector>
#include <FrontProtocol.h>
#include "TDBSCANCore.h"
#include "Statistics.h"
#include "ClustersInfo.h"

using namespace Synapse;

/**
 * This class implements the front-end side of the TDBSCAN protocol,
 * which is the same both for the on-line and off-line back-ends.
 */
class TDBSCANRoot : public TDBSCANCore, public FrontProtocol
{
  private:
      string OutputGlobalClusteringFileName;
      libDistributedClustering *libClustering;

  protected:
      string OutputPrefix;

  public:
    TDBSCANRoot (double Eps,
                 int    MinPts,
                 string ClusteringDefinitionXML,
                 string InputTraceName,
                 string OutputFileName,
                 bool   Verbose,
                 bool   ReconstructTrace);

    TDBSCANRoot (string ClusteringDefinitionXML,
                 bool   Verbose);

    string ID()
    {
      return "TDBSCAN";
    }
    void Setup (void);
    int  Run  (void);

    void PrintGraphStats (Statistics &ClusteringStats);
};

#endif /* __TDBSCAN_ROOT_H__ */

