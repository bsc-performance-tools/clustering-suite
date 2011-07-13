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

#include "ClusteredEventsPRVGenerator.hpp"
#include <ParaverTraceParser.hpp>
#include <ParaverColors.h>

#include <unistd.h>
#include <cstring>

#include <algorithm>
using std::sort;
using std::merge;

#include <iostream>
#include <fstream>
#include <sstream>
using std::ifstream;
using std::ofstream;
using std::ostringstream;

// #define DEBUG_PRV_OUTPUT 0

#define RUNNING_STATE 1
#define CLUSTERS_EVENTS_TXT \
"\n"\
"EVENT_TYPE\n"\
"9\t90000001\tCluster ID\n"\
"VALUES\n"\
"0\tEnd\n"

ClusteredEventsPRVGenerator::ClusteredEventsPRVGenerator(string  InputTraceName,
                                                         string  OutputTraceName)
:ClusteredTraceGenerator(InputTraceName, OutputTraceName)
{
  string::size_type SubstrPos;
  
  if (GetError())
    return;

  TraceParser = new ParaverTraceParser(InputTraceName, InputTraceFile);
  
  /* Search for a possible PCF and ROW files */
  PCFPresent = false;
  ROWPresent = false;
  
  /* Check the PCF/ROW file name */
  SubstrPos = InputTraceName.rfind(".prv");
  
  if (SubstrPos == string::npos)
  {
    InputPCFName = InputTraceName+".pcf";
    InputROWName = InputTraceName+".row";
  }
  else
  {
    string InputBaseName;
    
    InputBaseName = InputTraceName.substr(0, SubstrPos);
    InputPCFName  = InputBaseName+".pcf";
    InputROWName  = InputBaseName+".row";
  }
  
  if ((InputPCFFile = fopen(InputPCFName.c_str(), "r" )) != NULL)
  {
    PCFPresent = true;
    /* Create output PCF file */
    
    SubstrPos = OutputTraceName.rfind(".prv");
  
    if (SubstrPos == string::npos)
    {
      OutputPCFName = InputTraceName+".pcf";
    }
    else
    {
      string OutputBaseName;
      
      OutputBaseName = OutputTraceName.substr(0, SubstrPos);
      OutputPCFName  = OutputBaseName+".pcf";
    }
    
    if ((OutputPCFFile = fopen(OutputPCFName.c_str(), "w")) == NULL)
    {
      /* We are unable to open the output PCF file. The PCF managment is
         disabled */
      fclose(InputPCFFile);
      PCFPresent = false;
    }
  }

  FILE* InputROWFile, *OutputROWFile;
  if ((InputROWFile = fopen(InputROWName.c_str(), "r" )) != NULL)
  {
    ROWPresent = true;

    SubstrPos = OutputTraceName.rfind(".prv");
  
    if (SubstrPos == string::npos)
    {
      OutputROWName = InputTraceName+".row";
    }
    else
    {
      string OutputBaseName;
      
      OutputBaseName = OutputTraceName.substr(0, SubstrPos);
      OutputROWName  = OutputBaseName+".row";
    }
    
    if ((OutputROWFile = fopen(OutputROWName.c_str(), "w")) == NULL)
    {
      /* We are unable to open the output PCF file. The PCF managment is
         disabled */
      ROWPresent = false;
    }
  }
  
  /* DEBUG */
  if (PCFPresent)
  {
    printf("PCF FOUND!!!!\n");
  }

  if (ROWPresent)
  {
    printf("ROW FOUND!!!!\n");
  }

  printf("Input PCF = %s\nOutput PCF = %s\n",
           InputPCFName.c_str(),
           OutputPCFName.c_str());

  printf("Input ROW = %s\nOutput ROW = %s\n",
           InputROWName.c_str(),
           OutputROWName.c_str());
  
  return;
}

bool ClusteredEventsPRVGenerator::SetEventsToDealWith (set<event_type_t>& EventsToDealWith)
{
  this->EventsToDealWith = EventsToDealWith;

  if (EventsToDealWith.size() == 0)
  {
    SetErrorMessage("no events definend in a PRV event parsing generator");
    SetError(true);
    
    return false;
  }

  return true;
}


bool ClusteredEventsPRVGenerator::Run(vector<CPUBurst*>&    Bursts,
                                      vector<cluster_id_t>& IDs,
                                      set<cluster_id_t>&    DifferentIDs,
                                      bool                  MinimizeInformation)
{
  ParaverHeader*                  Header;
  vector<ApplicationDescription*> AppsDescription;
  timestamp_t                     BeginTimeIndex;
  vector<cluster_id_t>            CompleteIDs;
                    
  ParaverRecord                *CurrentRecord;
  Event                        *CurrentEvent;
  percentage_t                  CurrentPercentage = 0;
  vector<vector<timestamp_t> >  BurstsEnd;
  size_t                        FilterBurstsEndIndex;
  size_t                        TotalBursts;
  double                        TimeFactor;

  if (EventsToDealWith.size() == 0)
  {
    SetErrorMessage("no event types set in the trace reconstruction");
    SetError(true);
    return false;
  }
  
  /* Sort all burst in terms of instance number, to guarantee the positional
     assignment in the IDs vector */
  sort(Bursts.begin(), Bursts.end(), InstanceNumCompare());
  
  /* Create the single cluster IDs vector */
  size_t CurrentClusteringBurst = 0;
  for (size_t i = 0; i < Bursts.size(); i++)
  {
    switch(Bursts[i]->GetBurstType())
    {
      case CompleteBurst:
        CompleteIDs.push_back(IDs[CurrentClusteringBurst]+PARAVER_OFFSET);
        CurrentClusteringBurst++;
        break;
      case DurationFilteredBurst:
        CompleteIDs.push_back(DURATION_FILTERED_CLUSTERID+PARAVER_OFFSET);
        break;
      case RangeFilteredBurst:
        CompleteIDs.push_back(RANGE_FILTERED_CLUSTERID+PARAVER_OFFSET);
        break;
      case MissingDataBurst:
        CompleteIDs.push_back(MISSING_DATA_CLUSTERID+PARAVER_OFFSET);
        break;
      default:
        /* This option should never happen */
          SetErrorMessage("incorrect burst type when generating Paraver trace");
          SetError(false);
          return false;
    }
  }
  
  /* Sort all bursts in terms of trace appearance, using the line comparison */
  sort(Bursts.begin(), Bursts.end(), LineCompare());

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
      BurstsEnd.push_back(vector<timestamp_t> (TasksInfo[j]->GetThreadCount()));
    }
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

  while (CurrentRecord != NULL)
  {
    percentage_t PercentageRead;
    bool  CurrentRecordFlushed = false;
    
    /* We are only interested on event and state records */
    if (CurrentRecord->GetRecordType() == PARAVER_EVENT)
    {
      CurrentEvent = (Event*) CurrentRecord;

      if (BurstOpeningEvent (CurrentEvent))
      {
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

          BurstsEnd[CurrentEvent->GetTaskId()][CurrentEvent->GetThreadId()] = 0;
        }

        if (BeginTimeIndex < Bursts.size())
        {
          CPUBurst* CurrentBurst = Bursts[BeginTimeIndex];

          if (CurrentEvent->GetTimestamp() == CurrentBurst->GetBeginTime() &&
              CurrentEvent->GetTaskId()    == CurrentBurst->GetTaskId() &&
              CurrentEvent->GetThreadId()  == CurrentBurst->GetThreadId())
          {
            Event* NewEvent;
            
            NewEvent = new Event(0, /* Line not needed */
                                 CurrentEvent->GetTimestamp(),
                                 CurrentEvent->GetCPU()+1,
                                 CurrentEvent->GetAppId()+1,
                                 CurrentEvent->GetTaskId()+1,
                                 CurrentEvent->GetThreadId()+1);
            
            NewEvent->AddTypeValue(90000001,
                                   (INT64) CompleteIDs[CurrentBurst->GetInstance()]);

            if (!NewEvent->Flush(OutputTraceFile))
            {
              SetError(true);
              SetErrorMessage("Error creating output trace",
                              NewEvent->GetLastError());
              return false;
            }
            
            BeginTimeIndex++;
            
            /* Set the end time */
            BurstsEnd[CurrentEvent->GetTaskId()][CurrentEvent->GetThreadId()] =
              CurrentBurst->GetEndTime();
          }
        }
      }
      else if (BurstClosingEvent (CurrentEvent))
      {
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

          /* FilterBurstsEnd.pop_front(); */
          BurstsEnd[CurrentEvent->GetTaskId()][CurrentEvent->GetThreadId()] = 0;
        }
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
  cout << "Points used = " << BeginTimeIndex << endl;
  */
  
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
  
  return true;
}

bool ClusteredEventsPRVGenerator::GenerateOutputPCF(set<cluster_id_t>& DifferentIDs)
{
  char   Buffer[256], AuxBuffer[256];
  size_t BufferSize = sizeof(Buffer);
  bool   ColorsSectionRead     = false;
  bool   ColorsSectionFinished = false;
  bool   FlushBuffer = false;

  vector<cluster_id_t> ClusterIDs;
  
  cluster_id_t MaxIDUsed;
  PrepareClusterIDsVector(ClusterIDs, DifferentIDs, MaxIDUsed);

  while (true)
  {
    if (fgets(Buffer, BufferSize, InputPCFFile) == NULL)
    { /* End Of File reachead (or error...) */
      break;
    }

    FlushBuffer = false;
    
    if (!ColorsSectionRead)
    {
      if (strncmp(Buffer, "STATES_COLOR", 12) == 0)
      { /* State color section found. Flush common-cluster colors */
        ColorsSectionRead = true;
        
        if (fprintf(OutputPCFFile, "%s", Buffer) < 0)
        {
          /* No error state, because PCF generation is optional */
          unlink(OutputPCFName.c_str());
          return false;
        }
        
        /* Print all state colors up to maximum cluster ID */
        for (size_t i = 0; i < MaxIDUsed+PARAVER_OFFSET; i++)
        {
          if (i >= DEF_NB_COLOR_STATE)
          {
            cout << "Warning! Number of clusters greater than available colors" << endl;
            break;
          }
          if (fprintf(OutputPCFFile,
                      "%d\t{%d,%d,%d}\n",
                      i,
                      ParaverDefaultPalette[i].RGB[0],
                      ParaverDefaultPalette[i].RGB[1],
                      ParaverDefaultPalette[i].RGB[2]) < 0)
          {
            /* No error state, because PCF generation is optional */
            unlink(OutputPCFName.c_str());
            return false;
          }
        }
      }
      else
      {
        FlushBuffer = true;
      }
    }
    else if (!ColorsSectionFinished)
    {
      INT32 Code, R, G, B;
      /* Check if current line is a color definition */
      if (sscanf(Buffer, "%d {%d,%d,%d}\n", &Code, &R, &G, &B) != 4)
      {
        FlushBuffer           = true;
        ColorsSectionFinished = true;
      }
    }
    else
    {
      FlushBuffer = true;
    }
    
    if (FlushBuffer)
    {
      if (fprintf(OutputPCFFile, "%s", Buffer) < 0)
      {
        /* No error state, because PCF generation is optional */
        unlink(OutputPCFName.c_str());
        return false;
      }
    }
  }
  
  
  if (ferror(OutputPCFFile))
  {
    unlink(OutputPCFName.c_str());
    return false;
  }
  
  /* Print clusters information */
  
  if (fprintf(OutputPCFFile, CLUSTERS_EVENTS_TXT) < 0)
  {
    unlink(OutputPCFName.c_str());
    return false;
  }
  
  if (fprintf(OutputPCFFile,
              "") < 0)
  {
    unlink(OutputPCFName.c_str());
    return false;
  }
  
  for (size_t i = 0; i < ClusterIDs.size(); i++)
  {
    fprintf(OutputPCFFile,
            "%d\t%s\n",
            ClusterIDs[i]+PARAVER_OFFSET, /* It needs the offset, because it uses internal numbering */
            GetClusterName(ClusterIDs[i]).c_str());
  }
  
  return true;
}



bool ClusteredEventsPRVGenerator::BurstOpeningEvent(Event* CurrentEvent)
{
  for (size_t i = 0; i < CurrentEvent->GetTypeValueCount(); i++)
  {
    event_type_t  EventType  = CurrentEvent->GetType(i);
    event_value_t EventValue = CurrentEvent->GetValue(i);

    if (EventsToDealWith.count(EventType) == 1 && EventValue != 0)
    {
      return true;
    }
  }
  return false;
}

bool ClusteredEventsPRVGenerator::BurstClosingEvent(Event* CurrentEvent)
{
  for (size_t i = 0; i < CurrentEvent->GetTypeValueCount(); i++)
  {
    event_type_t  EventType  = CurrentEvent->GetType(i);
    event_value_t EventValue = CurrentEvent->GetValue(i);

    if (EventsToDealWith.count(EventType) == 1 && EventValue == 0)
    {
      return true;
    }
  }
  
  return true;
}

bool ClusteredEventsPRVGenerator::CopyROWFile(void)
{
  ifstream InputROW(InputROWName.c_str(), std::ios::binary);
  if (!InputROW)
  {
    ostringstream WarningMessage;

    WarningMessage << "unable to open input .row file '" << InputROWName << "'";
    SetWarning(true);
    SetWarningMessage(WarningMessage.str());
    return false;
  }
  
  ofstream OutputROW(OutputROWName.c_str(), std::ios::binary);
  if (!OutputROW)
  {
    ostringstream WarningMessage;

    WarningMessage << "unable to open output .row file '" << OutputROWName << "'";
    SetWarningMessage(WarningMessage.str());
    return false;
  }
  
  OutputROW << InputROW.rdbuf();
  
  return true;
}

void ClusteredEventsPRVGenerator::PrepareClusterIDsVector(vector<cluster_id_t>& ClusterIDs,
                                                          set<cluster_id_t>&    DifferentIDs,
                                                          cluster_id_t&         MaxIDUsed)
{
  set<cluster_id_t>::iterator DifferentIDsIterator;
  
  MaxIDUsed = NOISE_CLUSTERID;
  
  if (DifferentIDs.count(DURATION_FILTERED_CLUSTERID) == 0)
  {
    ClusterIDs.push_back(DURATION_FILTERED_CLUSTERID);
  }
  
  if (DifferentIDs.count(RANGE_FILTERED_CLUSTERID) == 0)
  {
    ClusterIDs.push_back(RANGE_FILTERED_CLUSTERID);
  }
  
  if (DifferentIDs.count(THRESHOLD_FILTERED_CLUSTERID) == 0)
  {
    ClusterIDs.push_back(THRESHOLD_FILTERED_CLUSTERID);
  }
  
  if (DifferentIDs.count(NOISE_CLUSTERID) == 0)
  {
    ClusterIDs.push_back(NOISE_CLUSTERID);
  }
  
  for (DifferentIDsIterator  = DifferentIDs.begin();
       DifferentIDsIterator != DifferentIDs.end();
       ++DifferentIDsIterator)
  {
    ClusterIDs.push_back((*DifferentIDsIterator));
    if ((*DifferentIDsIterator) > MaxIDUsed)
    {
      MaxIDUsed = (*DifferentIDsIterator);
    }
  }
  sort(ClusterIDs.begin(), ClusterIDs.end());
  
  return;
}

/**
 * Returns the cluster name using the ID
 * 
 * \param ID ID value to generate the cluster name
 * 
 * \return The name of the cluster, taking into account the special cluster ids
 */
string ClusteredEventsPRVGenerator::GetClusterName(cluster_id_t ID)
{
  ostringstream ClusterName;
  
  switch (ID)
  {
    case UNCLASSIFIED:
      ClusterName << "Unclassified";
      break;
    case DURATION_FILTERED_CLUSTERID:
      ClusterName << "Duration Filtered";
      break;
    case RANGE_FILTERED_CLUSTERID:
      ClusterName << "Range Filtered";
      break;
    case THRESHOLD_FILTERED_CLUSTERID:
      ClusterName << "Threshold Filtered";
      break;
    case NOISE_CLUSTERID:
      ClusterName << "Noise";
      break;
    default:
      ClusterName << "Cluster " << ID;
      break;
  }

  return ClusterName.str();
}
