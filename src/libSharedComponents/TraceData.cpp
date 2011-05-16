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

  $URL::                                          $:  File
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

/* A  B  C  D  E  F  G  H  I  K  L  M  N  P  Q  R  S  T  V  W  X  Y  Z  * */

/*
INT32 TraceData::AminoacidTranslationSize = 23;
char TraceData::AminoacidTranslation[23] = {
  'Z',  // NOISE
  'A',  // 1
  'B',  // 2
  'C',  // 3
  'D',  // 4
  'E',  // 5
  'F',  // 6
  'G',  // 7
  'H',  // 8
  'I',  // 9
  'K',  // 10
  'L',  // 11
  'M',  // 12
  'N',  // 13
  'P',  // 14
  'Q',  // 15
  'R',  // 16
  'S',  // 17
  'T',  // 18
  'V',  // 19
  'W',  // 20
  'X',  // 21
  'Y'}; // 22
*/

/*
INT32 TraceData::AminoacidTranslationSize = 27;

char TraceData::AminoacidTranslation[27] = { 
  'Z',
  'A',
  'B',
  'C',
  'D',
  'E',
  'F',
  'G',
  'H',
  'I',
  'J',
  'K',
  'L',
  'M',
  'N',
  'O',
  'P',
  'Q',
  'R',
  'S',
  'T',
  'U',
  'V',
  'W',
  'X',
  'Y'};
*/

/*

INT32 TraceData::AminoacidTranslationSize = 50;

char TraceData::AminoacidTranslation[50] = { 'Z', 'A', 'B', 'C', 'D', 'E', 'F', 'G',
  'H', 'I', 'J', 'K', 'L', 'M', 'N', 'O', 'P', 'Q', 'R', 'S', 'T', 'U', 'V', 
  'W', 'Y', 'a', 'b', 'c', 'd', 'f', 'g', 'h', 'i', 'j', 'k', 'l', 'm',
  'n', 'o', 'p', 'q', 'r', 's', 't', 'u', 'v' , 'w', 'x', 'y', 'z' }; */

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
}

/*
void
TraceData::SetUseDuration(bool UseDuration)
{
  this->UseDuration = UseDuration;
}

void
TraceData::SetDurationFilter(UINT64 DurationFilter)
{
  this->DurationFilter = DurationFilter;
}
*/

/*
void
TraceData::SetClusteringParameters(
  vector<ClusteringParameter*>& ClusteringParamters)
{
  this->ClusteringParameters = ClusteringParameters;
}
*/

/*
void
TraceData::SetExtrapolationParameters(
  vector<ClusteringParameter*> ExtrapolationParameters)
{
  this->ExtrapolationParameters = ExtrapolationParameters;
}
*/

/*
void
TraceData::SetFilterThreshold(double FilterThreshold)
{
  this->FilterThreshold = FilterThreshold;
}
*/

/****************************************************************************
 * NewBurst
 ***************************************************************************/

bool
TraceData::NewBurst(task_id_t                         TaskId,
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
  
  /*
  if (BurstType == MissingDataBurst)
  {
    MissingDataBursts.push_back(Burst);
  }
  else if (BurstType == RangeFilteredBurst || BurstType == DurationFilteredBurst)
  {
    FilteredBursts.push_back(Burst);
  }
  else
  {
    ClusteringBursts.push_back(Burst);

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
  }
  */

  return true;
}

/*
bool
TraceData::NewBurst(INT32               TaskId,
                    INT32               ThreadId,
                    UINT64              Line,
                    UINT64              BeginTime,
                    UINT64              EndTime,
                    UINT64              BurstDuration,
                    vector<double>&     ClusteringRawData,
                    vector<double>&     ClusteringProcessedData,
                    map<INT32, double>& ExtrapolationData,
                    burst_type_t        BurstType)
{
  CPUBurst* Burst;

  if (this->ClusteringDimensions != ClusteringRawData.size())
  {
    SetErrorMessage("incorrect number of clustering dimensions");
    SetError(true);
    return false;
  }

  /* A point do not have to have all extrapolation dimensions 
  if (this->ExtrapolationDimensions != ExtrapolationDimensions.size())
  {
    SetErrorMessage("incorrect number of extrapolation dimensions");
    SetError(true);
    return false;
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

  if (BurstType == DurationFilteredBurst ||
      BurstType == RangeFilteredBurst    ||
      BurstType == MissingDataBurst)
  {
    FilteredBursts.push_back(Burst);
  }
  else
  {
    ClusteringBursts.push_back(Burst);

    for (INT32 i = 0; i < this->ClusteringDimensions; i++)
    {
      if (ClusteringProcessedData[i] > MaxValues[i])
      {
        MaxValues[i] = ClusteringProcessedData[i];
        MaxInstances[i] = Burst->GetInstance();
      }

      if (ClusteringProcessedData[i] < MinValues[i])
      {
        MinValues[i] = ClusteringProcessedData[i];
        MinInstances[i] = Burst->GetInstance();
      }

      SumValues[i] += ClusteringProcessedData[i];
    }
  }
}
*/

void 
TraceData::Normalize(void)
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
                            bool                 AllData)
{
  INT32 TotalPoints;
  
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
    Cluster_IDs = vector<cluster_id_t> (ClusteringBursts.size(), UNCLASSIFIED);
  }

  if (Cluster_IDs.size() != CompleteBursts.size())
  {
    ostringstream Message;
    Message << "number of IDs (" << Cluster_IDs.size() << ") ";
    Message << "different from number of bursts (" << CompleteBursts.size() << ")" << endl;
    SetError(true);
    SetErrorMessage(Message.str());
    return false;
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

  /* DEBUG
  cout << "Clustering Bursts = " << ClusteringBursts.size() << endl;
  cout << "All Bursts = "        << AllBursts.size() << endl;
  */

  if (AllData)
  {
    BeginLimitIterator = AllBursts.begin();
    EndLimitIterator   = AllBursts.end();
    DataSize           = AllBursts.size();
    sort(AllBursts.begin(), AllBursts.end(), InstanceNumCompare());
  }
  else
  { /* Print the complete points (as a result of a clustering analysis, all
     * complete points must have an ID */
    BeginLimitIterator = CompleteBursts.begin();
    EndLimitIterator   = CompleteBursts.end();
    DataSize           = CompleteBursts.size();
    sort(CompleteBursts.begin(), CompleteBursts.end(), InstanceNumCompare());
  }
  
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
        CurrentClusterId = Cluster_IDs[ClusteringBurstsCounter]+PARAVER_OFFSET;
        ++ClusteringBurstsCounter;
        // cout << " Complete" << endl;
        break;
      case DurationFilteredBurst:
        CurrentClusterId = DURATION_FILTERED_CLUSTERID;
        // cout << " Duration Filtered" << endl;
        break;
      case RangeFilteredBurst:
        CurrentClusterId = RANGE_FILTERED_CLUSTERID;
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

/*
bool
TraceData::FlushClusterSequences(ostream&  str,
                               bool      FlushNoisePoints)
{
  TraceData::iterator PointsIterator;
  INT32 CurrentTaskId = -1;
  bool  WarningShown = false;
  
  if (DimemasTrace)
  {
    sort(ClusteringBursts.begin(),
         ClusteringBursts.end(),
         DimemasDataPointTaskOrder());
  }
  else
  {
    sort(ClusteringBursts.begin(),
         ClusteringBursts.end(),
         ParaverDataPointTaskOrder());
  }
  
  for (PointsIterator  = ClusteringBursts.begin();
       PointsIterator != ClusteringBursts.end();
       PointsIterator++)
  {
    INT32 CurrentClusterId;
    char  CurrentClusterIdChar;
    
    if (CurrentTaskId != (*PointsIterator)->GetTaskId())
    {
      if (CurrentTaskId != -1)
      {
        str << endl;
      }
      CurrentTaskId = (*PointsIterator)->GetTaskId();
      str << ">TASK" << CurrentTaskId << endl;
    }
    
    if(!FlushNoisePoints &&  
       (*PointsIterator)->GetClusterId() == Constants::NOISE_CLUSTERID) // Noise point not required
    {
      continue;
    }
    
    CurrentClusterId = (*PointsIterator)->GetClusterId()-Constants::CLUSTERID_OFFSET;
    // CurrentClusterId = (*PointsIterator)->GetClusterId();
    
    if ( CurrentClusterId >  (TraceData::AminoacidTranslationSize - 1) )
    {
      /* Show a warning to indicate that some aminoacid number will be repeated 
      if (!WarningShown)
      {
        cerr << "Warning! More clusters than sequence id's. Some id's will be repeated" << endl;
        WarningShown = true;
      }
      
      CurrentClusterIdChar = '*';
      
      // CurrentClusterId = TraceData::AminoacidTranslationSize - 1;
      
      // SetError(true);
      // SetErrorMessage("Too much clusters to produce alignment sequence");
      // return false;
    }
    else
    {
      CurrentClusterIdChar = AminoacidTranslation[CurrentClusterId];
    }
      
    str << CurrentClusterIdChar;
  }
}

*/

/*****************************************************************************
 * DEBUG
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
  cerr << "Complete Points Size = " << CompleteBursts.size() << endl;
  cerr << "Clustering Points Size = " << ClusteringBursts.size() << endl;
  cerr << "Filtered Points Size = " << FilteredBursts.size() << endl;

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
