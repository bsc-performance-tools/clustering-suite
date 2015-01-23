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

#ifndef _ClusteredStatesPRVGenerator_HPP_
#define _ClusteredStatesPRVGenerator_HPP_


#include <ParaverTraceParser.hpp>
#include "ClusteredTraceGenerator.hpp"
#include "CPUBurst.hpp"

#include <cstring>
#include <cerrno>

#include <vector>
using std::vector;
#include <map>
using std::map;
#include <list>
using std::list;
#include <sstream>
using std::ostringstream;

class ParaverTraceParser;
class Event;

class ClusteredStatesPRVGenerator: public ClusteredTraceGenerator
{
  public:
    struct BurstInfo
    {
      timestamp_t  BeginTime;
      timestamp_t  EndTime;
      cluster_id_t ID;
      bool         InUse;
    };
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

    map<string, list<BurstInfo*> > BurstsToPrint;

  public:
    ClusteredStatesPRVGenerator (string  InputTraceName,
                                string  OutputTraceName);

    ~ClusteredStatesPRVGenerator (void);

    ReconstructorType GetType(void) { return SemanticGuidedPRV; };

    bool SetEventsToDealWith (set<event_type_t>& EventsToDealWith,
                              bool               ConsecutiveEvts);

    bool Run (vector<CPUBurst*>&    Bursts,
              vector<cluster_id_t>& IDs,
              set<cluster_id_t>&    DifferentIDs,
              bool                  PrintOnlyEvents,
              bool                  DoNotPrintFilteredBursts);

    template <typename T>
    bool Run (T                     begin,
              T                     end,
              vector<cluster_id_t>& IDs,
              set<cluster_id_t>&    DifferentIDs,
              bool                  PrintOnlyEvents,
              bool                  DoNotPrintFilteredBursts);

  private:
    bool GenerateOutputPCF (set<cluster_id_t>& DifferentIDs);

    bool BurstCloseAndOpenRecord (ParaverRecord* CurrentRecord, cluster_id_t& ID);

    bool BurstOpeningRecord (ParaverRecord* CurrentRecord, cluster_id_t& ID);

    bool BurstClosingRecord (ParaverRecord* CurrentRecord);

    bool DuplicatedOpening(string                                              TraceObject,
                           map<string, std::pair<timestamp_t, cluster_id_t> >& LastBurstsPrinted,
                           timestamp_t                                         CurrentTimestamp,
                           cluster_id_t                                        CurrentID);

    bool CopyROWFile();

    void PrepareClusterIDsVector (vector<cluster_id_t>& ClusterIDs,
                                  set<cluster_id_t>&    DifferentIDs,
                                  cluster_id_t&         MaxIDUsed);

    string GetClusterName (cluster_id_t ID);
};

template <typename T>
bool ClusteredStatesPRVGenerator::Run (T                     begin,
                                       T                     end,
                                       vector<cluster_id_t>& IDs,
                                       set<cluster_id_t>&    DifferentIDs,
                                       bool                  PrintOnlyEvents,
                                       bool                  DoNotPrintFilteredBursts)
{
  ParaverHeader*                  Header;
  vector<ApplicationDescription*> AppsDescription;

  ParaverRecord                  *CurrentRecord;
  Event                          *CurrentEvent;
  percentage_t                    CurrentPercentage = 0;

  size_t                          TotalBursts;
  double                          TimeFactor;

  /* struct to avoid duplicated open events */
  map<string, std::pair<timestamp_t, cluster_id_t> > LastBurstsPrinted;


  /* Create the map with the bursts information IDs vector */
  size_t CurrentClusteringBurst = 0;

  for (T Burst = begin; Burst != end; ++Burst)
  {
    cluster_id_t  CurrentID;
    ostringstream TraceObject;
    BurstInfo*    NewBurstInfo = new BurstInfo();



    switch ( (*Burst)->GetBurstType() )
    {
      case CompleteBurst:
        CurrentID = IDs[CurrentClusteringBurst];
        CurrentClusteringBurst++;
        break;
      case DurationFilteredBurst:
        CurrentID = DURATION_FILTERED_CLUSTERID;
        break;
      case RangeFilteredBurst:
        CurrentID = RANGE_FILTERED_CLUSTERID;
        break;
      case MissingDataBurst:
        CurrentID = MISSING_DATA_CLUSTERID;
        break;
      default:
        /* This option should never happen */
        SetErrorMessage ("incorrect burst type when generating Paraver trace");
        SetError (false);
        return false;
    }

    if (!DoNotPrintFilteredBursts ||
         (CurrentID != DURATION_FILTERED_CLUSTERID &&
          CurrentID != RANGE_FILTERED_CLUSTERID    &&
          CurrentID != MISSING_DATA_CLUSTERID))
    {
      NewBurstInfo = new BurstInfo();

      NewBurstInfo->BeginTime = (*Burst)->GetBeginTime();
      NewBurstInfo->EndTime   = (*Burst)->GetEndTime();
      NewBurstInfo->InUse     = false;
      NewBurstInfo->ID        = CurrentID + PARAVER_OFFSET;

      TraceObject << (*Burst)->GetTaskId() << "." << (*Burst)->GetThreadId();
      if ( BurstsToPrint.find(TraceObject.str()) == BurstsToPrint.end())
      {
        BurstsToPrint.insert(make_pair(TraceObject.str(), list<BurstInfo*>()));
      }

      BurstsToPrint[TraceObject.str()].push_back(NewBurstInfo);
    }
  }

  /* Sort all bursts in terms of trace appearance, using the line comparison */
  // sort (Bursts.begin(), Bursts.end(), LineCompare() );

  /* DEBUG
  for (size_t i = 0; i < Bursts.size(); i++)
  {
    cout << "Burst Line = " << Bursts[i]->GetLine() << " ";
    cout << "Instance = " << Bursts[i]->GetInstance () << " ";
    cout << "BeginTime = " << Bursts[i]->GetBeginTime() << " ";
    cout << "EndTime = " << Bursts[i]->GetEndTime() << " ";
    cout << "ID = " << CompleteIDs[Bursts[i]->GetInstance()] << endl;
  }
  */

  if (!TraceParser->InitTraceParsing() )
  {
    SetError (true);
    SetErrorMessage ("Error initalizing trace parsing",
                     TraceParser->GetLastError() );
    return false;
  }

  /* Get the header and flush it to output trace */
  Header = TraceParser->GetHeader();

  if (Header != NULL)
  {
    if (!Header->Flush (OutputTraceFile) )
    {
      SetError (true);
      SetErrorMessage ("Error creating output trace",
                       Header->GetLastError() );
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
      if (!Communicators[j]->Flush (OutputTraceFile) )
      {
        SetError (true);
        SetErrorMessage ("Error creating output trace",
                         Communicators[j]->GetLastError() );
        return false;
      }
    }

    /* This is buggy, because we need to capture a multi-application trace */
    vector<TaskDescription_t> TasksInfo = AppsDescription[i]->GetTaskInfo();
  }

  if (!TraceParser->FlushComments(OutputTraceFile))
  {
    SetError (true);
    SetErrorMessage ("Error creating output trace",
                     Header->GetLastError() );
    return false;
  }

  /* Add clusters information to trace */
  /* BeginTimeIndex = EndTimeIndex = FilterBurstsEndIndex = 0; */
  TraceParser->Reload();

  if (TraceParser->GetTimeUnits() == MICROSECONDS)
    TimeFactor = 1e3;
  else
    TimeFactor = 1;

  CurrentPercentage = TraceParser->GetFilePercentage();
  system_messages::show_percentage_progress ("Generation Paraver Output Trace",
      CurrentPercentage);


  CurrentRecord = TraceParser->GetNextRecord();

  while (CurrentRecord != NULL)
  {
    percentage_t PercentageRead;
    bool  CurrentRecordFlushed = false;

    /* We are only interested on event and state records */
    cluster_id_t ID;

    if (BurstCloseAndOpenRecord(CurrentRecord, ID))
    { /* Here we close the previous region and open the following at the
       * the same point */
      Event* NewEvent;
      ostringstream TraceObject;

      TraceObject << CurrentRecord->GetTaskId() << "." << CurrentRecord->GetThreadId();

      NewEvent = new Event (0, /* Line not needed */
                            CurrentRecord->GetTimestamp(),
                            CurrentRecord->GetCPU() + 1,
                            CurrentRecord->GetAppId() + 1,
                            CurrentRecord->GetTaskId() + 1,
                            CurrentRecord->GetThreadId() + 1);

      NewEvent->AddTypeValue (90000001, 0);  // Close
      NewEvent->AddTypeValue (90000001, ID); // and Open... :)

      if (!NewEvent->Flush (OutputTraceFile) )
      {
        SetError (true);
        SetErrorMessage ("Error creating output trace",
                         NewEvent->GetLastError() );
        return false;
      }

      LastBurstsPrinted[TraceObject.str()] = std::make_pair(CurrentRecord->GetTimestamp(),
                                                             ID);


      delete NewEvent;
    }
    else if (BurstOpeningRecord(CurrentRecord, ID) )
    {
      Event*        NewEvent;
      ostringstream TraceObject;

      TraceObject << CurrentRecord->GetTaskId() << "." << CurrentRecord->GetThreadId();

      if (!DuplicatedOpening(TraceObject.str(),
                             LastBurstsPrinted,
                             CurrentRecord->GetTimestamp(),
                             ID))
      {
        NewEvent = new Event (0, /* Line not needed */
                              CurrentRecord->GetTimestamp(),
                              CurrentRecord->GetCPU() + 1,
                              CurrentRecord->GetAppId() + 1,
                              CurrentRecord->GetTaskId() + 1,
                              CurrentRecord->GetThreadId() + 1);


        NewEvent->AddTypeValue (90000001, ID);

        if (!NewEvent->Flush (OutputTraceFile) )
        {
          SetError (true);
          SetErrorMessage ("Error creating output trace",
                           NewEvent->GetLastError() );
          return false;
        }

        LastBurstsPrinted[TraceObject.str()] = std::make_pair(CurrentRecord->GetTimestamp(),
                                                              ID);

        delete NewEvent;
      }
    }
    else if (BurstClosingRecord(CurrentRecord))
    {
      Event* NewEvent;

      NewEvent = new Event (0, /* Line not needed */
                            CurrentRecord->GetTimestamp(),
                            CurrentRecord->GetCPU() + 1,
                            CurrentRecord->GetAppId() + 1,
                            CurrentRecord->GetTaskId() + 1,
                            CurrentRecord->GetThreadId() + 1);

      NewEvent->AddTypeValue (90000001, 0);

      if (!NewEvent->Flush (OutputTraceFile) )
      {
        SetError (true);
        SetErrorMessage ("Error creating output trace",
                         NewEvent->GetLastError() );
        return false;
      }

      delete NewEvent;
    }


    /* Show progress */
    PercentageRead = TraceParser->GetFilePercentage();

    if (PercentageRead > CurrentPercentage)
    {
      CurrentPercentage = PercentageRead;
      system_messages::show_percentage_progress ("Generating Paraver Output Trace",
          CurrentPercentage);
    }

    /* If 'MinimizeInformation' is active, general records are not flushed */
    if (!CurrentRecordFlushed && !PrintOnlyEvents)
    {
      if (!CurrentRecord->Flush (OutputTraceFile) )
      {
        SetError (true);
        SetErrorMessage ("Error creating output trace",
                         CurrentEvent->GetLastError() );
        return false;
      }
    }

    delete CurrentRecord;
    CurrentRecord = TraceParser->GetNextRecord();
  }

  /* DEBUG
  cout << "Points used = " << BeginTimeIndex << endl;
  */

  if (TraceParser->GetError() )
  {
    SetError (true);
    SetErrorMessage ("Error creating output trace ", TraceParser->GetLastError() );
    return false;
  }

  if (ferror (InputTraceFile) != 0)
  {
    SetError (true);
    SetErrorMessage ("Error creating output trace", strerror (errno) );
    return false;
  }

  system_messages::show_percentage_end ("Generating Paraver Output Trace");

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
    GenerateOutputPCF (DifferentIDs);
  }

  if (ROWPresent)
  {
    CopyROWFile();
  }

  delete Header;

  return true;
}

#endif

