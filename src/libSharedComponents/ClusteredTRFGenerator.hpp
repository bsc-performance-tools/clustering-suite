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

#include "ClusteredTraceGenerator.hpp"


#include <climits>
#include <cstdio>

#include <string>

#include <vector>
using std::vector;

class ClusteredTRFGenerator: public ClusteredTraceGenerator
{
  public:
    static const INT32 NO_MORE_CLUSTERS             = INT_MIN;
    static const INT32 READ_ERROR                   = INT_MIN + 1;

  private:
    bool          PrintClusterBlocks;

  public:
    ClusteredTRFGenerator(string  InputTraceName,
                          string  OutputTraceName,
                          bool    PrintClusterBlocks = false);

    ~ClusteredTRFGenerator(void);
  
    void SetPrintClusterBlocks (bool PrintClusterBlocks)
    {
      this->PrintClusterBlocks = PrintClusterBlocks;
    }

    bool SetEventsToDealWith (set<event_type_t>& EventsToDealWith);
    
    bool Run(vector<CPUBurst*>&    ClusteringBursts,
             vector<cluster_id_t>& IDs,
             size_t                NumberOfClusters,
             bool                  MinimizeInformation = false);

  private:
    INT32 GetNextClusterId(void);

    bool CheckFirstBlockDefinition(char*  Buffer,
                                   size_t NumberOfClusters);
                                   

    bool PrintClusteredBurst(FILE        *Trace,
                             long         TaskId,
                             long         ThreadId,
                             double       BurstDuration,
                             cluster_id_t ClusterId,
                             bool         ReplaceDurations = false);
};

typedef ClusteredTRFGenerator* ClusteredTRFGenerator_t;
