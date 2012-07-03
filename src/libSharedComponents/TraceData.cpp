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

#ifdef HAVE_MPI
#include <mpi.h>
#endif

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
  if (Distributed = Configuration->GetDistributed())
  {
    MyRank     = Configuration->GetMyRank();
    TotalRanks = Configuration->GetTotalRanks();
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

  /* Add duration to events data */
  EventsData.insert(make_pair(DURATION_EVENT_TYPE,
                              (event_value_t) BurstDuration));

  /* Add data to parameters manager */
  Parameters->Clear();
  Parameters->NewData(EventsData);

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

  /* DEBUG
  ostringstream Messages;
  Messages << "Burst number " << Burst->GetInstance() << " Type = " << BurstType << endl;
  system_messages::information(Messages.str().c_str());
  */

  if (Master || ReadThisTask(TaskId))
  {
    AllBursts.push_back(Burst);
  }

  if (BurstType == CompleteBurst)
  {
    if (Master || ReadThisTask(TaskId))
    {
      if (TaskId > NumberOfTasks)
      { /* Keep track of the total number of tasks */
        NumberOfTasks = TaskId;
      }

      CompleteBursts.push_back(Burst);
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

  if (Master || ReadThisTask(TaskId))
  {
    AllBursts.push_back(Burst);
  }

  if (BurstType == CompleteBurst)
  {
    if (Master || ReadThisTask(TaskId))
    {
      if (TaskId > NumberOfTasks)
      { /* Keep track of the total number of tasks */
        NumberOfTasks = TaskId;
      }

      CompleteBursts.push_back(Burst);
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
  for (size_t i = 0; i < BurstsPerTask.size(); i++)
  {
    size_t CurrentTaskSampleSize;

    CurrentTaskSampleSize = (size_t) std::floor((BurstsPerTask[i].size()*MaxSamples)/CompleteBursts.size());

    /* DEBUG
    cout << i << ":" << CurrentTaskSampleSize << " "; */

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

void TraceData::Normalize(void)
{
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

    // cout << "END OF NORMALIZATIONS" << endl;

    Normalized = true;
  }
}

void
TraceData::ScalePoints(void)
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

size_t TraceData::GetCompleteBurstsSize(void) const
{
  return CompleteBursts.size();
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
  if (Distributed)
  {
    SetTasksToRead();
  }
}

/*
void TraceData::ResetClusteringPoints(void)
{ Moves all complete bursts in 'AllBursts' vector to 'ClusteringBursts'
  vector

  ClusteringBursts.clear();

  for (size_t i = 0; i < AllBursts.size(); i++)
  {
    if (AllBursts[i]->GetBurstType() == CompleteBurst)
    {
      ClusteringBursts.push_back(AllBursts[i]);
    }
  }

  return;
}
*/

/*
bool
TraceData::ReplaceClusterIds(vector< pair<UINT64, INT32> >& ClustersIdList)
{
  INT32 i;
  bool MorePoints = true;
  bool Found;

  TraceData::iterator PointsIterator;

  /* DEBUG
  cout << "Real Points = " << ClusteringPoints.size() << endl;

  PointsIterator = ClusteringPoints.begin();

  for (i = 0; i < ClustersIdList.size() && MorePoints; i++)
  {
    Found = false;

    while (!Found && MorePoints)
    {
      if ((*PointsIterator)->GetInstance() == ClustersIdList[i].first)
      {
        (*PointsIterator)->SetClusterId(ClustersIdList[i].second);
        Found = true;
      }

      PointsIterator++;
      if (PointsIterator == ClusteringPoints.end())
      {
        MorePoints = false;
      }
    }
  }

  if (i != ClustersIdList.size())
  {
    /* Some Points where not assigned
    cout << "i = " << i << " ClustersIdList.size() = " << ClustersIdList.size() << endl;

    SetError(true);
    SetErrorMessage("some instances have not found the corresponding point");
    return false;
  }

  return true;
}
*/

bool TraceData::FlushPoints(ostream&             str,
                            vector<cluster_id_t> Cluster_IDs,
                            DataPrintSet         WhatToPrint)
{
  size_t TotalPoints;

  bool   Unclassified = false;

  ParametersManager *Parameters = ParametersManager::GetInstance();

  vector<string> ClusteringParametersNames;
  vector<string> ExtrapolationParametersNames;

  vector<bool>   ClusteringParametersPrecision;
  vector<bool>   ExtrapolationParametersPrecision;

  TraceData::iterator ClusteringBurstsIterator;
  TraceData::iterator FilteredBurstsIterator;

  TraceData::iterator BurstsIterator, BeginLimitIterator, EndLimitIterator;

  size_t ClusteringBurstsCounter = 0, DataSize;

  /* Check if cluster_ids is empty! */
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

      BeginLimitIterator = ClusteringBursts.begin();
      EndLimitIterator   = ClusteringBursts.end();
      DataSize           = ClusteringBursts.size();

      sort(ClusteringBursts.begin(),
           ClusteringBursts.end(),
           InstanceNumCompare());

      break;
    }
    case PrintCompleteBursts:
    {
      /* DEBUG
      cout << "Printing Complete Bursts" << endl; */

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

      BeginLimitIterator = CompleteBursts.begin();
      EndLimitIterator   = CompleteBursts.end();
      DataSize           = CompleteBursts.size();

      sort(CompleteBursts.begin(), CompleteBursts.end(), InstanceNumCompare());

      break;
    }
    case PrintAllBursts:
    {
      /* DEBUG
      cout << "Printint All Bursts" << endl; */

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

      BeginLimitIterator = AllBursts.begin();
      EndLimitIterator   = AllBursts.end();
      DataSize           = AllBursts.size();

      sort(AllBursts.begin(), AllBursts.end(), InstanceNumCompare());

      break;
    }
    default:
    {
      ostringstream Message;
      Message << "wrong definition about what to print ";
      Message << "number of IDs (" << Cluster_IDs.size() << ") ";
      Message << "different from number of bursts (";
      Message << CompleteBursts.size() << ")";

      SetErrorMessage(Message.str());
      SetError(true);
      return false;
    }
  }

  if (NormalizeData && !Normalized)
  {
    Normalize();
  }

  ClusteringParametersNames    = Parameters->GetClusteringParametersNames();
  ExtrapolationParametersNames = Parameters->GetExtrapolationParametersNames();

  ClusteringParametersPrecision    = Parameters->GetClusteringParametersPrecision();
  ExtrapolationParametersPrecision = Parameters->GetExtrapolationParametersPrecision();

  /* Heading line */
  str << "# Instance,TaskId,ThreadId,Begin_Time,End_Time,Duration, Line";

  for (INT32 i = 0; i < ClusteringParametersNames.size(); i++)
  {
    str << "," << ClusteringParametersNames[i];
  }

  for (INT32 i = 0; i < ClusteringParametersNames.size(); i++)
  {
    str << "," << ClusteringParametersNames[i] << "_Norm";
  }

  for (INT32 i = 0; i < ExtrapolationParametersNames.size(); i++)
  {
    str << "," << ExtrapolationParametersNames[i];
  }

  str << ",ClusterID" << endl;

  system_messages::show_progress("Writing point to disc", 0, DataSize);
  for (BurstsIterator  = BeginLimitIterator, ClusteringBurstsCounter = 0, TotalPoints = 0;
       BurstsIterator != EndLimitIterator;
       ++BurstsIterator)
  {
    cluster_id_t CurrentClusterId;

    /* DEBUG
    cout << "Bursts = " << TotalPoints;
    cout << " Dimensions = " << (*BurstsIterator)->size();
    cout << " Burst Type = " << (*BurstsIterator)->GetBurstType();
    */
    ++TotalPoints;

    switch((*BurstsIterator)->GetBurstType())
    {
      case CompleteBurst:

        if (Unclassified)
        {
          CurrentClusterId = UNCLASSIFIED+PARAVER_OFFSET;
        }
        else
        {
          CurrentClusterId = Cluster_IDs[ClusteringBurstsCounter]+PARAVER_OFFSET;
          ++ClusteringBurstsCounter;
        }
        // cout << " Complete" << endl;
        break;
      case DurationFilteredBurst:
        CurrentClusterId = DURATION_FILTERED_CLUSTERID+PARAVER_OFFSET;
        // cout << " Duration Filtered" << endl;
        break;
      case RangeFilteredBurst:
        CurrentClusterId = RANGE_FILTERED_CLUSTERID+PARAVER_OFFSET;
        // cout << " Range Filtered" << endl;
        break;
      default:
        /* This bursts should not be printed */
        // cout << " Unknown!" << endl;
        continue;
    }

    (*BurstsIterator)->Print(str,
                             ClusteringParametersPrecision,
                             ExtrapolationParametersPrecision,
                             CurrentClusterId);

    system_messages::show_progress("Writing point to disc", TotalPoints, DataSize);
  }
  system_messages::show_progress_end("Writing point to disc", DataSize);

  return true;
}


/*****************************************************************************
 * Debug/Informative Methods
 ****************************************************************************/

void
TraceData::PrintPoints(void)
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
    size_t CurrentSample;

    CurrentSample  = (size_t) (SamplingFrequency*std::rand()/(RAND_MAX+1));
    CurrentSample += (i*SamplingFrequency);

    ClusteringBursts.push_back(TaskBursts[CurrentSample]);
  }

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
