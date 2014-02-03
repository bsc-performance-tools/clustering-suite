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

  $Id:: PRVStatesDataExtractor.hpp 85 2013-05-06 #$:  Id
  $Rev:: 85                                       $:  Revision of last commit
  $Author:: jgonzale                              $:  Author of last commit
  $Date:: 2013-05-06 15:35:11 +0200 (lun, 06 may #$:  Date of last commit

\* -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=- */

#ifndef PRVSEMANTICGUIDEDDATAEXTRACTOR_H
#define PRVSEMANTICGUIDEDDATAEXTRACTOR_H

#include <trace_clustering_types.h>

#include "DataExtractor.hpp"
#include "ParaverTraceParser.hpp"

#include <math.h>
#include <string>

#include <vector>
using std::vector;

#include <map>
using std::map;

#include <list>
using std::list;

#include <fstream>
using std::ifstream;

/* Common semantic of Paraver */
#define RUNNING_STATE 1
#define HWC_GROUP_CHANGE_TYPE 42009999

/* Semantic CSV fields */
#define SEMANTIC_CSV_FIELDS        4

class PRVSemanticGuidedDataExtractor: public DataExtractor
{
  public:
    class BurstContainer
    {
      public:
        task_id_t                        TaskId;
        thread_id_t                      ThreadId;
        line_t                           Line;
        timestamp_t                      BeginTime;
        timestamp_t                      EndTime;
        duration_t                       Duration;
        map<event_type_t, event_value_t> EventsData;
        bool                             IntermediateHWChange;
        set<event_type_t>                NotCommonEvents;
        set<event_type_t>                BurstEndEvents;

        string toString(void);
    };

  private:
    string              InputSemanticCSV;
    ifstream            SemanticCSVStream;
    ParaverTraceParser *TraceParser;
    double              TimeFactor;

    bool                OneThreadPerTask;

    // vector<BurstContainer*> BurstsToLoad;
    map<string, list<BurstContainer*> > BurstsToLoad;

  public:

    PRVSemanticGuidedDataExtractor(string InputTraceName, string InputSemanticCSV);
    ~PRVSemanticGuidedDataExtractor();

    bool SetEventsToDealWith(set<event_type_t>& EventsToDealWith,
                             bool               ConsecutivEvts);

    bool GetPartition(Partition& DataPartition) { return false; };

    bool ExtractData(TraceData* TraceDataSet);

    input_file_t GetFileType(void) { return SematicGuided; };

  private:

    bool ProcessSemanticCSV(timestamp_t CutOffset);

    void PopulateRecord(vector<string> &Record,
                        const string   &Line,
                        char            Delimiter);

    bool PartialBurstFill(vector<string> &Record,
                          timestamp_t     Offset,
                          UINT32          CurrentLine);

    bool ProcessEvent(Event* CurrentEvent, TraceData* TraceDataSet);
};

#endif /* PRVSTATESDATAEXTRACTOR_H */
