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

#include <SystemMessages.hpp>
using cepba_tools::system_messages;

#include "PRVStatesDataExtractor.hpp"
#include "ParaverTraceParser.hpp"

#include <cstring>
#include <cerrno>

#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

#include <iostream>
using std::cout;
using std::cerr;
using std::endl;

#include <sstream>
using std::ostringstream;

// #define DEBUG_PARAVER_INPUT 1

/******************************************************************************
 * Class 'TaskDataContainer'
 ******************************************************************************/
string PRVStatesDataExtractor::TaskDataContainer::toString(void)
{
    ostringstream Result;

  Result << "[" << TaskId << "|" << ThreadId << "] (" << Line << ") ";
  Result << "Begin: " << BeginTime << " End: " << EndTime << " (Dur:";
  Result << BurstDuration << ")" << endl;

  Result << "*** EventsData *** " << endl;
  for (map<event_type_t, event_value_t>::iterator it  = EventsData.begin();
                                                  it != EventsData.end();
                                                ++it)
  {
    Result << "[ " << it->first << " ] = " << it->second << endl;
  }

  Result << "*** BurstEndEvents ***" << endl;
  Result << "[ ";
  for (set<event_type_t>::iterator it  = BurstEndEvents.begin();
                                   it != BurstEndEvents.end();
                                 ++it)
  {
    Result << (*it) <<  " ";
  }
  Result << "]";

  return Result.str();
}

/******************************************************************************
 * Class 'PRVStatesDataExtractor'
 ******************************************************************************/
PRVStatesDataExtractor::PRVStatesDataExtractor(string InputTraceName)
:DataExtractor(InputTraceName)
{
  if (GetError())
    return;

  TraceParser = new ParaverTraceParser(InputTraceName, InputTraceFile);

  if (!TraceParser->InitTraceParsing())
  {
    SetError(true);
    SetErrorMessage("error init tracing", TraceParser->GetLastError());
    return;
  }

  if (TraceParser->GetTimeUnits() == MICROSECONDS)
    TimeFactor = 1e3;
  else
    TimeFactor = 1;

  return;
}

PRVStatesDataExtractor::~PRVStatesDataExtractor()
{
  unlink(TraceDataFileName.c_str());
}

bool PRVStatesDataExtractor::SetEventsToDealWith (set<event_type_t>& EventsToDealWith,
                                                  bool               ConsecutiveEvts)
{
  SetError(true);
  SetErrorMessage("trying to use a state guided parser to an event guided parsing");

  return false;
}

bool PRVStatesDataExtractor::ExtractData(TraceData* TraceDataSet)
{
  vector<ApplicationDescription_t> AppsDescription;
  vector<TaskDescription_t>        TaskInfo;
  size_t                           TraceObjects = 0;

  ParaverRecord *CurrentRecord;
  State         *CurrentState;
  Event         *CurrentEvent;

  INT32  CurrentPercentage = 0;

  /*
  if (InputDataManager == NULL)
  {
    SetError(true);
    SetErrorMessage("data manager must be defined to extract data");
    return false;
  }
  */

  /* Create the structure to manage the different bursts foreach task/thread */
  AppsDescription = TraceParser->GetApplicationsDescription();

  if (TraceParser->GetError())
  {
    SetError(true);
    SetErrorMessage("unable to start data extraction",
                    TraceParser->GetLastError());
    return false;
  }

  if (AppsDescription.size() != 1)
  {
    /* DEBUG
    cout << "Application description size is " << AppsDescription.size() << endl;
    */

    SetError(true);
    SetErrorMessage("unable to apply cluster analysis on a trace with more than one application");
    return false;
  }

  TaskInfo = AppsDescription[0]->GetTaskInfo();

  /* Set the number of tasks on trace data, to perform the possible data distribution */
  TraceDataSet->SetNumberOfTasks(TaskInfo.size());

  for (INT32 i = 0; i < TaskInfo.size(); i++)
  {
    TraceObjects += TaskInfo[i]->GetThreadCount();
    TaskData.push_back(vector<TaskDataContainer>(TaskInfo[i]->GetThreadCount()));
    FutureTaskData.push_back(vector<TaskDataContainer>(TaskInfo[i]->GetThreadCount()));
  }

  TraceDataSet->SetTraceObjects(TraceObjects);

  for (INT32 i = 0; i < TaskData.size(); i++)
  {
    for (INT32 j = 0; j < TaskData[i].size(); j++)
    {
      /*
      TaskData[i][j].TaskId   = i;
      TaskData[i][j].ThreadId = j;
      */
      TaskData[i][j].Clear();

      /*
      FutureTaskData[i][j].TaskId   = i;
      FutureTaskData[i][j].ThreadId = j;
      */

      FutureTaskData[i][j].Clear();
    }
  }

  CurrentPercentage = TraceParser->GetFilePercentage();

  system_messages::show_percentage_progress("Parsing Paraver Input Trace",
                                            CurrentPercentage);

  while (true)
  {
    INT32 PercentageRead;

    CurrentRecord = TraceParser->GetNextRecord(STATE_REC|EVENT_REC);

    if (CurrentRecord == NULL)
      break;

    if (CurrentRecord->GetRecordType() == PARAVER_STATE)
    {
      if (!CheckState((State*) CurrentRecord, TraceDataSet))
        return false;
    }
    else if (CurrentRecord->GetRecordType() == PARAVER_EVENT)
    {
      if (!CheckEvent((Event*) CurrentRecord, TraceDataSet))
        return false;
    }
    else
    {
      SetError(true);
      SetErrorMessage("unable to get a correct record from input trace");
      return false;
    }

    /* Show progress */
    PercentageRead = TraceParser->GetFilePercentage();
    if (PercentageRead > CurrentPercentage)
    {
      CurrentPercentage = PercentageRead;
      system_messages::show_percentage_progress("Parsing Paraver Input Trace", CurrentPercentage);
    }

    /* Free memory used by the parser */
    delete CurrentRecord;
  }

  if (TraceParser->GetError())
  {
    SetError(true);
    SetErrorMessage("error parsing trace ", TraceParser->GetLastError());
    return false;
  }

  system_messages::show_percentage_end("Parsing Paraver Input Trace");

  if (ferror(InputTraceFile) != 0)
  {
    SetError(true);
    SetErrorMessage("error reading input trace file", strerror(errno));
    return false;
  }
  else
  { /* Check if there is any pending burst */
    for (INT32 i = 0; i < TaskData.size(); i++)
    {
      for (INT32 j = 0; j < TaskData[i].size(); j++)
      {
        if (TaskData[i][j].OngoingBurst)
        { /* There is an ongoing burst.  */

#ifdef DEBUG_PARAVER_INPUT
          cout << "*** CALLING TraceDataSet->NewBurst ***" << endl;
          cout << TaskData[i][j].toString() << endl;
#endif

          if (!TraceDataSet->NewBurst(TaskData[i][j].TaskId,
                                      TaskData[i][j].ThreadId,
                                      TaskData[i][j].Line,
                                      TaskData[i][j].BeginTime,
                                      TaskData[i][j].EndTime,
                                      TaskData[i][j].BurstDuration,
                                      TaskData[i][j].EventsData,
                                      TaskData[i][j].BurstEndEvents))  /* DEBUG (2013/10/23): To be changed to correct the parsing */
          {
            SetError(true);
            SetErrorMessage("error storing burst data",
                            TraceDataSet->GetLastError());
            return false;
          }
        }
      }
    }
  }

  if (!TraceDataSet->DataExtractionFinished())
  {
    SetError(true);
    SetErrorMessage(TraceDataSet->GetLastError());
    return false;
  }

  /* No more burst
  if (!TraceDataSet->NoMoreBursts())
  {
    SetError(true);
    SetErrorMessage("error finishing data adcquisition");
    return false;
  }
  */

#ifdef DEBUG_PARAVER_INPUT
  // cout << "Data Size = " << TraceDataSet->GetDataSetSize() << endl;
#endif

  if (TraceDataSet->GetClusteringBurstsSize() == 0)
  {
    SetError(true);
    SetErrorMessage("No bursts extracted, cluster analysis cannot proceed");
    return false;
  }
  else
  {
    ostringstream Message;
    Message << "Points to analyse " << TraceDataSet->GetClusteringBurstsSize() << endl;
    system_messages::silent_information(Message.str());
  }

  return true;
}


bool PRVStatesDataExtractor::CheckState(State     *CurrentState,
                                        TraceData *TraceDataSet)
{
  TaskDataContainer &CurrentTaskData =
    TaskData[CurrentState->GetTaskId()][CurrentState->GetThreadId()];

  TaskDataContainer &NextTaskData =
    FutureTaskData[CurrentState->GetTaskId()][CurrentState->GetThreadId()];



  /* Check if this new state is a running state */
  if (CurrentState->GetStateValue() == RUNNING_STATE)
  {
    if (CurrentTaskData.OngoingBurst)
    {
      if (CurrentTaskData.EndTime == CurrentState->GetBeginTime())
      { /* There is an ongoing burst that finishes when current state starts! */

        FillDataContainer(NextTaskData, CurrentState);

#ifdef DEBUG_PARAVER_INPUT
        printf("Double Ongoing BURST for T%02d:Th%02d (%lld - %lld)\n",
               NextTaskData.TaskId,
               NextTaskData.ThreadId,
               NextTaskData.BeginTime,
               NextTaskData.EndTime);
#endif
      }
      else if (CurrentTaskData.EndTime < CurrentState->GetBeginTime())
      { /* There is an ongoing burst.  */

#ifdef DEBUG_PARAVER_INPUT
        /*
        printf("Flushing BURST for T%02d:Th%02d (%lld - %lld) - NEW STATE\n",
               CurrentTaskData.TaskId,
               CurrentTaskData.ThreadId,
               CurrentTaskData.BeginTime,
               CurrentTaskData.EndTime);
        */
        cout << "*** CALLING TraceDataSet->NewBurst ***" << endl;
        cout << CurrentTaskData.toString() << endl;
#endif

        if (!TraceDataSet->NewBurst(CurrentTaskData.TaskId,
                                    CurrentTaskData.ThreadId,
                                    CurrentTaskData.Line,
                                    CurrentTaskData.BeginTime,
                                    CurrentTaskData.EndTime,
                                    CurrentTaskData.BurstDuration,
                                    CurrentTaskData.EventsData,
                                    CurrentTaskData.BurstEndEvents)) /* DEBUG (2013/10/23): To be changed to correct the parsing */
        {
          SetError(true);
          SetErrorMessage("error storing burst data",
                          TraceDataSet->GetLastError());
          return false;
        }

        if (NextTaskData.OngoingBurst)
        { /* There is a pending burst */
#ifdef DEBUG_PARAVER_INPUT
        printf("Changing Next Data T%02d:Th%02d (%lld - %lld <- %lld - %lld)\n",
               CurrentTaskData.TaskId,
               CurrentTaskData.ThreadId,
               CurrentTaskData.BeginTime,
               CurrentTaskData.EndTime,
               NextTaskData.BeginTime,
               NextTaskData.EndTime);
#endif

          CurrentTaskData = NextTaskData;
          NextTaskData.Clear();
          FillDataContainer(NextTaskData, CurrentState);

#ifdef DEBUG_PARAVER_INPUT
        printf("Double Ongoing BURST for T%02d:Th%02d (%lld - %lld)\n",
               NextTaskData.TaskId,
               NextTaskData.ThreadId,
               NextTaskData.BeginTime,
               NextTaskData.EndTime);
#endif

        }
        else
        {
          CurrentTaskData.Clear();

          FillDataContainer(CurrentTaskData, CurrentState);

#ifdef DEBUG_PARAVER_INPUT
          printf("Ongoing BURST for T%02d:Th%02d (%lld - %lld)\n",
                 CurrentTaskData.TaskId,
                 CurrentTaskData.ThreadId,
                 CurrentTaskData.BeginTime,
                 CurrentTaskData.EndTime);
#endif
        }
      }
    }
    else
    {
      FillDataContainer(CurrentTaskData, CurrentState);
#ifdef DEBUG_PARAVER_INPUT
      printf("Ongoing BURST for T%02d:Th%02d (%lld - %lld)\n",
             CurrentTaskData.TaskId,
             CurrentTaskData.ThreadId,
             CurrentTaskData.BeginTime,
             CurrentTaskData.EndTime);
#endif
    }
  }
  else
  { /* Is not a running state. Wait if there are more events on the same
     * timestamp */
#ifdef DEBUG_PARAVER_INPUT
      printf("Overlapped state change for T%02d:Th%02d (%lld - %lld)\n",
             CurrentState->GetTaskId(),
             CurrentState->GetThreadId(),
             CurrentState->GetBeginTime(),
             CurrentState->GetEndTime());
#endif
  }

  return true;
}

bool
PRVStatesDataExtractor::CheckEvent(Event     *CurrentEvent,
                                   TraceData *TraceDataSet)
{
  map<event_type_t, event_value_t>::iterator EventsDataIterator;

  TaskDataContainer &CurrentTaskData =
    TaskData[CurrentEvent->GetTaskId()][CurrentEvent->GetThreadId()];

  TaskDataContainer &NextTaskData =
    FutureTaskData[CurrentEvent->GetTaskId()][CurrentEvent->GetThreadId()];

  if (CurrentTaskData.OngoingBurst)
  {
    if (CurrentEvent->GetTimestamp() > CurrentTaskData.EndTime)
    { /* Events not related to the ongoing burst. It must be flushed */

#ifdef DEBUG_PARAVER_INPUT
      /*
      printf("Flushing BURST for T%02d:Th%02d (%lld - %lld) - FUTURE EVENT (%lld)\n",
             CurrentTaskData.TaskId,
             CurrentTaskData.ThreadId,
             CurrentTaskData.BeginTime,
             CurrentTaskData.EndTime,
             CurrentEvent->GetTimestamp());
      */
      cout << "*** CALLING TraceDataSet->NewBurst ***" << endl;
      cout << CurrentTaskData.toString() << endl;
#endif

      if (!TraceDataSet->NewBurst(CurrentTaskData.TaskId,
                                  CurrentTaskData.ThreadId,
                                  CurrentTaskData.Line,
                                  CurrentTaskData.BeginTime,
                                  CurrentTaskData.EndTime,
                                  CurrentTaskData.BurstDuration,
                                  CurrentTaskData.EventsData,
                                  CurrentTaskData.BurstEndEvents)) /* DEBUG (2013/10/23): To be changed to correct the parsing */
      {
        SetError(true);
        SetErrorMessage("error storing burst data",
                        TraceDataSet->GetLastError());
        return false;
      }

      /* And also check if there is a future burst */

      if (NextTaskData.OngoingBurst)
      {

#ifdef DEBUG_PARAVER_INPUT
        printf("Changing Next Data T%02d:Th%02d (%lld - %lld <- %lld - %lld)\n",
               CurrentTaskData.TaskId,
               CurrentTaskData.ThreadId,
               CurrentTaskData.BeginTime,
               CurrentTaskData.EndTime,
               NextTaskData.BeginTime,
               NextTaskData.EndTime);
#endif

        CurrentTaskData = NextTaskData;
        NextTaskData.Clear();
      }
      else
      {
        CurrentTaskData.Clear();
      }
    }
    /* BUGFIX: once we update the current task data, we have to check the
     * validity of the current event */
    // else if (CurrentEvent->GetTimestamp() == CurrentTaskData.EndTime) */

    if (CurrentEvent->GetTimestamp() == CurrentTaskData.EndTime)
    { /* Events at the end of the CPU burst */
      for (INT32 i = 0; i < CurrentEvent->GetTypeValueCount(); i++)
      {
        event_type_t  CurrentEventType  = CurrentEvent->GetType(i);
        event_value_t CurrentEventValue = CurrentEvent->GetValue(i);


        /* Check if there is a HWC change */
        if (CurrentEventType == HWC_GROUP_CHANGE_TYPE)
        { /* Hardware counters change. Future events are not eligible */
#ifdef DEBUG_PARAVER_INPUT
          /*
          printf("Flushing BURST for T%02d:Th%02d (%lld - %lld) - HWC CHANGE\n",
                 CurrentTaskData.TaskId,
                 CurrentTaskData.ThreadId,
                 CurrentTaskData.BeginTime,
                 CurrentTaskData.EndTime);
          */
          cout << "*** CALLING TraceDataSet->NewBurst ***" << endl;
          cout << CurrentTaskData.toString() << endl;
#endif

          if (!TraceDataSet->NewBurst(CurrentTaskData.TaskId,
                                      CurrentTaskData.ThreadId,
                                      CurrentTaskData.Line,
                                      CurrentTaskData.BeginTime,
                                      CurrentTaskData.EndTime,
                                      CurrentTaskData.BurstDuration,
                                      CurrentTaskData.EventsData,
                                      CurrentTaskData.BurstEndEvents)) /* DEBUG (2013/10/23): To be changed to correct the parsing */
          {
            SetError(true);
            SetErrorMessage("error storing burst data",
                            TraceDataSet->GetLastError());
            return false;
          }

          /* And also check if there is a future burst */

          if (NextTaskData.OngoingBurst)
          {

    #ifdef DEBUG_PARAVER_INPUT
            printf("Changing Next Data T%02d:Th%02d (%lld - %lld <- %lld - %lld)\n",
                   CurrentTaskData.TaskId,
                   CurrentTaskData.ThreadId,
                   CurrentTaskData.BeginTime,
                   CurrentTaskData.EndTime,
                   NextTaskData.BeginTime,
                   NextTaskData.EndTime);
    #endif

            CurrentTaskData = NextTaskData;
            NextTaskData.Clear();

          }
          else
          {
            CurrentTaskData.Clear();
          }
        }
        else
        {
          /* Burst end events */
          CurrentTaskData.BurstEndEvents.insert(CurrentEventType);

          /* Avoid repeating events with same type */
          EventsDataIterator = CurrentTaskData.EventsData.find(CurrentEventType);

          if (EventsDataIterator == CurrentTaskData.EventsData.end())
          { /* Add this event to 'EventsData' map */
            CurrentTaskData.EventsData[CurrentEventType] = CurrentEventValue;

  #ifdef DEBUG_PARAVER_INPUT
            /*
            printf("Burst BeginTime = %lld EndTime = %lld\n",
                   CurrentTaskData.BeginTime, CurrentTaskData.EndTime);
            printf("Storing data for T%02d:Th%02d (%lld) [%d:%lld]\n",
                   CurrentEvent->GetTaskId(),
                   CurrentEvent->GetThreadId(),
                   CurrentEvent->GetTimestamp(),
                   CurrentEventType,
                   CurrentEventValue);
            */
  #endif
          }
          else
          { /* This event was already stored in the map. If necessary, sum values
             */
            CurrentTaskData.EventsData[CurrentEventType] += CurrentEventValue;

  #ifdef DEBUG_PARAVER_INPUT
            /*
            printf("Adding data for T%02d:Th%02d (%lld )[%d:%lld]\n",
                   CurrentEvent->GetTaskId(),
                   CurrentEvent->GetThreadId(),
                   CurrentEvent->GetTimestamp(),
                   CurrentEventType,
                   CurrentTaskData.EventsData[CurrentEventType]);
            */
  #endif
          }
        }
      }
    }
    else if (CurrentEvent->GetTimestamp() > CurrentTaskData.BeginTime &&
             CurrentEvent->GetTimestamp() < CurrentTaskData.EndTime)
    { /* BUGFIX (08/05/2013): The second line of the comparison is to
       * guarantee that we have not changed the ongoing burst! */

      /* Events inside the burst (SAMPLING!) */
      for (INT32 i = 0; i < CurrentEvent->GetTypeValueCount(); i++)
      {
        INT32 CurrentEventType  = CurrentEvent->GetType(i);
        INT64 CurrentEventValue = CurrentEvent->GetValue(i);


        /* Avoid repeating events with same type */

        EventsDataIterator = CurrentTaskData.EventsData.find(CurrentEventType);

        if (EventsDataIterator == CurrentTaskData.EventsData.end())
        { /* Add this event to 'EventsData' map */
          CurrentTaskData.EventsData[CurrentEventType] = CurrentEventValue;

#ifdef DEBUG_PARAVER_INPUT
          printf("Burst BeginTime = %lld EndTime = %lld\n",
                   CurrentTaskData.BeginTime, CurrentTaskData.EndTime);

          printf("Storing data for T%02d:Th%02d (%lld) [%d:%lld]\n",
                 CurrentEvent->GetTaskId(),
                 CurrentEvent->GetThreadId(),
                 CurrentEvent->GetTimestamp(),
                 CurrentEventType,
                 CurrentEventValue);
#endif
        }
        else
        { /* This event was already stored in the map. If necessary, sum values
           */
          CurrentTaskData.EventsData[CurrentEventType] += CurrentEventValue;

#ifdef DEBUG_PARAVER_INPUT
          printf("Adding data for T%02d:Th%02d (%lld) [%d:%lld]\n",
                 CurrentEvent->GetTaskId(),
                 CurrentEvent->GetThreadId(),
                 CurrentEvent->GetTimestamp(),
                 CurrentEventType,
                 CurrentTaskData.EventsData[CurrentEventType]);
#endif
        }
      }
    }
  }

  return true;
}

void
PRVStatesDataExtractor::FillDataContainer(TaskDataContainer &TaskData,
                                          State             *CurrentState)
{
  TaskData.TaskId        = CurrentState->GetTaskId();
  TaskData.ThreadId      = CurrentState->GetThreadId();
  TaskData.Line          = CurrentState->GetLine();
  TaskData.OngoingBurst  = true;
  TaskData.BeginTime     = CurrentState->GetBeginTime();
  TaskData.EndTime       = CurrentState->GetEndTime();
  TaskData.BurstDuration =
    (UINT64) ((CurrentState->GetEndTime() - CurrentState->GetBeginTime()) * TimeFactor);
}
