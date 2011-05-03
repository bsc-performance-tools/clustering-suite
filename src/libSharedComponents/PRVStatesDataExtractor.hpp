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

  $URL:: https://svn.bsc.#$:  File
  $Rev:: 20               $:  Revision of last commit
  $Author:: jgonzale      $:  Author of last commit
  $Date:: 2010-03-09 17:1#$:  Date of last commit

\* -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=- */

#ifndef PRVSTATESDATAEXTRACTOR_H
#define PRVSTATESDATAEXTRACTOR_H

#include <trace_clustering_types.h>

#include "DataExtractor.hpp"

#include <math.h>
#include <string>

#include <map>
using std::map;

/* Forward declarations */
class ParaverTraceParser;
class State;
class Event;

/* Common semantic of Paraver */
#define RUNNING_STATE 1
#define HWC_GROUP_CHANGE_TYPE 42009999


class PRVStatesDataExtractor: public DataExtractor
{
  public:
    class TaskDataContainer
    {
      public:
        bool                             OngoingBurst;
        task_id_t                        TaskId;
        thread_id_t                      ThreadId;
        line_t                           Line;
        timestamp_t                      BeginTime;
        timestamp_t                      EndTime;
        duration_t                       BurstDuration;
        map<event_type_t, event_value_t> EventsData;
      
        TaskDataContainer() {
          TaskId        = 0;
          ThreadId      = 0;
          Line          = 0;
          BeginTime     = 0;
          EndTime       = 0;
          BurstDuration = 0;
          EventsData.clear();
          OngoingBurst  = false;
        };
      
        void Clear() {
          TaskId        = 0;
          ThreadId      = 0;
          Line          = 0;
          BeginTime     = 0;
          EndTime       = 0;
          BurstDuration = 0;
          EventsData.clear();
          OngoingBurst  = false;
        }
      
        TaskDataContainer& operator= (const TaskDataContainer& Other)
        {
          TaskId        = Other.TaskId;
          ThreadId      = Other.ThreadId;
          Line          = Other.Line;
          BeginTime     = Other.BeginTime;
          EndTime       = Other.EndTime;
          BurstDuration = Other.BurstDuration;
          EventsData    = Other.EventsData;
        }
    };
  
  private:
    ParaverTraceParser                 *TraceParser;
    vector< vector<TaskDataContainer> > TaskData;
    vector< vector<TaskDataContainer> > FutureTaskData;
    double                              TimeFactor;

    set<event_type_t> EventsToDealWith;
  
  public:
    
    PRVStatesDataExtractor(string InputTraceName);
    ~PRVStatesDataExtractor();

    bool SetEventsToDealWith(set<event_type_t>& EventsToDealWith);
    
    bool ExtractData(TraceData* TraceDataSet);

    input_file_t GetFileType(void) { return ParaverTrace; };
  
  private:

    bool NormalizeData(void);
  
    bool CheckState(State* CurrentState, TraceData* TraceDataSet);
  
    bool CheckEvent(Event* CurrentEvent, TraceData* TraceDataSet);
  
    void FillDataContainer(TaskDataContainer &DataContainer,
                           State             *CurrentState);
};

#endif /* PRVSTATESDATAEXTRACTOR_H */
