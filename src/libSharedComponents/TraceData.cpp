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

#include <trace_clustering_types.h>

#include <SystemMessages.hpp>
using cepba_tools::system_messages;

#include <ParametersManager.hpp>

#include "TraceData.hpp"
#include "CPUBurst.hpp"

#include <iostream>
using std::cerr;
using std::endl;

#include <sstream>
using std::ostringstream;

#include <algorithm>
using std::sort;
#include <limits>
using std::numeric_limits;

using std::make_pair;

#include <cmath>
#include <cstdlib>

#ifdef HAVE_MPI
#include <mpi.h>
#endif

// #define DEBUG_PARAVER_INPUT 1

/******************************************************************************
/* Singleton management
 *****************************************************************************/
TraceData* TraceData::Instance = NULL;

TraceData* TraceData::GetInstance(void)
{
  if (TraceData::Instance == NULL)
  {
    TraceData::Instance = new TraceData();
  }
  return TraceData::Instance;
}

/******************************************************************************
/* Static variables
 *****************************************************************************/
bool TraceData::distributed = false;

/****************************************************************************
 * Constructor                                                              *
 ***************************************************************************/

TraceData::TraceData(void)
{
  ClusteringConfiguration *Configuration;

  /* Get Configuration Manager */
  Configuration = ClusteringConfiguration::GetInstance();
  if (!Configuration->IsInitialized())
  {
    SetError(true);
    SetErrorMessage("configuration not initialized");
    return;
  }

  /* Get Parameters Manager */
  Parameters = ParametersManager::GetInstance();
  if (Parameters->GetError())
  {
    SetError(true);
    SetErrorMessage(Parameters->GetLastError());
    return;
  }

  /* Set if we work in a distributed scenario is being used */
  if (TraceData::distributed = Configuration->GetDistributed())
  {
    MyRank     = Configuration->GetMyRank();
    TotalRanks = Configuration->GetTotalRanks();
#ifdef HAVE_SQLITE3
    DBInitialized = false;
#endif
  }

  /* Set normalization attributes */
  NormalizeData = Configuration->GetNormalizeData();
  Normalized    = false;

  /* Get the duration filter */
  DurationFilter = Configuration->GetDurationFilter();

  /* Initialization of Max and Min values */
  ClusteringDimensions = Parameters->GetClusteringParametersSize();

  MinValues    = vector<double>(ClusteringDimensions, numeric_limits<double>::max());
  MaxValues    = vector<double>(ClusteringDimensions, -1.0 * numeric_limits<double>::max()); // Tricky!!
  MinInstances = vector<instance_t>(ClusteringDimensions, 0);
  MaxInstances = vector<instance_t>(ClusteringDimensions, 0);

  SumValues    = vector<double>(ClusteringDimensions, 0);

  /* NO distribution defaults */
  ReadAllTasks = true;
  Master       = false;

  /* NO sampling by default */
  SampleData    = false;
  NumberOfTasks = 0;
}


/****************************************************************************
 * NewBurst
 ***************************************************************************/

bool TraceData::NewBurst(task_id_t                         TaskId,
                         thread_id_t                       ThreadId,
                         line_t                            Line,
                         timestamp_t                       BeginTime,
                         timestamp_t                       EndTime,
                         duration_t                        BurstDuration,
                         map<event_type_t, event_value_t>& EventsData,
                         set<event_type_t>&                BurstEndEvents,
                         bool                              toCluster)
{
  CPUBurst* Burst;

  vector<double>      ClusteringRawData;
  vector<double>      ClusteringProcessedData;
  map<size_t, double> ExtrapolationData;
  bool                DurationFiltered, RangeFiltered, Incomplete;
  burst_type_t        BurstType;

  /* JGG: Check InstDisp potential overflows! */
  event_value_t CyclesCheck   = 0;
  event_value_t TotInstCheck  = 0;
  event_value_t InstDispCheck = 0;

  /* DEBUG */
#ifdef DEBUG_PARAVER_INPUT
  printf("New BURST for T%02d:Th%02d Accepted (DurationFilter = %lld BurstDuration = %lld)!\n",
         TaskId,
         ThreadId,
         DurationFilter,
         BurstDuration);
#endif

#ifdef HAVE_SQLITE3
  if (Master && !DBInitialized)
  {
    AllBurstsDB = BurstsDB(ParametersManager::GetInstance());
    if (AllBurstsDB.GetError())
    {
      SetError(true);
      SetErrorMessage(AllBurstsDB.GetLastError());
      return false;
    }

    if (!AllBurstsDB.BeginInserts())
    {
      SetError(true);
      SetErrorMessage(AllBurstsDB.GetLastError());
      return false;
    }

    DBInitialized = true;
  }
#endif

  /* Add duration to events data */
  EventsData.insert(make_pair(DURATION_EVT_TYPE,
                              (event_value_t) BurstDuration));

  /* Add data to parameters manager */
  Parameters->Clear();
  Parameters->NewData(EventsData, BurstEndEvents);

  /* Retrieve data from parameters manager */
  BurstType = Parameters->GetData(ClusteringRawData,
                                  ClusteringProcessedData,
                                  ExtrapolationData);

  /* Check the duration filter */
  if (BurstType != MissingDataBurst && (BurstDuration < DurationFilter))
  {
    BurstType = DurationFilteredBurst;
  }

  if (Line == 0 && BurstDuration == 0)
  { /* Burst not valid */
    return true;
  }

  Burst = new CPUBurst (TaskId,
                        ThreadId,
                        Line,
                        BeginTime,
                        EndTime,
                        BurstDuration,
                        ClusteringRawData,
                        ClusteringProcessedData,
                        ExtrapolationData,
                        BurstType);

  // if (Master || ReadThisTask(TaskId))

#ifdef HAVE_SQLITE3
  if (Master)
  {
    if (!AllBurstsDB.NewBurst(Burst))
    {
      SetError(true);
      SetErrorMessage(AllBurstsDB.GetLastError());
      return false;
    }
  }
#else
  if (Master || ReadThisTask((TaskId)))
  {
    AllBursts.push_back(Burst);
  }
#endif


  if (BurstType == CompleteBurst)
  {
    if (Master || ReadThisTask(TaskId))
    {
      if (TaskId > NumberOfTasks)
      { /* Keep track of the total number of tasks */
        NumberOfTasks = TaskId;
      }
#ifndef HAVE_SQLITE3
      CompleteBursts.push_back(Burst);
#endif
    }

    for (size_t i = 0; i < ClusteringDimensions; i++)
    {
      if (ClusteringProcessedData[i] > MaxValues[i])
      {
        MaxValues[i]    = ClusteringProcessedData[i];
        MaxInstances[i] = Burst->GetInstance();
      }

      if (ClusteringProcessedData[i] < MinValues[i])
      {
        MinValues[i]    = ClusteringProcessedData[i];
        MinInstances[i] = Burst->GetInstance();
      }

      SumValues[i] += ClusteringProcessedData[i];
    }

    if (ReadThisTask(TaskId))
    {
      ClusteringBursts.push_back(Burst);
    }
  }

  if (!ReadThisTask(TaskId))
  {
    if (!Master)
    {
      delete Burst;
    }
#ifdef HAVE_SQLITE3
    if (Master)
    {
      delete Burst;
    }
#endif
  }

  /* DEBUG
  cout << "ClusteringBursts.size = " << ClusteringBursts.size() << endl;
  cout << "BurstType = " << BurstType << endl;
  */

  return true;
}

/****************************************************************************
 * NewBurst
 ***************************************************************************/
/** 2014/08/28: This version of 'NewBurst' is essentially an adapter to call
  * it from the TreeDBSCAN, avoiding the requirement of providing the 
  * BurstEnd events, closely related to the Trace based clustering
  */
bool TraceData::NewBurst(task_id_t                         TaskId,
				         thread_id_t                       ThreadId,
				         line_t                            Line,
				         timestamp_t                       BeginTime,
				         timestamp_t                       EndTime,
				         duration_t                        BurstDuration,
				         map<event_type_t, event_value_t>& EventsData,
				         bool                              toCluster)
{
  set<event_type_t> FakeBurstEndEvents;

  for(std::map<event_type_t, event_value_t>::iterator iter  = EventsData.begin();
													  iter != EventsData.end();
                                                      ++iter)
  {
    FakeBurstEndEvents.insert(iter->first);
  }

  return NewBurst(TaskId,
                  ThreadId,
                  Line,
                  BeginTime,
                  EndTime,
                  BurstDuration,
                  EventsData,
                  FakeBurstEndEvents,
                  toCluster);
}

/****************************************************************************
 * NewBurst
 ***************************************************************************/
bool TraceData::NewBurst(instance_t           Instance,
                         task_id_t            TaskId,
                         thread_id_t          ThreadId,
                         line_t               Line,
                         timestamp_t          BeginTime,
                         timestamp_t          EndTime,
                         duration_t           BurstDuration,
                         vector<double>      &ClusteringRawData,
                         vector<double>      &ClusteringProcessedData,
                         map<size_t, double> &ExtrapolationData,
                         burst_type_t         BurstType)
{
  CPUBurst *Burst = new CPUBurst (Instance,
                                  TaskId,
                                  ThreadId,
                                  Line,
                                  BeginTime,
                                  EndTime,
                                  BurstDuration,
                                  ClusteringRawData,
                                  ClusteringProcessedData,
                                  ExtrapolationData,
                                  BurstType);

  /* DEBUG
  ostringstream Messages;
  Messages << "Burst number " << Burst->GetInstance() << " Type = " << BurstType << endl;
  system_messages::information(Messages.str().c_str());
  */

#ifdef HAVE_SQLITE3
  if (Master && !DBInitialized)
  {
    AllBurstsDB = BurstsDB(ParametersManager::GetInstance());
    if (AllBurstsDB.GetError())
    {
      SetError(true);
      SetErrorMessage(AllBurstsDB.GetLastError());
      return false;
    }

    DBInitialized = true;
  }
#endif

#ifdef HAVE_SQLITE3
  if (Master)
  {
    if (!AllBurstsDB.NewBurst(Burst))
    {
      SetError(true);
      SetErrorMessage(AllBurstsDB.GetLastError());
      return false;
    }
  }
#else
  if (Master || ReadThisTask((TaskId)))
  {
    AllBursts.push_back(Burst);
  }
#endif


  if (BurstType == CompleteBurst)
  {
    if (Master || ReadThisTask(TaskId))
    {
      if (TaskId > NumberOfTasks)
      { /* Keep track of the total number of tasks */
        NumberOfTasks = TaskId;
      }
#ifndef HAVE_SQLITE3
      CompleteBursts.push_back(Burst);
#endif
    }

    for (size_t i = 0; i < ClusteringDimensions; i++)
    {
      if (ClusteringProcessedData[i] > MaxValues[i])
      {
        MaxValues[i]    = ClusteringProcessedData[i];
        MaxInstances[i] = Burst->GetInstance();
      }

      if (ClusteringProcessedData[i] < MinValues[i])
      {
        MinValues[i]    = ClusteringProcessedData[i];
        MinInstances[i] = Burst->GetInstance();
      }

      SumValues[i] += ClusteringProcessedData[i];
    }

    if (ReadThisTask(TaskId))
    {
      ClusteringBursts.push_back(Burst);
    }
  }

  if (!Master && !ReadThisTask(TaskId))
  {
    delete Burst;
  }

  return true;
}


/****************************************************************************
 * DataExtractionFinished
 ***************************************************************************/
bool TraceData::DataExtractionFinished(void)
{
#ifdef HAVE_SQLITE3

  if (!AllBurstsDB.CommitInserts())
  {
    SetErrorMessage(AllBurstsDB.GetLastError());
    SetError(true);
    return false;
  }

#endif

  return (Normalize());

  /*
  if (!Normalized)
  {
    return false;
  }

  return true;
  */
}


/****************************************************************************
 * Sampling
 ***************************************************************************/
bool TraceData::Sampling(size_t MaxSamples)
{
  vector< vector<CPUBurst*> > BurstsPerTask (NumberOfTasks, vector<CPUBurst*>());

  if (CompleteBursts.size() <= MaxSamples)
  {
    /* No Sampling Needed! */
    this->SampleData = false;
    return true;
  }
  else
  {
    this->SampleData = true;
    /* Previously selected bursts are not used */
    ClusteringBursts.clear();
  }

  /* Compute the total number of bursts per task */
  for (size_t i = 0; i < CompleteBursts.size(); i++)
  {
    BurstsPerTask[CompleteBursts[i]->GetTaskId()].push_back(CompleteBursts[i]);
  }

  /* DEBUG
  cout << "Bursts per Task: ";
  for (size_t i = 0; i < BurstsPerTask.size(); i++)
  {
    cout << "[" << i+1 << ":" << BurstsPerTask[i].size() << "] ";
  }
  cout << endl; */

  /* DEBUG
  cout << "Sampling Sizes = "; */
  srand(time(NULL));

  for (size_t i = 0; i < BurstsPerTask.size(); i++)
  {
    size_t CurrentTaskSampleSize;

    CurrentTaskSampleSize = (size_t) std::floor((BurstsPerTask[i].size()*MaxSamples)/CompleteBursts.size());

    if (!SampleSingleTask(BurstsPerTask[i], CurrentTaskSampleSize))
    {
      return false;
    }
  }

  /* Sort sampled vector */
  sort(ClusteringBursts.begin(), ClusteringBursts.end(), InstanceNumCompare());
  // cout << endl;

  /* DEBUG
  cout << "ClusteringBursts.size() =" << ClusteringBursts.size() << endl;
  exit(EXIT_SUCCESS); */

  return true;
}

bool TraceData::Normalize(void)
{
  bool EmptyRanges = true;

  TraceData::iterator DataIterator;

  if (NormalizeData && !Normalized)
  {
    /* DEBUG
    cout << "MaxValues = { ";
    for (INT32 i = 0; i < MaxValues.size(); i++)
    {
      cout << MaxValues[i] << " ";
    }
    cout << "}" << endl;

    cout << "MaxInstances = { ";
    for (INT32 i = 0; i < MaxInstances.size(); i++)
    {
      cout << MaxInstances[i] << " ";
    }
    cout << "}" << endl;

    cout << "MinValues = { ";
    for (INT32 i = 0; i < MinValues.size(); i++)
    {
      cout << MinValues[i] << " ";
    }
    cout << "}" << endl;

    cout << "MinInstances = { ";
    for (INT32 i = 0; i < MinInstances.size(); i++)
    {
      cout << MinInstances[i] << " ";
    }
    cout << "}" << endl;
    */

#ifdef HAVE_MPI
    /* No longer needed
    if (MPI)
    { /* Range of dimensions should be distributed
      double LocalMaxValues[MaxValues.size()];
      double GlobalMaxValues[MaxValues.size()];
      double LocalMinValues[MinValues.size()];
      double GlobalMinValues[MinValues.size()];

      for (size_t i = 0; i < MaxValues.size(); i++)
      {
        LocalMaxValues[i] = MaxValues[i];
        LocalMinValues[i] = MinValues[i];
      }

      /* Reduction of Max Values
      MPI_Allreduce((void*) LocalMaxValues,
                    (void*) GlobalMaxValues,
                    (int)   MaxValues.size(),
                    MPI_DOUBLE,
                    MPI_MAX,
                    MPI_COMM_WORLD);

      for (size_t i = 0; i < MaxValues.size(); i++)
      {
        MaxValues[i] = GlobalMaxValues[i];
      }

      /* Reduction of Min Values
      MPI_Allreduce((void*) LocalMinValues,
                    (void*) GlobalMinValues,
                    (int)   MaxValues.size(),
                    MPI_DOUBLE,
                    MPI_MIN,
                    MPI_COMM_WORLD);

      for (size_t i = 0; i < MinValues.size(); i++)
      {
        MinValues[i] = GlobalMinValues[i];
      }

      DEBUG

      cout << "After Reduce MaxValues = { ";
      for (INT32 i = 0; i < MaxValues.size(); i++)
      {
        cout << MaxValues[i] << " ";
      }
      cout << "}" << endl;

      cout << "After Reduce MinValues = { ";
      for (INT32 i = 0; i < MinValues.size(); i++)
      {
        cout << MinValues[i] << " ";
      }
      cout << "}" << endl;

    } */
#endif

    vector<double> Factors = Parameters->GetClusteringParametersFactors();

    /* Check parameters ranges */
    for (size_t i = 0; i < MaxValues.size(); i++)
    {
      if (MaxValues[i] !=  MinValues[i])
      {
        EmptyRanges = false;
        break;
      }
    }

    if (EmptyRanges)
    {
      SetError(true);
      SetErrorMessage("all clustering parameters have empty ranges");
      Normalized = false;
      return false;
    }

#ifdef HAVE_SQLITE3
    if (Master)
    {
      if (!AllBurstsDB.NormalizeBursts(MaxValues, MinValues, Factors))
      {
        return;
      }

      for (DataIterator  = ClusteringBursts.begin();
           DataIterator != ClusteringBursts.end();
         ++DataIterator)
      {
        if (!(*DataIterator)->IsNormalized())
        {
          (*DataIterator)->RangeNormalization(MaxValues, MinValues, Factors);
        }
      }
    }
    else
    {
      for (DataIterator  = CompleteBursts.begin();
           DataIterator != CompleteBursts.end();
         ++DataIterator)
      {
        if (!(*DataIterator)->IsNormalized())
        {
          (*DataIterator)->RangeNormalization(MaxValues, MinValues, Factors);
        }
      }

      for (DataIterator  = FilteredBursts.begin();
           DataIterator != FilteredBursts.end();
         ++DataIterator)
      {
        if (!(*DataIterator)->IsNormalized())
        {
          (*DataIterator)->RangeNormalization(MaxValues, MinValues, Factors);
        }
      }
    }
#else

    for (DataIterator  = CompleteBursts.begin();
         DataIterator != CompleteBursts.end();
         DataIterator++)
    {
      if (!(*DataIterator)->IsNormalized())
      {
        (*DataIterator)->RangeNormalization(MaxValues, MinValues, Factors);
      }
    }
    for (DataIterator  = FilteredBursts.begin();
         DataIterator != FilteredBursts.end();
         DataIterator++)
    {
      if (!(*DataIterator)->IsNormalized())
      {
        (*DataIterator)->RangeNormalization(MaxValues, MinValues, Factors);
      }
    }
#endif



    // cout << "END OF NORMALIZATIONS" << endl;

    Normalized = true;
  }

  return true;
}

void TraceData::ScalePoints(void)
{
  TraceData::iterator DataIterator;

  vector<double> Mean          (ClusteringDimensions);
  vector<double> SumDiffSquare (ClusteringDimensions);
  vector<double> RMS           (ClusteringDimensions);

  for (INT32 i = 0; i < Mean.size(); i++)
  {
    Mean[i]          = SumValues[i]/ClusteringBursts.size();
    SumDiffSquare[i] = 0.0;
    RMS[i]           = 0.0;
  }

  /* Compute the Root-Mean-Square */
  for (DataIterator  = ClusteringBursts.begin();
       DataIterator != ClusteringBursts.end();
       DataIterator++)
  {
    for (INT32 i = 0; i < ClusteringDimensions; i++)
    {
      SumDiffSquare[i] +=
        pow((*(*DataIterator))[i] - Mean[i],2.0);
    }
  }

  for (INT32 i = 0; i < ClusteringDimensions; i++)
  {
    RMS[i] = sqrt(SumDiffSquare[i]/(ClusteringBursts.size()-1));
  }

  /* Scale the points */
  for (DataIterator  = ClusteringBursts.begin();
       DataIterator != ClusteringBursts.end();
       DataIterator++)
  {
    (*DataIterator)->Scale(Mean, RMS);
  }
}

void
TraceData::MeanAdjust(void)
{
  TraceData::iterator DataIterator;
  vector<double> DimensionsAverage (ClusteringDimensions);

  for (INT32 i = 0; i < ClusteringDimensions; i++)
  {
    DimensionsAverage[i] = 0.0;
  }

  for (DataIterator  = ClusteringBursts.begin();
       DataIterator != ClusteringBursts.end();
       DataIterator++)
  {
    for (INT32 i = 0; i < ClusteringDimensions; i++)
    {
      DimensionsAverage[i] += (*(*DataIterator))[i];
    }
  }

  for (INT32 i = 0; i < ClusteringDimensions; i++)
  {
    DimensionsAverage[i] = DimensionsAverage[i]/ClusteringBursts.size();
  }

  for (DataIterator  = ClusteringBursts.begin();
       DataIterator != ClusteringBursts.end();
       DataIterator++)
  {
    (*DataIterator)->MeanAdjust(DimensionsAverage);
  }
}

void
TraceData::BaseChange(vector< vector<double> >& BaseChangeMatrix)
{
  TraceData::iterator DataIterator;

  /* TEST */
  cout << "Changing base of data set" << endl;

  for (DataIterator  = ClusteringBursts.begin();
       DataIterator != ClusteringBursts.end();
       DataIterator++)
  {
    (*DataIterator)->BaseChange(BaseChangeMatrix);
  }
}

size_t TraceData::GetAllBurstsSize(void) const
{
#ifdef HAVE_SQLITE3
  return AllBurstsDB.AllBurstsSize();
#else
  return AllBursts.size();
#endif
}

size_t TraceData::GetCompleteBurstsSize(void) const
{
#ifdef HAVE_SQLITE3
  return AllBurstsDB.CompleteBurstsSize();
#else
  return CompleteBursts.size();
#endif
}

size_t TraceData::GetClusteringBurstsSize(void) const
{
  return ClusteringBursts.size();
}

size_t TraceData::GetFilteredBurstsize(void) const
{
  return FilteredBursts.size();
}

size_t TraceData::GetTraceDataSize(void) const
{
  return ClusteringBursts.size() + FilteredBursts.size();
}

void TraceData::SetNumberOfTasks(size_t NumberOfTasks)
{
  this->NumberOfTasks = NumberOfTasks;
  if (TraceData::distributed)
  {
    SetTasksToRead();
  }
}

bool TraceData::FlushPoints(ostream&             str,
                            vector<cluster_id_t> Cluster_IDs,
                            DataPrintSet         WhatToPrint)
{
  bool Unclassified = false;

  if (Cluster_IDs.size() == 0)
  {
    Unclassified = true;
    // Cluster_IDs = vector<cluster_id_t> (ClusteringBursts.size(), UNCLASSIFIED);

    /* DEBUG
    cout << "Clustering Bursts = " << ClusteringBursts.size() << endl;
    cout << "Complete Bursts = " << CompleteBursts.size() << endl; */
  }

  switch (WhatToPrint)
  {
    case PrintClusteringBursts:
    {
      /* DEBUG
      cout << "Printing Clustering Bursts" << endl; */
      if (Cluster_IDs.size() != ClusteringBursts.size())
      {
        ostringstream Message;
        Message << "number of IDs (" << Cluster_IDs.size() << ") ";
        Message << "different from number of clustering bursts (";
        Message << ClusteringBursts.size() << ")";

        SetErrorMessage(Message.str());
        SetError(true);
        return false;
      }

      /*
      BeginLimitIterator = ClusteringBursts.begin();
      EndLimitIterator   = ClusteringBursts.end();
      DataSize           = ClusteringBursts.size();
      */

      sort(ClusteringBursts.begin(),
           ClusteringBursts.end(),
           InstanceNumCompare());

      return GenericFlushPoints(str,
                                ClusteringBursts.begin(),
                                ClusteringBursts.end(),
                                ClusteringBursts.size(),
                                Cluster_IDs);
      break;
    }
    case PrintCompleteBursts:
    {
      /* DEBUG
      cout << "Printing Complete Bursts" << endl; */
#ifdef HAVE_SQLITE3
      if (Cluster_IDs.size() != AllBurstsDB.CompleteBurstsSize())
      {
        ostringstream Message;
        Message << "number of IDs (" << Cluster_IDs.size() << ") ";
        Message << "different from number of complete bursts (";
        Message << AllBurstsDB.CompleteBurstsSize() << ")";

        SetErrorMessage(Message.str());
        SetError(true);
        return false;
      }

      return GenericFlushPoints(str,
                                AllBurstsDB.complete_bursts_begin(),
                                AllBurstsDB.complete_bursts_end(),
                                AllBurstsDB.CompleteBurstsSize(),
                                Cluster_IDs);

#else
      if (Cluster_IDs.size() != CompleteBursts.size())
      {
        ostringstream Message;
        Message << "number of IDs (" << Cluster_IDs.size() << ") ";
        Message << "different from number of complete bursts (";
        Message << ClusteringBursts.size() << ")";

        SetErrorMessage(Message.str());
        SetError(true);
        return false;
      }

      sort(CompleteBursts.begin(), CompleteBursts.end(), InstanceNumCompare());

      return GenericFlushPoints(str,
                                CompleteBursts.begin(),
                                CompleteBursts.end(),
                                CompleteBursts.size(),
                                Cluster_IDs);
#endif
      break;
    }
    case PrintAllBursts:
    {
      /* DEBUG
      cout << "Printint All Bursts" << endl; */

#ifdef HAVE_SQLITE3
      if (Cluster_IDs.size() != AllBurstsDB.AllBurstsSize())
      {
        ostringstream Message;
        Message << "number of IDs (" << Cluster_IDs.size() << ") ";
        Message << "different from number of complete bursts (";
        Message << AllBurstsDB.AllBurstsSize() << ")";

        SetErrorMessage(Message.str());
        SetError(true);
        return false;
      }

      return GenericFlushPoints(str,
                                AllBurstsDB.all_bursts_begin(),
                                AllBurstsDB.all_bursts_end(),
                                AllBurstsDB.AllBurstsSize(),
                                Cluster_IDs);
#else
      /* When printing all bursts, or the data have not been classified
         or  all the complete bursts must have an ID  */

      if (!Unclassified && (Cluster_IDs.size() != CompleteBursts.size()))
      {
        ostringstream Message;
        Message << "number of IDs (" << Cluster_IDs.size() << ") ";
        Message << "different from number of bursts (";
        Message << CompleteBursts.size() << ")";

        SetErrorMessage(Message.str());
        SetError(true);
        return false;
      }

      sort(AllBursts.begin(), AllBursts.end(), InstanceNumCompare());

      return GenericFlushPoints(str,
                                AllBursts.begin(),
                                AllBursts.end(),
                                AllBursts.size(),
                                Cluster_IDs);
#endif

      break;
    }
    default:
    {
      ostringstream Message;
      Message << "wrong definition about what to print ";

      SetErrorMessage(Message.str());
      SetError(true);
      return false;
    }
  }
  return true;
}

/*****************************************************************************
 * Debug/Informative Methods
 ****************************************************************************/

void TraceData::PrintPoints(void)
{
  TraceData::iterator BurstsIterator;

  for (BurstsIterator  = ClusteringBursts.begin();
       BurstsIterator != ClusteringBursts.end();
       BurstsIterator++)
  {
    (*BurstsIterator)->PrintBurst();
  }
}

void
TraceData::PrintTraceDataInformation(void)
{
  cerr << "*** DATA SET INFORMATION ***" << endl;
  cerr << "Complete Points Size   = " << CompleteBursts.size() << endl;
  cerr << "Clustering Points Size = " << ClusteringBursts.size() << endl;
  cerr << "Filtered Points Size   = " << FilteredBursts.size() << endl;

}

/*****************************************************************************
 * Private Methods
 ****************************************************************************/

bool TraceData::SampleSingleTask(vector<CPUBurst*>& TaskBursts,
                                 size_t             NumSamples)
{
  size_t SamplingFrequency = (size_t) std::floor(TaskBursts.size()/NumSamples);

  if (NumSamples > TaskBursts.size())
  {
    ostringstream ErrorMessage;
    ErrorMessage << "more samples samples required (" << NumSamples;
    ErrorMessage << ") than number os bursts (" << TaskBursts.size() << ")";

    SetError(true);
    SetErrorMessage(ErrorMessage.str());

    return false;
  }

  for (size_t i = 0; i < NumSamples; i++)
  {
    vector<CPUBurst*>::size_type CurrentSample;

    CurrentSample = ( random() % SamplingFrequency );
    CurrentSample += (i*SamplingFrequency);

    ClusteringBursts.push_back(TaskBursts[CurrentSample]);
  }

  return true;
}

bool CheckBurstEndEvents(map<event_type_t, event_value_t>& EventsData,
                         set<event_type_t>&                BurstEndEvents)
{

  return true;
}


void TraceData::SetTasksToRead()
{
  INT32 TasksPerProcess;
  INT32 Remainder;

  if (TasksToRead.size() != 0)
  {
    // TasksToRead set has been previously set
    return;
  }

  if (!Master && MyRank == 0)
  {
    Master = true;
  }

  TasksToRead.clear();

  TasksPerProcess = NumberOfTasks/TotalRanks;
  Remainder       = NumberOfTasks%TotalRanks;

  for (INT32 i = (MyRank*TasksPerProcess);
             i < (MyRank*TasksPerProcess)+TasksPerProcess;
             i++)
  {
    TasksToRead.insert(i);
  }


  if (Remainder != 0)
  {
    if (MyRank+1 <= Remainder)
    {
      TasksToRead.insert(TasksPerProcess*TotalRanks+MyRank+1);
    }
  }

  /*
  cout << "My Rank is = " << Me << endl;
  cout << "[" << Me << "]: Tasks assigned = [";

  set<INT32>::iterator it;

  for (it = TasksToRead.begin(); it != TasksToRead.end(); ++it)
  {
    cout << " " << (*it)+1;
  }
  cout << " ]" << endl;
  */
}

bool TraceData::ReadThisTask(task_id_t Task)
{
  set<int>::iterator SetIterator;

  if (TasksToRead.size() == 0)
  {
    return true;
  }
  else
  {
    SetIterator = TasksToRead.find((int) Task);

    if (SetIterator == TasksToRead.end())
    {
      return false;
    }
  }

  return true;
}
