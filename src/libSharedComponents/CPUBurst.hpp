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
#ifndef DATAPOINT_H
#define DATAPOINT_H

#include "trace_clustering_types.h"

#include <Error.hpp>
using cepba_tools::Error;

#include <Point.hpp>

#include <sys/stat.h>
#include <stdio.h>
#include <string>
#include <errno.h>
#include <math.h>
#include <float.h>

#include <ios>
using std::ios;

#include <vector>
using std::vector;

#include <map>
using std::map;

#include <iostream>
using std::ostream;
using std::cout;
using std::endl;
using std::fixed;

#define FIELDS_NUM_CSV 7
#define INSTANCE_CSV   "# Instance"
#define TASKID_CSV     "TaskId"
#define THREADID_CSV   "ThreadId"
#define BEGINTIME_CSV  "Begin_Time"
#define ENDTIME_CSV    "End_Time"
#define DURATION_CSV   "Duration"
#define LINE_CSV       "Line"
#define CLUSTERID_CSV  "ClusterID"

class CPUBurst: public Point, public Error
{
  protected:
    task_id_t    TaskId;
    thread_id_t  ThreadId;

    line_t       Line;
    timestamp_t  BeginTime;
    timestamp_t  EndTime;

    duration_t   Duration; /* Each point has always the (raw) duration */

    burst_type_t BurstType;

    vector<double>      RawDimensions;
    map<size_t, double> ExtrapolationDimensions; /* Key:   Position
                                                 * Value: DimensionValue */

    bool ToClassify;

  public:

    typedef map<INT32, double>::iterator iterator;
    typedef map<INT32, double>::const_iterator const_iterator;
    typedef std::pair<const size_t, double> pair;

    CPUBurst(task_id_t           TaskId,
             thread_id_t         ThreadId,
             line_t              Line,
             timestamp_t         BeginTime,
             timestamp_t         EndTime,
             duration_t          Duration,
             vector<double>&     ClusteringRawData,
             vector<double>&     ClusteringProcessedData,
             map<size_t, double>& ExtrapolationData,
             burst_type_t        BurstType,
             bool                ToClassify = false);

    CPUBurst(instance_t          Instance,
             task_id_t           TaskId,
             thread_id_t         ThreadId,
             line_t              Line,
             timestamp_t         BeginTime,
             timestamp_t         EndTime,
             duration_t          Duration,
             vector<double>&     ClusteringRawData,
             vector<double>&     ClusteringProcessedData,
             map<size_t, double>& ExtrapolationData,
             burst_type_t        BurstType,
             bool                ToClassify = false);

    CPUBurst(void);
    ~CPUBurst(void);

    bool Classified(void);

    void       SetDuration(duration_t Duration) { this->Duration = Duration; };
    duration_t GetDuration(void)                { return Duration; };

    task_id_t   GetTaskId(void)    { return TaskId; };
    thread_id_t GetThreadId(void)  { return ThreadId; };
    line_t      GetLine(void)      { return Line; };
    timestamp_t GetBeginTime(void) { return BeginTime; };
    timestamp_t GetEndTime(void)   { return EndTime; };

    // bool   WillBeClusterized(void) { return ToBeClusterized; }; //

    vector<double>& GetRawDimensions(void) { return RawDimensions; };
    double          GetRawDimension(size_t Index);

    size_t GetExtrapolationDimensionsCount(void) const;

    map<size_t, double>& GetExtrapolationDimensions(void) { return ExtrapolationDimensions; };

    burst_type_t GetBurstType(void) const;

    bool Normalize(const vector<double>& MaxValues,
                   const vector<double>& MinValues);

    bool Scale(vector<double>& Mean, vector<double>& RMS);

    bool MeanAdjust(vector<double>& DimensionsAverage);

    bool BaseChange(vector< vector<double> >& BaseChangeMatrix);

    bool Print(ostream& str,
               vector<bool>& ClusteringParametersPrecision,
               vector<bool>& ExtrapolationParametersPrecision,
               cluster_id_t ClusterId);

    friend ostream &operator<<(ostream &str, CPUBurst const &Burst);

    friend ostream &PrintFullInformation (ostream &str, CPUBurst const &Burst);

    friend ostream &PrintBasicInformation (ostream &str, CPUBurst const &Burst);

    /* DEBUG */
    void PrintBurst(void);

    static string BurstTypeStr(burst_type_t T);
};

inline ostream &operator<<(ostream &str, const CPUBurst &Burst)
{

  str << Burst.Instance << ",";

  str << Burst.Line     << ",";
  str << Burst.TaskId   << ",";
  str << Burst.ThreadId << ",";

  str << Burst.BeginTime << ",";
  str << Burst.EndTime   << ",";

  INT32                     CurrentPosition = 0;
  // DataPoint::const_iterator ExtrapolationDimensionsIterator;

  str.setf(ios::fixed,ios::floatfield);
  str.precision(0);

  return str;
}

/* Classes for CPUBurst comparisons */
class BeginTimeCompare
{
  public:
    bool operator()(CPUBurst* P1, CPUBurst* P2)
    {
      if (P1->GetBeginTime() < P2->GetBeginTime())
        return true;
      else if (P1->GetBeginTime() == P2->GetBeginTime())
        return (P1->GetLine() < P2->GetLine());
        // return (P1->GetInstance() < P2->GetInstance());
        // return (P1->GetEndTime() < P2->GetEndTime());
        // return (P1->GetTaskId() < P2->GetTaskId());
      else
        return false;
    };
};

class EndTimeCompare
{
  public:
    bool operator()(CPUBurst* P1, CPUBurst* P2)
    {
      if (P1->GetEndTime() < P2->GetEndTime())
        return true;
      else if (P1->GetEndTime() == P2->GetEndTime())
        return (P1->GetTaskId() < P2->GetTaskId());
      else
        return false;
    };
};

class InstanceNumCompare
{
  public:
    bool operator()(CPUBurst* P1, CPUBurst* P2)
    {
      if (P1->GetInstance() < P2->GetInstance())
        return true;
      else
        return false;
    }
};

class LineCompare
{
  public:
    bool operator()(CPUBurst* P1, CPUBurst* P2)
    {
      if (P1->GetLine() < P2->GetLine())
        return true;
      else
        return false;
    }
};

#ifndef DATAPOINT_SEPARATOR
#define DATAPOINT_SEPARATOR
// char DataPoint::Separator = ',';
#endif

/* Functors to sort the points */
class ParaverBurstTaskOrder
{
  public:
    bool operator()(CPUBurst* P1, CPUBurst* P2)
    {
      if (P1->GetTaskId() < P2->GetTaskId())
        return true;
      else if (P1->GetTaskId() > P2->GetTaskId())
        return false;
      else
        return (P1->GetBeginTime() < P2->GetBeginTime());
    };
};

class DimemasBurstTaskOrder
{
  public:
    bool operator()(CPUBurst* P1, CPUBurst* P2)
    {
      if (P1->GetTaskId() < P2->GetTaskId())
        return true;
      else if (P1->GetTaskId() > P2->GetTaskId())
        return false;
      else
        return (P1->GetLine() < P2->GetLine());
    };
};

#endif
