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

#include "TRFDataExtractor.hpp"

#include <cstring>
#include <cerrno>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

#include <iostream>
using std::cout;
using std::endl;

TRFDataExtractor::TRFDataExtractor(string InputTraceName)
:DataExtractor(InputTraceName)
{
  struct stat FileStat;

  if (GetError())
    return;

  if (fstat(fileno(InputTraceFile), &FileStat) < 0)
  {
    SetError(true);
    SetErrorMessage("Error reading input trace file statistics",
                    strerror(errno));
    return;
  }
  this->InputTraceFileSize = FileStat.st_size;

  return;
}


TRFDataExtractor::~TRFDataExtractor()
{
  unlink(TraceDataFileName.c_str());
}

bool TRFDataExtractor::SetEventsToDealWith(set<event_type_t>& EventsToDealWith,
                                           bool               ConsecutiveEvts)
{
  SetError(true);
  SetErrorMessage("TRF traces do not permit parsing based on events");

  return false;
}

bool
TRFDataExtractor::ExtractData(TraceData* TraceDataSet)
{
  char   Buffer[256];
  size_t BufferSize = sizeof(Buffer);

  bool          OngoingBurst, InIdleBlock;
  task_id_t     TaskId, LastTaskId = 0;
  thread_id_t   ThreadId, LastThreadId = 0;
  INT32         FirstsBursts = -1;
  event_type_t  CurrentEventType;
  event_value_t CurrentEventValue;
  double        ReadBurstDuration;
  duration_t    LastBurstDuration;
  line_t        CurrentLine, LastBurstLine;

  map<event_type_t, event_value_t> EventsData;
  map<event_type_t, event_value_t>::iterator EventsDataIterator;



  percentage_t  CurrentPercentage = 0;

  /*
  if (DataManager == NULL)
  {
    SetError(true);
    SetErrorMessage("data manager must be defined to extract data");
    return false;
  }
  */

  if (fseeko(this->InputTraceFile, 0, SEEK_SET) == -1)
  {
    SetError(true);
    SetErrorMessage("unable to seek initial position on input trace file",
                    strerror(errno));
    return false;
  }

  system_messages::show_percentage_progress("Parsing Dimemas Input Trace",
                                            CurrentPercentage);
  CurrentLine = 0;
  EventsData.clear();
  InIdleBlock  = false;
  OngoingBurst = false;

  while (true)
  {
    INT32 PercentageRead;

    if (fgets(Buffer, BufferSize, this->InputTraceFile) == NULL)
    { /* End Of File reachead (or error...) */
      break;
    }
    CurrentLine++;

    /* Idle block checks */
    if (sscanf(Buffer,
               "\"block begin\" { %d, %d, 0 };;\n",
               &TaskId,
               &ThreadId) == 2)
    {
      InIdleBlock = true;
    }

    if (sscanf(Buffer,
               "\"block end\" { %d, %d, 0 };;\n",
               &TaskId,
               &ThreadId) == 2)
    {
      InIdleBlock = false;
    }

    if (sscanf(Buffer,
               "\"CPU burst\" { %d, %d, %le };;\n",
               &TaskId,
               &ThreadId,
               &ReadBurstDuration) == 3)
    {
      if (OngoingBurst)
      { /* Store current burst information */
        if (!TraceDataSet->NewBurst(TaskId,
                                    ThreadId,
                                    LastBurstLine,
                                    0,
                                    0,
                                    LastBurstDuration,
                                    EventsData))
        {
          SetError(true);
          SetErrorMessage("error storing burst data",
                          TraceDataSet->GetLastError());
          return false;
        }
        /* DEBUG
        cout << "Current Point (" << LastBurstLine << " - " << LastBurstDuration << "): ";

        for (EventsDataIterator  = EventsData.begin();
             EventsDataIterator != EventsData.end();
             EventsDataIterator++)
        {
          cout << "\"" << EventsDataIterator->first << "\" = " << EventsDataIterator->second << " ";
        }
        cout << endl;
        */

        /* Reset variables */
        EventsData.clear();
        OngoingBurst = false;
      }

      if (FirstsBursts < TaskId)
      {
        FirstsBursts++;
      }

      if (!InIdleBlock)
      { /* Save current 'CPU burst' information */
        LastBurstLine     = CurrentLine;
        LastBurstDuration = (UINT64) llround(ReadBurstDuration*1e9);
        LastTaskId        = TaskId;
        LastThreadId      = ThreadId;
        OngoingBurst      = true;

        /* Process next trace line */
        continue;
      }
    }

    if (OngoingBurst)
    {
      if (sscanf(Buffer,
                 "\"user event\" { %d, %d, %d, %lld };;\n",
                 &TaskId,
                 &ThreadId,
                 &CurrentEventType,
                 &CurrentEventValue) == 4)
      {
        /* Avoid repeating events */
        EventsDataIterator = EventsData.find(CurrentEventType);

        if (EventsDataIterator == EventsData.end())
        { /* Add this event to 'EventsData' map */
          EventsData[CurrentEventType] = CurrentEventValue;
        }

        /* Process next trace line */
        continue;
      }

      /* Check if a communication primitive is read */
      if ((sscanf(Buffer, "\"NX send\" { %d, %d", &TaskId, &ThreadId) == 2) ||
          (sscanf(Buffer, "\"NX recv\" { %d, %d", &TaskId, &ThreadId) == 2) ||
          (sscanf(Buffer, "\"global OP\" { %d, %d", &TaskId, &ThreadId) == 2))
      {
        if (!TraceDataSet->NewBurst(TaskId,
                                    ThreadId,
                                    LastBurstLine,
                                    0,
                                    0,
                                    LastBurstDuration,
                                    EventsData))
        {
          SetError(true);
          SetErrorMessage("error storing burst data",
                          TraceDataSet->GetLastError());
          return false;
        }

        /* DEBUG
        cout << "Current Point (" << LastBurstLine << " - " << LastBurstDuration << "): ";

        for (EventsDataIterator  = EventsData.begin();
             EventsDataIterator != EventsData.end();
             EventsDataIterator++)
        {
          cout << "\"" << EventsDataIterator->first << "\" = " << EventsDataIterator->second << " ";
        }
        cout << endl;
        */

        /* Reset variables */
        EventsData.clear();
        OngoingBurst = false;
      }
    }

    /* Show progress */
    PercentageRead = GetInputTraceFilePercentage();
    if (PercentageRead > CurrentPercentage)
    {
      CurrentPercentage = PercentageRead;
      system_messages::show_percentage_progress("Parsing Dimemas Input Trace",
                                                CurrentPercentage);
    }
  }

  system_messages::show_percentage_end("Parsing Dimemas Input Trace");

  if (ferror(InputTraceFile) != 0)
  {
    SetError(true);
    SetErrorMessage("error reading input trace file", strerror(errno));
    return false;
  }
  else
  { /* Check if there is a pendig burst */
    if (OngoingBurst)
    {
      if (!TraceDataSet->NewBurst(TaskId,
                                  ThreadId,
                                  LastBurstLine,
                                  0,
                                  0,
                                  LastBurstDuration,
                                  EventsData))
      {
        SetError(true);
        SetErrorMessage("error storing burst data", TraceDataSet->GetLastError());
        return false;
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

  return true;
}
