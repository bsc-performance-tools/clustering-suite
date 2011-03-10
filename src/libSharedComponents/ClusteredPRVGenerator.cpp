/*******************************************************************************
 *  $RCSfile: ClusteredPRVGenerator.cpp,v $
 *
 *  Last modifier: $Author: jgonzale $
 *  Last check in: $Date: 2009/02/27 10:42:21 $
 *  Revision:      $Revision: 1.7 $
 *
 *  Copyright (c) 2006  CEPBA Tools
 *  jgonzale@cepba.upc.edu
 *
 *  Description: Paraver trace reconstruction class implementation
 *
 ******************************************************************************/

#include <SystemMessages.hpp>
using cepba_tools::system_messages;

#include "ClusteredPRVGenerator.hpp"
#include "ParaverTraceParser.hpp"
#include <ParaverColors.h>

#include <unistd.h>
#include <cstring>

#include <algorithm>
using std::sort;
using std::merge;

// #define DEBUG_PRV_OUTPUT 0

#define RUNNING_STATE 1
#define CLUSTERS_EVENTS_TXT \
"\n"\
"EVENT_TYPE\n"\
"9\t90000001\tCluster ID\n"\
"VALUES\n"\
"0\tEnd\n"\
"1\tMissing Data\n"\
"2\tDuration Filtered\n"\
"3\tRange Filtered\n"\
"4\tThreshold Filtered\n"\
"5\tNoise\n"

ClusteredPRVGenerator::ClusteredPRVGenerator(string  InputTraceName,
                                             string  OutputTraceName)
:ClusteredTraceGenerator(InputTraceName, OutputTraceName)
{
  string::size_type SubstrPos;
  
  if (GetError())
    return;

  TraceParser = new ParaverTraceParser(InputTraceName, InputTraceFile);
  
  /* Search for a possible PCF file */
  PCFPresent = false;
  
  /* Check the PCF file name */
  SubstrPos = InputTraceName.rfind(".prv");
  
  if (SubstrPos == string::npos)
  {
    InputPCFName = InputTraceName+".pcf";
  }
  else
  {
    string InputPCFBaseName;
    
    InputPCFBaseName = InputTraceName.substr(0, SubstrPos);
    InputPCFName     = InputPCFBaseName+".pcf";
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
      string OutputPCFBaseName;
      
      OutputPCFBaseName = OutputTraceName.substr(0, SubstrPos);
      OutputPCFName     = OutputPCFBaseName+".pcf";
    }
    
    if ((OutputPCFFile = fopen(OutputPCFName.c_str(), "w")) == NULL)
    {
      /* We are unable to open the output PCF file. The PCF managment is
         disabled */
      fclose(InputPCFFile);
      PCFPresent = false;
    }
  }
  
  /* DEBUG 
  if (PCFPresent)
  {
    printf("PCF FOUND!!!!\n");
  }
  
  printf("Input PCF = %s\nOutput PCF = %s\n",
           InputPCFName.c_str(),
           OutputPCFName.c_str());
  */
  
  return;
}

bool ClusteredPRVGenerator::Run(vector<CPUBurst*>&    Bursts,
                                vector<cluster_id_t>& IDs,
                                size_t                NumberOfClusters)
{
  ParaverHeader*                  Header;
  vector<ApplicationDescription*> AppsDescription;
  timestamp_t                     BeginTimeIndex;
  vector<cluster_id_t>            CompleteIDs;
                    
  ParaverRecord                *CurrentRecord;
  Event                        *CurrentEvent;
  State                        *CurrentState;
  INT32                         CurrentPercentage = 0;
  vector<timestamp_t>           BurstsEnd;
  size_t                        FilterBurstsEndIndex;
  size_t                        TotalBursts;
  double                        TimeFactor;

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
        CompleteIDs.push_back(DURATION_FILTERED_CLUSTERID);
        break;
      case RangeFilteredBurst:
        CompleteIDs.push_back(RANGE_FILTERED_CLUSTERID);
        break;
      case MissingDataBurst:
        CompleteIDs.push_back(MISSING_DATA_CLUSTERID);
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
  
  for (INT32 i = 0; i < AppsDescription.size(); i++)
  {
    vector<Communicator*> Communicators;
    
    Communicators = AppsDescription[i]->GetCommunicators();
    
    for (INT32 j = 0; j < Communicators.size(); j++)
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
    for (INT32 j = 0; j < AppsDescription[i]->GetTaskCount(); j++)
    {
      BurstsEnd.push_back(0);
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
    INT32 PercentageRead;
    bool  CurrentRecordFlushed = false;
    
    /* We are only interested on event and state records */
    if (CurrentRecord->GetRecordType() == PARAVER_STATE)
    {
      CurrentState = (State*) CurrentRecord;
      CurrentEvent = NULL;
    }
    else if (CurrentRecord->GetRecordType() == PARAVER_EVENT)
    {
      CurrentState = NULL;
      CurrentEvent = (Event*) CurrentRecord;
    }
    else
    {
      CurrentState = NULL;
      CurrentEvent = NULL;
    }
    
    if (CurrentState != NULL)
    {
      if (CurrentState->GetStateValue() == RUNNING_STATE)
      {
        /* Check if there is a pending cluster region to close */
        if (BurstsEnd[CurrentState->GetTaskId()] == CurrentState->GetBeginTime()
        &&  BurstsEnd[CurrentState->GetTaskId()] != 0)
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
          
          BurstsEnd[CurrentState->GetTaskId()] = 0;
        }
          
        /* DEBUG
        cout << "BeginTimeIndex = " << BeginTimeIndex << " - ";
        cout << "BeginTimePoints.size() = " << BeginTimePoints.size() << endl; */
          
        if (BeginTimeIndex < Bursts.size())
        {
          CPUBurst* CurrentBurst = Bursts[BeginTimeIndex];

          if (CurrentState->GetBeginTime() == CurrentBurst->GetBeginTime() &&
              CurrentState->GetTaskId()    == CurrentBurst->GetTaskId())
          {
            Event* NewEvent;
            
            NewEvent = new Event(0, /* Line not needed */
                                 CurrentState->GetTimestamp(),
                                 CurrentState->GetCPU()+1,
                                 CurrentState->GetAppId()+1,
                                 CurrentState->GetTaskId()+1,
                                 CurrentState->GetThreadId()+1);
            
            NewEvent->AddTypeValue(90000001,
                                   (INT64) CompleteIDs[CurrentBurst->GetInstance()]);
            
#ifdef DEBUG_PRV_OUTPUT
            cout << "Flushing opening cluster event: " << *NewEvent;
            
            if (CompleteIDs[CurrentBurst->GetInstance() > 20)
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
            
            BeginTimeIndex++;
            
            /* Set the end time */
            BurstsEnd[CurrentState->GetTaskId()] = CurrentBurst->GetEndTime();
          }
        }
      }
    }
    else if (CurrentEvent != NULL)
    {
      if (BurstsEnd[CurrentEvent->GetTaskId()] == CurrentEvent->GetTimestamp()
      &&  BurstsEnd[CurrentEvent->GetTaskId()] != 0) // In timestamp 0 it is impossible to close a burst
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
        
#ifdef DEBUG_PRV_OUTPUT
        cout << "Flush closing cluster event [Filtered]: " << *NewEvent;
#endif
        
        /* FilterBurstsEnd.pop_front(); */
        BurstsEnd[CurrentEvent->GetTaskId()] = 0;
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
    
    if (!CurrentRecordFlushed)
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
    GenerateOutputPCF(NumberOfClusters);
  }
  
  return true;
}

bool
ClusteredPRVGenerator::GenerateOutputPCF(size_t NumberOfClusters)
{
  char   Buffer[256], AuxBuffer[256];
  size_t BufferSize = sizeof(Buffer);
  bool   ColorsSectionRead     = false;
  bool   ColorsSectionFinished = false;
  bool   FlushBuffer = false;

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
        
        for (INT32 i = 0;
             i < (NumberOfClusters+PARAVER_OFFSET);
             i++)
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

  /* Predefined cluster ids */
  fprintf(OutputPCFFile,
          "%d\t%s\n",
          MISSING_DATA_CLUSTERID,
          "Missing Data");

  fprintf(OutputPCFFile,
          "%d\t%s\n",
          DURATION_FILTERED_CLUSTERID,
          "Duration Filtered");

  fprintf(OutputPCFFile,
          "%d\t%s\n",
          RANGE_FILTERED_CLUSTERID,
          "Range Filtered");

  fprintf(OutputPCFFile,
          "%d\t%s\n",
          THRESHOLD_FILTERED_CLUSTERID,
          "Threshold Filtered");

  fprintf(OutputPCFFile,
          "%d\t%s\n",
          NOISE_CLUSTERID+PARAVER_OFFSET, /* It needs the offset, because it uses internal numbering */
          "Noise");
  
  /* The numbering of clusters starts at 1 */
  for (size_t i = 1; i <= NumberOfClusters; i++)
  {
    fprintf(OutputPCFFile,
          "%d\tCluster %d\n",
          i+PARAVER_OFFSET, /* It needs the offset, because it uses internal numbering */
          i);
  }
  
  return true;
}