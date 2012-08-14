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

#ifndef _CLUSTEREDSTATESPRVGENERATOR_HPP_
#define _CLUSTEREDSTATESPRVGENERATOR_HPP_

#include <ParaverTraceParser.hpp>

#include "ClusteredTraceGenerator.hpp"
#include "CPUBurst.hpp"

#include <cstring>
#include <cerrno>
#include <vector>
using std::vector;

class ParaverTraceParser;

#define RUNNING_STATE 1

class ClusteredStatesPRVGenerator: public ClusteredTraceGenerator
{
  private:
    ParaverTraceParser *TraceParser;

    bool   PCFPresent;
    string InputPCFName;
    FILE  *InputPCFFile;
    string OutputPCFName;
    FILE  *OutputPCFFile;

    bool   ROWPresent;
    string InputROWName;
    string OutputROWName;

    vector<CPUBurst*> BurstsBeginTime;

  public:
    ClusteredStatesPRVGenerator(string  InputTraceName,
                          string  OutputTraceName);

    ~ClusteredStatesPRVGenerator(void);

    ReconstructorType GetType(void) { return PRVStates; };

    bool SetEventsToDealWith (set<event_type_t>& EventsToDealWith);

    template <typename T>
    bool Run (T                     begin,
              T                     end,
              vector<cluster_id_t>& IDs,
              set<cluster_id_t>&    DifferentIDs,
              bool                  MinimizeInformation = false);

    bool Run(vector<CPUBurst*>&    Bursts,
             vector<cluster_id_t>& IDs,
             set<cluster_id_t>&    DifferentIDs,
             bool                  MinimizeInformation = false);

  private:

    bool GenerateOutputPCF(set<cluster_id_t>& DifferentIDs);

    bool CopyROWFile(void);

    void PrepareClusterIDsVector(vector<cluster_id_t>& ClusterIDs,
                                 set<cluster_id_t>&    DifferentIDs,
                                 cluster_id_t&         MaxIDUsed);

    string GetClusterName(cluster_id_t ID);

};

template <typename T>
bool ClusteredStatesPRVGenerator::Run (T                     begin,
                                       T                     end,
                                       vector<cluster_id_t>& IDs,
                                       set<cluster_id_t>&    DifferentIDs,
                                       bool                  MinimizeInformation)
{
  ParaverHeader*                  Header;
  vector<ApplicationDescription*> AppsDescription;
  timestamp_t                     BeginTimeIndex;


  ParaverRecord                *CurrentRecord;
  Event                        *CurrentEvent;
  State                        *CurrentState;
  INT32                         CurrentPercentage = 0;
  vector<vector<timestamp_t> >  BurstsEnd;
  size_t                        FilterBurstsEndIndex;
  size_t                        TotalBursts;
  double                        TimeFactor;

  /* This map contains all the information required to the reconstruction.
     It is indexed by line to guarantee the trace ordering */
  map<line_t, std::pair<cluster_id_t, timestamp_t> >           CompleteIDs;
  /* Iterator values:

     + 'first'         : the line (not use, just for the map ordering)

     + 'second.first'  : the Cluster ID associated to the region (in this case
                          the parser is an events parser, not burst parser)

     + 'second.second' : the End Time of the current region
   */
  map<line_t, std::pair<cluster_id_t, timestamp_t> >::iterator BurstsInfo;

  if (fseeko(this->InputTraceFile, 0, SEEK_SET) == -1)
  {
    SetError(true);
    SetErrorMessage("unable to seek initial position on input trace file",
                    strerror(errno));
    return false;
  }

    if (!TraceParser->InitTraceParsing())
  {
    SetError(true);
    SetErrorMessage("Error initalizing trace parsing",
                    TraceParser->GetLastError());
    return false;
  }

  /* Get the header and flush it to output trace */
  Header = TraceParser->GetHeader();

  if (Header != NULL)
  {
    if (!Header->Flush(OutputTraceFile))
    {
      SetError(true);
      SetErrorMessage("Error creating output trace",
                      Header->GetLastError());
      return false;
    }
  }

  /* Print all communicators */
  AppsDescription = TraceParser->GetApplicationsDescription();

  for (size_t i = 0; i < AppsDescription.size(); i++)
  {
    vector<Communicator*> Communicators;

    Communicators = AppsDescription[i]->GetCommunicators();

    for (size_t j = 0; j < Communicators.size(); j++)
    {
      if (!Communicators[j]->Flush(OutputTraceFile))
      {
        SetError(true);
        SetErrorMessage("Error creating output trace",
                        Communicators[j]->GetLastError());
        return false;
      }
    }

    /* This is buggy, because we need to capture a multi-application trace */
    vector<TaskDescription_t> TasksInfo = AppsDescription[i]->GetTaskInfo();

    for (size_t j = 0; j < TasksInfo.size(); j++)
    {
      BurstsEnd.push_back(vector<timestamp_t> (TasksInfo[j]->GetThreadCount(), 0));
    }
  }

  /* Create the single cluster IDs map */
  size_t CurrentClusteringBurst = 0;

  for (T Burst = begin; Burst != end; ++Burst)
  {
    switch ( (*Burst)->GetBurstType() )
    {
      case CompleteBurst:
        // CompleteIDs.push_back (IDs[CurrentClusteringBurst] + PARAVER_OFFSET);
        CompleteIDs[ (*Burst)->GetLine() ] =
          std::make_pair (IDs[CurrentClusteringBurst] + PARAVER_OFFSET,
                          (*Burst)->GetEndTime());
        CurrentClusteringBurst++;
        break;
      case DurationFilteredBurst:
        // CompleteIDs.push_back (DURATION_FILTERED_CLUSTERID + PARAVER_OFFSET);
        CompleteIDs[ (*Burst)->GetLine() ] =
          std::make_pair (DURATION_FILTERED_CLUSTERID + PARAVER_OFFSET,
                          (*Burst)->GetEndTime());
        break;
      case RangeFilteredBurst:
        // CompleteIDs.push_back (RANGE_FILTERED_CLUSTERID + PARAVER_OFFSET);
        CompleteIDs[ (*Burst)->GetLine() ] =
          std::make_pair (RANGE_FILTERED_CLUSTERID + PARAVER_OFFSET,
                          (*Burst)->GetEndTime());
        break;
      case MissingDataBurst:
        // CompleteIDs.push_back (MISSING_DATA_CLUSTERID + PARAVER_OFFSET);
        CompleteIDs[ (*Burst)->GetLine() ] =
          std::make_pair (MISSING_DATA_CLUSTERID + PARAVER_OFFSET,
                          (*Burst)->GetEndTime());
        break;
      default:
        /* This option should never happen */
        SetErrorMessage ("incorrect burst type when generating Paraver trace");
        SetError (false);
        return false;
    }
#ifdef HAVE_SQLITE3
    delete (*Burst);
#endif
  }

  /* Add clusters information to trace */
  /* BeginTimeIndex = EndTimeIndex = FilterBurstsEndIndex = 0; */
  BeginTimeIndex = 0;

  TraceParser->Reload();

  if (TraceParser->GetTimeUnits() == MICROSECONDS)
    TimeFactor = 1e3;
  else
    TimeFactor = 1;

  CurrentPercentage = TraceParser->GetFilePercentage();
  system_messages::show_percentage_progress("Generation Paraver Output Trace",
                                            CurrentPercentage);

  CurrentRecord = TraceParser->GetNextRecord();
  BurstsInfo    = CompleteIDs.begin();

  while (CurrentRecord != NULL)
  {
    percentage_t PercentageRead;
    bool         CurrentRecordFlushed = false;

    /* We are only interested on event and state records */

    if (CurrentRecord->GetRecordType() == PARAVER_STATE)
    {
      CurrentState = (State*) CurrentRecord;

      if (CurrentState->GetStateValue() == RUNNING_STATE)
      {
        /* Check if there is a pending cluster region to close */
        if (BurstsEnd[CurrentState->GetTaskId()][CurrentState->GetThreadId()] == CurrentState->GetBeginTime()
        &&  BurstsEnd[CurrentState->GetTaskId()][CurrentState->GetThreadId()] != 0)
        {
          Event* NewEvent;

          NewEvent = new Event(0, /* Line not needed */
                               CurrentState->GetTimestamp(),
                               CurrentState->GetCPU()+1,
                               CurrentState->GetAppId()+1,
                               CurrentState->GetTaskId()+1,
                               CurrentState->GetThreadId()+1);

          NewEvent->AddTypeValue(90000001,0);

#ifdef DEBUG_PRV_OUTPUT
          cout << "Flushing closing cluster event: " << *NewEvent;
#endif

          if (!NewEvent->Flush(OutputTraceFile))
          {
            SetError(true);
            SetErrorMessage("Error creating output trace",
                            NewEvent->GetLastError());
            return false;
          }

          delete NewEvent;

          BurstsEnd[CurrentState->GetTaskId()][CurrentState->GetThreadId()] = 0;
        }

        /* DEBUG
        cout << "BeginTimeIndex = " << BeginTimeIndex << " - ";
        cout << "BeginTimePoints.size() = " << BeginTimePoints.size() << endl; */

        if (BurstsInfo != CompleteIDs.end())
        {
          Event* NewEvent;

          NewEvent = new Event(0, /* Line not needed */
                               CurrentState->GetTimestamp(),
                               CurrentState->GetCPU()+1,
                               CurrentState->GetAppId()+1,
                               CurrentState->GetTaskId()+1,
                               CurrentState->GetThreadId()+1);

          NewEvent->AddTypeValue(90000001,
                                 (INT64) (*BurstsInfo).second.first);

#ifdef DEBUG_PRV_OUTPUT
          cout << "Flushing opening cluster event: " << *NewEvent;

          if (CompleteIDs[CurrentBurst->GetInstance()] > 20)
          {
            cout << "Strange cluster id. Instance = " << CurrentBurst->GetInstance() << endl;
          }
#endif

          if (!NewEvent->Flush(OutputTraceFile))
          {
            SetError(true);
            SetErrorMessage("Error creating output trace",
                            NewEvent->GetLastError());
            return false;
          }

          delete NewEvent;

          /* Set the end time */
          BurstsEnd[CurrentState->GetTaskId()][CurrentState->GetThreadId()] =
            (*BurstsInfo).second.second;

          ++BurstsInfo;

        }

        /*
        if (BeginTimeIndex < Bursts.size())
        {
          CPUBurst* CurrentBurst = Bursts[BeginTimeIndex];

          if (CurrentState->GetBeginTime() == CurrentBurst->GetBeginTime() &&
              CurrentState->GetTaskId()    == CurrentBurst->GetTaskId() &&
              CurrentState->GetThreadId()  == CurrentBurst->GetThreadId())
          {
            Event* NewEvent;

            NewEvent = new Event(0, /* Line not needed
                                 CurrentState->GetTimestamp(),
                                 CurrentState->GetCPU()+1,
                                 CurrentState->GetAppId()+1,
                                 CurrentState->GetTaskId()+1,
                                 CurrentState->GetThreadId()+1);

            NewEvent->AddTypeValue(90000001,
                                   (INT64) CompleteIDs[CurrentBurst->GetInstance()]);

#ifdef DEBUG_PRV_OUTPUT
            cout << "Flushing opening cluster event: " << *NewEvent;

            if (CompleteIDs[CurrentBurst->GetInstance()] > 20)
            {
              cout << "Strange cluster id. Instance = " << CurrentBurst->GetInstance() << endl;
            }
#endif

            if (!NewEvent->Flush(OutputTraceFile))
            {
              SetError(true);
              SetErrorMessage("Error creating output trace",
                              NewEvent->GetLastError());
              return false;
            }

            delete NewEvent;

            BeginTimeIndex++;

            /* Set the end time
            BurstsEnd[CurrentState->GetTaskId()][CurrentState->GetThreadId()] =
              CurrentBurst->GetEndTime();
          }
        }
        */
      }
    }
    else if (CurrentRecord->GetRecordType() == PARAVER_EVENT)
    {
      CurrentEvent = (Event*) CurrentRecord;

      if (BurstsEnd[CurrentEvent->GetTaskId()][CurrentEvent->GetThreadId()] == CurrentEvent->GetTimestamp()
      &&  BurstsEnd[CurrentEvent->GetTaskId()][CurrentEvent->GetThreadId()] != 0) // In timestamp 0 it is impossible to close a burst
      {
        Event* NewEvent;

        NewEvent = new Event(0, /* Line not needed */
                             CurrentEvent->GetTimestamp(),
                             CurrentEvent->GetCPU()+1,
                             CurrentEvent->GetAppId()+1,
                             CurrentEvent->GetTaskId()+1,
                             CurrentEvent->GetThreadId()+1);

        NewEvent->AddTypeValue(90000001,0);

        if (!NewEvent->Flush(OutputTraceFile))
        {
          SetError(true);
          SetErrorMessage("Error creating output trace",
                          NewEvent->GetLastError());
          return false;
        }

        delete NewEvent;

#ifdef DEBUG_PRV_OUTPUT
        cout << "Flush closing cluster event [Filtered]: " << *NewEvent;
#endif

        /* FilterBurstsEnd.pop_front(); */
        BurstsEnd[CurrentEvent->GetTaskId()][CurrentEvent->GetThreadId()] = 0;
      }

      /* Show progress */
      PercentageRead = TraceParser->GetFilePercentage();
      if (PercentageRead > CurrentPercentage)
      {
        CurrentPercentage = PercentageRead;
        system_messages::show_percentage_progress("Generating Paraver Output Trace",
                                                  CurrentPercentage);
      }
    }

    /* If 'MinimizeInformation' is active, general records are not flushed */
    if (!CurrentRecordFlushed && !MinimizeInformation)
    {
      if (!CurrentRecord->Flush(OutputTraceFile))
      {
        SetError(true);
        SetErrorMessage("Error creating output trace",
                        CurrentEvent->GetLastError());
        return false;
      }
    }

    delete CurrentRecord;

    CurrentRecord = TraceParser->GetNextRecord();
  }

  /* DEBUG
  cout << "Points used = " << BeginTimeIndex << endl; */

  if (TraceParser->GetError())
  {
    SetError(true);
    SetErrorMessage("Error creating output trace ", TraceParser->GetLastError());
    return false;
  }

  if (ferror(InputTraceFile) != 0)
  {
    SetError(true);
    SetErrorMessage("Error creating output trace", strerror(errno));
    return false;
  }

  system_messages::show_percentage_end("Generating Paraver Output Trace");

  /* DEBUG
  printf("BeginTimeIndex = %d, EndTimeIndex = %d FilterBurstsEndIndex = %d\n",
         BeginTimeIndex,
         EndTimeIndex,
         FilterBurstsEndIndex);
  printf("BeginTime.size = %d EndTime.size = %d FilterBursts.size = %d\n",
         BeginTimePoints.size(),
         EndTimePoints.size(),
         FilterBurstsEnd.size());
  */

  if (PCFPresent)
  {
    GenerateOutputPCF(DifferentIDs);
  }

  if (ROWPresent)
  {
    CopyROWFile();
  }

  delete Header;

  return true;
}

#endif

