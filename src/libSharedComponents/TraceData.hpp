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

  $URL:: https://svn.bsc.#$:  File
  $Rev:: 20               $:  Revision of last commit
  $Author:: jgonzale      $:  Author of last commit
  $Date:: 2010-03-09 17:1#$:  Date of last commit

\* -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=- */

#ifndef _TRACEDATA_HPP_
#define _TRACEDATA_HPP_

#include "trace_clustering_types.h"

#include <ParametersManager.hpp>
#include <Error.hpp>
using cepba_tools::Error;

#include "CPUBurst.hpp"

#include <iostream>
using std::ostream;
using std::cout;
using std::endl;

#include <map>
using std::map;

#include<set>
using std::set;

using std::pair;

class TraceData: public Error
{
  protected:

    static TraceData* Instance;
    TraceData(void);

    /*
    static char  AminoacidTranslation[];
    static INT32 AminoacidTranslationSize;
    */

    /* The main containers */
    vector<CPUBurst*> AllBursts;
    vector<CPUBurst*> CompleteBursts;
    vector<CPUBurst*> ClusteringBursts;
    vector<CPUBurst*> FilteredBursts;
    vector<CPUBurst*> MissingDataBursts;

    /* Parameters parsing */
    duration_t         DurationFilter;
    ParametersManager *Parameters;

    /* Sampling containers */
    size_t            NumberOfTasks;
    vector<size_t>    PointsPerTask;

    size_t ClusteringDimensions;
    size_t ExtrapolationDimensions;
    
    bool NormalizeData;
    bool Normalized;

    /* Distributed version management */
    bool     Distributed;
    bool     Master;
    INT32    MyRank;
    INT32    TotalRanks;
    bool     ReadAllTasks;
    set<int> TasksToRead;


    /* Normalization and statistics attributes */
    bool                Used;      /* Useful to initialize Max & Min arrays */
    vector<double>      MaxValues; /* Maximum and minimum values for each */
    vector<double>      MinValues; /* dimension used on the clustering    */

    vector<double>      SumValues; /* Sumatory of each clustering dimension */
      
    vector<instance_t>  MaxInstances; /* Instances containing the max and min */
    vector<instance_t>  MinInstances; /* values */

  public:

    typedef vector<CPUBurst*>::iterator iterator;

    static TraceData* GetInstance(void);
    
    bool NewBurst(task_id_t                         TaskId,
                  thread_id_t                       ThreadId,
                  line_t                            Line,
                  timestamp_t                       BeginTime,
                  timestamp_t                       EndTime,
                  duration_t                        BurstDuration,
                  map<event_type_t, event_value_t>& EventsData,
                  bool                              toCluster = true);

    vector<const Point*>& GetClusteringPoints(void) 
    { 
      if (NormalizeData && Normalized == false)
      {
        Normalize();
      }
      return (vector<const Point*>&) ClusteringBursts;
    };

    vector<CPUBurst*>& GetAllBursts(void)         { return AllBursts;     };
    vector<CPUBurst*>& GetCompleteBursts(void)    { return CompleteBursts; };
    vector<CPUBurst*>& GetClusteringBursts(void)  { return ClusteringBursts; };
    vector<CPUBurst*>& GetFilteredBursts(void)    { return FilteredBursts; };
    vector<CPUBurst*>& GetMissingDataBursts(void) { return MissingDataBursts; };

    /* Clustering points modifiers */
    void  Normalize(void);
    void  ScalePoints(void);
    void  MeanAdjust(void);
    void  BaseChange(vector< vector<double> >& BaseChangeMatrix);

    /* Size getters */
    size_t GetCompleteBurstsSize(void) const;
    size_t GetClusteringBurstsSize(void) const;
    size_t GetFilteredBurstsize(void) const;
    size_t GetTraceDataSize(void) const;

    /* Distribution Managers */
    void SetNumberOfTasks(size_t NumberOfTasks);
    void SetMaster(bool Master)               { this->Master = Master; };
    void SetReadAllTasks(bool ReadAllTasks)   { this->ReadAllTasks = ReadAllTasks; };
    void SetTasksToRead(set<int> TasksToRead) { this->TasksToRead = TasksToRead; };

    vector<string> GetClusteringParametersNames;
    vector<string> GetExtrapolationParametersNames;

    vector<bool>   GetClusteringParametersPrecision;
    vector<bool>   GetExtrapolationParametersPrecision;
    
    /*
    Point* operator[](const size_t Index)
    {
      if (Index >= 0 && Index < ClusteringBursts.size())
      {
        return (Point*) ClusteringBursts[Index];
      }
      else
      {
        return NULL;
      }
    }
    */
    
  
    vector<double>& GetMinValues(void) { return MinValues; };
    vector<double>& GetMaxValues(void) { return MaxValues; };

    // bool ComputeClusterStatistics(vector<cluster_id_t>& Cluster_IDs, );

    bool FlushPoints(ostream&             str,
                     vector<cluster_id_t> Cluster_IDs = vector<cluster_id_t> (0),
                     bool                 AllPoints   = true);


    /*
    bool FlushClusterSequences(ostream&  str,
                               bool      FlushNoisePoints = true); */

      
    /* DEBUG */
    void PrintPoints(void);
    void PrintTraceDataInformation(void);
    
  private:
    void SetTasksToRead();
    bool ReadThisTask(task_id_t Task);
};


#endif /* _TRACEDATA_HPP_ */
