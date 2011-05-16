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

#ifndef _CLUSTEREDEVENTSPRVGENERATOR_HPP_
#define _CLUSTEREDEVENTSPRVGENERATOR_HPP_

#include "ClusteredTraceGenerator.hpp"
#include "CPUBurst.hpp"

#include <vector>
using std::vector;

class ParaverTraceParser;
class Event;

class ClusteredEventsPRVGenerator: public ClusteredTraceGenerator
{
  private:
    ParaverTraceParser *TraceParser;
  
    bool   PCFPresent;
    string InputPCFName;
    FILE  *InputPCFFile;
    string OutputPCFName;
    FILE  *OutputPCFFile;
  
    vector<CPUBurst*> BurstsBeginTime;

    set<event_type_t> EventsToDealWith;
  
  public:
    ClusteredEventsPRVGenerator(string  InputTraceName,
                          string  OutputTraceName);

    ~ClusteredEventsPRVGenerator(void){};

    bool SetEventsToDealWith (set<event_type_t>& EventsToDealWith);
    
    bool Run(vector<CPUBurst*>&    Bursts,
             vector<cluster_id_t>& IDs,
             size_t                NumberOfClusters,
             bool                  MinimizeInformation = false);
  
    bool GenerateOutputPCF(size_t NumberOfClusters);
  
  private:

    bool BurstOpeningEvent(Event* CurrentEvent);

    bool BurstClosingEvent(Event* CurrentEvent);
};

#endif

