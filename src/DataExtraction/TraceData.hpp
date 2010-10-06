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

#include <trace_clustering_types.h>

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
    
    vector<CPUBurst*> ClusteringBursts;
    vector<CPUBurst*> FilteredBursts;
    vector<CPUBurst*> MissingDataBursts;

    /* Parameters parsing */
    duration_t         DurationFilter;
    ParametersManager *Parameters;

    /* Sampling containers */
    vector<CPUBurst*> TrainBursts;
    vector<INT32>     PointsPerTask;

    size_t ClusteringDimensions;
    size_t ExtrapolationDimensions;
    
    bool NormalizeData;
    bool Normalized;

    /* Normalization and statistics attributes */
    bool                Used;      /* Useful to initialize Max & Min arrays */
    vector<double>      MaxValues; /* Maximum and minimum values for each */
    vector<double>      MinValues; /* dimension used on the clustering    */

    vector<double>      SumValues; /* Sumatory of each clustering dimension */
      
    vector<UINT64>      MaxInstances; /* Instances containing the max and min */
    vector<UINT64>      MinInstances; /* values */

  public:

    typedef vector<CPUBurst*>::iterator iterator;

    static TraceData* GetInstance(void);

    /*
    void Initialize(INT32 ClusteringDimensions,
                    INT32 ExtrapolationDimensions,
                    bool  Normalize);
    */

    /*
    bool NewBurst(INT32               TaskId,
                  INT32               ThreadId,
                  UINT64              Line,
                  UINT64              BeginTime,
                  UINT64              EndTime,
                  UINT64              BurstDuration,
                  vector<double>&     ClusteringRawData,
                  vector<double>&     ClusteringProcessedData,
                  map<INT32, double>& ExtrapolationData,
                  burst_type_t        PointType);
    */
    
    bool NewBurst(task_id_t                         TaskId,
                  thread_id_t                       ThreadId,
                  line_t                            Line,
                  timestamp_t                       BeginTime,
                  timestamp_t                       EndTime,
                  duration_t                        BurstDuration,
                  map<event_type_t, event_value_t>& EventsData,
                  bool                              Clustering = true);

    vector<const Point*>& GetClusteringPoints(void) 
    { 
      if (NormalizeData && Normalized == false)
      {
        Normalize();
      }
      return (vector<const Point*>&) ClusteringBursts;
    };

    /* Clustering points modifiers */
    void  Normalize(void);
    void  ScalePoints(void);
    void  MeanAdjust(void);
    void  BaseChange(vector< vector<double> >& BaseChangeMatrix);

    /* Size getters */
    size_t GetClusteringBurstsSize(void) const;
    size_t GetTrainBurstsSize(void) const;
    size_t GetFilteredBurstsize(void) const;
    size_t GetTraceDataSize(void) const;

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

    bool FlushPoints(ostream& str, vector<cluster_id_t> Cluster_IDs = vector<cluster_id_t> (0));


    /*
    bool FlushClusterSequences(ostream&  str,
                               bool      FlushNoisePoints = true); */

      
    /* DEBUG */
    void PrintPoints(void);
    void PrintTraceDataInformation(void);
};


#endif /* _TRACEDATA_HPP_ */
