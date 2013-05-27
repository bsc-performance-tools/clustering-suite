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

#ifndef _TRACEDATA_HPP_
#define _TRACEDATA_HPP_

#include "trace_clustering_types.h"

#include <ParametersManager.hpp>
#include <Error.hpp>
using cepba_tools::Error;

#include <SystemMessages.hpp>
using cepba_tools::system_messages;

#include "CPUBurst.hpp"

#ifdef HAVE_SQLITE3
#include "BurstsDB.hpp"
#endif

#include <iostream>
using std::ostream;
using std::cout;
using std::endl;

#include <map>
using std::map;

#include<set>
using std::set;

using std::pair;


enum DataPrintSet { PrintClusteringBursts, PrintCompleteBursts, PrintAllBursts};


class TraceData: public Error
{
  public:
    /* Distributed version management */
    static bool distributed;

  protected:

    static TraceData* Instance;

    TraceData(void);

    /* Number of total objects present in the trace = Tasks x (Threads x Task) */
    size_t TraceObjects;

    /* The main containers */
    vector<CPUBurst*> AllBursts;
    vector<CPUBurst*> CompleteBursts;
    vector<CPUBurst*> ClusteringBursts;
    vector<CPUBurst*> FilteredBursts;
    vector<CPUBurst*> MissingDataBursts;

#ifdef HAVE_SQLITE3
    bool              DBInitialized;
    BurstsDB          AllBurstsDB;
#endif

    /* Parameters parsing */
    duration_t         DurationFilter;
    ParametersManager *Parameters;

    /* Sampling attributes */
    size_t            NumberOfTasks;
    bool              SampleData;

    size_t ClusteringDimensions;
    size_t ExtrapolationDimensions;

    bool NormalizeData;
    bool Normalized;

    bool     Master;
    INT32     MyRank;
    INT32     TotalRanks;
    bool     ReadAllTasks;
    set<int> TasksToRead;


    /* Normalization and statistics attributes */
    bool                Used;      /* Useful to initialize Max & Min arrays */
    vector<double>      MaxValues; /* Maximum and minimum values for each */
    vector<double>      MinValues; /* dimension used on the clustering    */

    vector<double>      SumValues; /* Sumatory of each clustering dimension */

    vector<instance_t>   MaxInstances; /* Instances containing the max and min */
    vector<instance_t>   MinInstances; /* values */

  public:

    typedef vector<CPUBurst*>::iterator iterator;

    static TraceData* GetInstance(void);

    void   SetTraceObjects(size_t TraceObjects) { this->TraceObjects = TraceObjects; };
    size_t GetTraceObjects(void) { return TraceObjects; };

    bool NewBurst(task_id_t                         TaskId,
                  thread_id_t                       ThreadId,
                  line_t                            Line,
                  timestamp_t                       BeginTime,
                  timestamp_t                       EndTime,
                  duration_t                        BurstDuration,
                  map<event_type_t, event_value_t>& EventsData,
                  bool                              toCluster = true);

    bool NewBurst(instance_t           Instance,
                  task_id_t            TaskId,
                  thread_id_t          ThreadId,
                  line_t               Line,
                  timestamp_t          BeginTime,
                  timestamp_t          EndTime,
                  duration_t           BurstDuration,
                  vector<double>      &ClusteringRawData,
                  vector<double>      &ClusteringProcessedData,
                  map<size_t, double> &ExtrapolationData,
                  burst_type_t         BurstType);

    bool DataExtractionFinished(void);

    bool Sampling(size_t MaxSamples);

    vector<const Point*>& GetClusteringPoints(void)
    {
      if (NormalizeData && !Normalized)
      {
        Normalize();
      }

      return (vector<const Point*>&) ClusteringBursts;
    };

    vector<const Point*>& GetCompletePoints(void) { return (vector<const Point*>&) CompleteBursts; }

    vector<CPUBurst*>& GetAllBursts(void)         { return AllBursts;         }
    vector<CPUBurst*>& GetCompleteBursts(void)    { return CompleteBursts;    }
    vector<CPUBurst*>& GetClusteringBursts(void)  { return ClusteringBursts;  }
    vector<CPUBurst*>& GetFilteredBursts(void)    { return FilteredBursts;    }
    vector<CPUBurst*>& GetMissingDataBursts(void) { return MissingDataBursts; }

#ifdef HAVE_SQLITE3
    BurstsDB::iterator GetAllBursts_begin(void)       { return AllBurstsDB.all_bursts_begin(); };
    BurstsDB::iterator GetAllBursts_end(void)         { return AllBurstsDB.all_bursts_end(); };
    BurstsDB::iterator GetCompleteBursts_begin(void)  { return AllBurstsDB.complete_bursts_begin(); };
    BurstsDB::iterator GetCompleteBursts_end(void)    { return AllBurstsDB.complete_bursts_end(); };
#endif

    /* Clustering points modifiers */
    void  Normalize(void);
    void  ScalePoints(void);
    void  MeanAdjust(void);
    void  BaseChange(vector< vector<double> >& BaseChangeMatrix);

    /* Size getters */
    size_t GetAllBurstsSize(void) const;
    size_t GetCompleteBurstsSize(void) const;
    size_t GetClusteringBurstsSize(void) const;
    size_t GetFilteredBurstsize(void) const;
    size_t GetTraceDataSize(void) const;

    /* Distribution Managers */
    void SetNumberOfTasks(size_t NumberOfTasks);
    void SetMaster(bool Master)               { this->Master = Master; };
    void SetReadAllTasks(bool ReadAllTasks)   { this->ReadAllTasks = ReadAllTasks; };
    void SetTasksToRead(set<int> TasksToRead) { this->TasksToRead = TasksToRead; };

    size_t         GetClusteringDimensionsCount(void)    { return ClusteringDimensions; };
    size_t         GetExtrapolationDimensionsCount(void) { return ExtrapolationDimensions; };

    vector<double>& GetMinValues(void) { return MinValues; };
    vector<double>& GetMaxValues(void) { return MaxValues; };

    // bool ComputeClusterStatistics(vector<cluster_id_t>& Cluster_IDs, );

    bool FlushPoints(ostream&             str,
                     vector<cluster_id_t> Cluster_IDs = vector<cluster_id_t> (0),
                     DataPrintSet         WhatToPrint = PrintAllBursts);

    /* DEBUG */
    void PrintPoints(void);

    void PrintTraceDataInformation(void);

  private:

    bool SampleSingleTask(vector<CPUBurst*>& TaskBursts, size_t NumSamples);

    void SetTasksToRead();

    bool ReadThisTask(task_id_t Task);

    template <typename T>
    bool GenericFlushPoints(ostream&             str,
                            T                    start,
                            T                    end,
                            size_t               DataSize,
                            vector<cluster_id_t> IDs);

};

template <typename T>
bool TraceData::GenericFlushPoints(ostream&             str,
                                   T                    begin,
                                   T                    end,
                                   size_t               DataSize,
                                   vector<cluster_id_t> IDs)
{
  T BurstsIterator;

  ParametersManager *Parameters = ParametersManager::GetInstance();

  vector<string> ClusteringParametersNames;
  vector<string> ExtrapolationParametersNames;

  vector<bool>   ClusteringParametersPrecision;
  vector<bool>   ExtrapolationParametersPrecision;

  size_t ClusteringBurstsCounter, TotalPoints;

  bool   Unclassified = false;

  /* Check if cluster_ids is empty! */
  if (IDs.size() == 0)
  {
    Unclassified = true;
    // Cluster_IDs = vector<cluster_id_t> (ClusteringBursts.size(), UNCLASSIFIED);

    /* DEBUG
    cout << "Clustering Bursts = " << ClusteringBursts.size() << endl;
    cout << "Complete Bursts = " << CompleteBursts.size() << endl; */
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
    str << ", d_" << ClusteringParametersNames[i];
  }

  for (INT32 i = 0; i < ClusteringParametersNames.size(); i++)
  {
    str << ", n_" << ClusteringParametersNames[i];
  }

  for (INT32 i = 0; i < ExtrapolationParametersNames.size(); i++)
  {
    str << ", x_" << ExtrapolationParametersNames[i];
  }

  str << ",ClusterID" << endl;

  system_messages::show_progress("Writing point to disc", 0, DataSize);
  for (BurstsIterator  = begin, ClusteringBurstsCounter = 0, TotalPoints = 0;
       BurstsIterator != end;
     ++BurstsIterator)
  {
    cluster_id_t CurrentClusterId;
    CPUBurst*    CurrentBurst;

    /* DEBUG
    cout << "Bursts = " << TotalPoints;
    cout << " Dimensions = " << (*BurstsIterator)->size();
    cout << " Burst Type = " << (*BurstsIterator)->GetBurstType();
    */
    ++TotalPoints;

    CurrentBurst = (*BurstsIterator);

#ifdef HAVE_SQLITE3
    if (CurrentBurst == NULL)
    {
      SetError(true);
      SetErrorMessage(AllBurstsDB.GetLastError());
      return false;
    }
#endif

    switch((*BurstsIterator)->GetBurstType())
    {
      case CompleteBurst:

        if (Unclassified)
        {
          CurrentClusterId = UNCLASSIFIED+PARAVER_OFFSET;
        }
        else
        {
          CurrentClusterId = IDs[ClusteringBurstsCounter]+PARAVER_OFFSET;
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

    CurrentBurst->Print(str,
                        ClusteringParametersPrecision,
                        ExtrapolationParametersPrecision,
                        CurrentClusterId);

#ifdef HAVE_SQLITE3
    /* When working with the data base, the bursts must be erased */
    delete CurrentBurst;
#endif

    system_messages::show_progress("Writing point to disc", TotalPoints, DataSize);
  }
  system_messages::show_progress_end("Writing point to disc", DataSize);

  return true;

}

#endif /* _TRACEDATA_HPP_ */
