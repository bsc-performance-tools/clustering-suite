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

#ifndef PRVEVENTSDATAEXTRACTOR_H
#define PRVEVENTSDATAEXTRACTOR_H

#include <trace_clustering_types.h>

#include "DataExtractor.hpp"

#include <math.h>
#include <string>

#include <map>
using std::map;

#include <stack>
using std::stack;

/* Forward declarations */
class ParaverTraceParser;
class State;
class Event;

/* Common semantic of Paraver */
#define RUNNING_STATE 1
#define HWC_GROUP_CHANGE_TYPE 41999999

class PRVEventsDataExtractor: public DataExtractor
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
        bool                             IntermediateHWChange;
        set<event_type_t>                NotCommonEvents;
        set<event_type_t>                BurstEndEvents;

        TaskDataContainer() {
          OngoingBurst         = false;
          TaskId               = 0;
          ThreadId             = 0;
          Line                 = 0;
          BeginTime            = 0;
          EndTime              = 0;
          BurstDuration        = 0;
          EventsData.clear();
          IntermediateHWChange = false;
          NotCommonEvents.clear();
          BurstEndEvents.clear();
        };

        void Clear() {
          OngoingBurst          = false;
          TaskId                = 0;
          ThreadId              = 0;
          Line                  = 0;
          BeginTime             = 0;
          EndTime               = 0;
          BurstDuration         = 0;
          EventsData.clear();
          IntermediateHWChange  = false;
          NotCommonEvents.clear();
          BurstEndEvents.clear();
        }

        TaskDataContainer& operator= (const TaskDataContainer& Other)
        {
          OngoingBurst         = Other.OngoingBurst;
          TaskId               = Other.TaskId;
          ThreadId             = Other.ThreadId;
          Line                 = Other.Line;
          BeginTime            = Other.BeginTime;
          EndTime              = Other.EndTime;
          BurstDuration        = Other.BurstDuration;
          EventsData           = Other.EventsData;
          IntermediateHWChange = Other.IntermediateHWChange;
          NotCommonEvents      = Other.NotCommonEvents;
          BurstEndEvents       = Other.BurstEndEvents;

          return *this;
        }
    };

  private:
    ParaverTraceParser                   *TraceParser;
    vector<vector<TaskDataContainer> >    TaskData;
    vector<vector<TaskDataContainer> >    FutureTaskData;
    vector<vector<stack<event_type_t> > > EventsStack;

    double                              TimeFactor;

    set<event_type_t> EventsToDealWith;
    bool              ConsecutiveEvts;

  public:

    PRVEventsDataExtractor(string InputTraceName);
    ~PRVEventsDataExtractor();

    bool SetEventsToDealWith (set<event_type_t>& EventsToDealWith,
                              bool               ConsecutiveEvts);

    bool GetPartition(Partition& DataPartition) { return false; };

    bool ExtractData(TraceData* TraceDataSet);

    input_file_t GetFileType(void) { return ParaverTrace; };

  private:

    bool NormalizeData(void);

    bool CheckEvent(Event* CurrentEvent, TraceData* TraceDataSet);

    void FillDataContainer(TaskDataContainer &DataContainer,
                           Event             *CurrentEvent);

    bool GenerateBurst(TraceData*         TraceDataSet,
                       TaskDataContainer& Data);

    bool BurstOpeningEvent(event_type_t EventType, event_value_t EventValue);

    bool BurstClosingEvent(event_type_t EventType, event_value_t EventValue);

    bool UpdateTaskData(TaskDataContainer& DataContainer,
                        event_type_t       EventType,
                        event_value_t      EventValue,
                        timestamp_t        EventTime);
};

#endif /* PRVSTATESDATAEXTRACTOR_H */
