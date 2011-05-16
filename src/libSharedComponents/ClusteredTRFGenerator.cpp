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

#include <SystemMessages.hpp>

#include "ClusteredTRFGenerator.hpp"
#include <EventEncoding.h>

#include <cstring>
#include <cerrno>

#include <iostream>
using std::cout;
using std::endl;

#include <vector>
using std::vector;

#include <algorithm>
using std::sort;


ClusteredTRFGenerator::ClusteredTRFGenerator(string       InputTraceName,
                                             string       OutputTraceName,
                                             bool         PrintClusterBlocks)
:ClusteredTraceGenerator(InputTraceName, OutputTraceName)
{
  if (GetError())
    return;
  
  this->PrintClusterBlocks = PrintClusterBlocks;
}


ClusteredTRFGenerator::~ClusteredTRFGenerator(void)
{
}

bool ClusteredTRFGenerator::SetEventsToDealWith (set<event_type_t>& EventsToDealWith)
{
  
  SetError(true);
  SetErrorMessage("TRF traces doesn't permit parsing based on events");
  
  return false;
}

bool ClusteredTRFGenerator::Run(vector<CPUBurst*>&    Bursts,
                                vector<cluster_id_t>& IDs,
                                size_t                NumberOfClusters,
                                bool                  MinimizeInformation)
{
  vector<cluster_id_t> CompleteIDs;
  
  bool               InIdleBlock = false;
  char               Buffer[256], AuxBuffer[256];
  size_t             BufferSize = sizeof(Buffer);
  INT32              TaskId, ThreadId, FirstsBursts = -1;
  double             BurstDuration;
  UINT64             InternalBurstDuration;
  line_t             CurrentLine;
  size_t             CurrentBurst, TotalBursts;
  bool               FirstBlockDefinitionFound = false;

  if (fseeko(this->InputTraceFile, 0, SEEK_SET) == -1)
  {
    SetError(true);
    SetErrorMessage("unable to seek initial position on input trace file",
                    strerror(errno));
    return false;
  }

  /* Create the single cluster IDs vector */
  size_t CurrentClusteringBurst = 0;
  for (size_t i = 0; i < Bursts.size(); i++)
  {
    switch(Bursts[i]->GetBurstType())
    {
      case CompleteBurst:
        CompleteIDs.push_back(IDs[CurrentClusteringBurst]);
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

  CurrentBurst = 0;
  CurrentLine  = 0;
  while (true)
  {
    if (fgets(Buffer, BufferSize, InputTraceFile) == NULL)
    { /* End Of File reachead (or error...) */
      break;
    }
    CurrentLine++;

    if (!FirstBlockDefinitionFound)
    { /* Search for the first 'block definition' record */
      FirstBlockDefinitionFound = CheckFirstBlockDefinition (Buffer,
                                                             NumberOfClusters);
      
      fprintf(OutputTraceFile, "%s", Buffer);
      continue; /* Do not look for CPU bursts */
    }
    else
    {
      if (sscanf(Buffer,
               "\"CPU burst\" { %d, %d, %le };;\n",
               &TaskId,
               &ThreadId,
               &BurstDuration) == 3)
      {
        
        if (FirstsBursts < TaskId)
        {
          FirstsBursts++;
          // fprintf(OutputTraceFile, "%s", Buffer);
          // continue;
        }

        /* DEBUG */
        cout << "CurrentLine = " << CurrentLine << " - ";
        cout << "CurrentClusterLine = " << Bursts[CurrentBurst]->GetLine() << endl;
        
        if (CurrentLine != Bursts[CurrentBurst]->GetLine())
        {
          if (!PrintClusteredBurst(OutputTraceFile,
                                   TaskId,
                                   ThreadId,
                                   BurstDuration,
                                   MISSING_DATA_CLUSTERID+PARAVER_OFFSET))
          {
            return false;
          }
        }
        else
        {
          if (!PrintClusteredBurst(OutputTraceFile,
                                   TaskId,
                                   ThreadId,
                                   BurstDuration,
                                   CompleteIDs[Bursts[CurrentBurst]->GetInstance()]+PARAVER_OFFSET))
          {
            return false;
          }
          CurrentBurst++;
        }
      }
      else
        fprintf(OutputTraceFile, "%s", Buffer);
    }
  }

  /* TESTING
  CurrentClusterId = GetNextClusterId();
  while (CurrentClusterId != ClusteredTRFGenerator::NO_MORE_CLUSTERS)
  {
    cout << "ClusterID: " << CurrentClusterId << endl;
    CurrentClusterId = GetNextClusterId();

    if (CurrentClusterId == ClusteredTRFGenerator::READ_ERROR)
    {
      cout << "ERROR!!!!" << endl;
      break;
    }
  }
  */

  return true;
}

/* PrintClusteredBurst *******************************************************/
bool
ClusteredTRFGenerator::CheckFirstBlockDefinition(char  *Buffer,
                                                 size_t NumberOfClusters)
{
  char AuxBuffer[256];
  bool Result = false;

  if (sscanf(Buffer,
             "\"block definition\" {  %[^\n]\n",
             AuxBuffer) == 1)
  { /* Print Cluster ID event description records */
    Result = true;

    fprintf(OutputTraceFile, "\"user event type definition\" { 90000001, ");
    fprintf(OutputTraceFile, "\"Cluster ID\", 9 };;\n");
    fprintf(OutputTraceFile, "\"user event value definition\" { 90000001, ");
    fprintf(OutputTraceFile, " 0, \"End\" };;\n");

    /* Cluster for missing data bursts */
    fprintf(OutputTraceFile, "\"user event value definition\" { 90000001, ");
    fprintf(OutputTraceFile,
            " %d, \"Missing Data\"};;\n",
            MISSING_DATA_CLUSTERID+PARAVER_OFFSET);


     /* Cluster for duration filtered bursts */
    fprintf(OutputTraceFile, "\"user event value definition\" { 90000001, ");
    fprintf(OutputTraceFile,
            " %d, \"Dur. Filtered\"};;\n",
            DURATION_FILTERED_CLUSTERID+PARAVER_OFFSET);
    
     /* Cluster for range filtered bursts */
    fprintf(OutputTraceFile, "\"user event value definition\" { 90000001, ");
    fprintf(OutputTraceFile,
            " %d, \"Range Filtered\"};;\n",
            RANGE_FILTERED_CLUSTERID+PARAVER_OFFSET);


    /* Cluster for duration filtered bursts */
    fprintf(OutputTraceFile, "\"user event value definition\" { 90000001, ");
    fprintf(OutputTraceFile,
            " %d, \"Th. Filtered\"};;\n",
            THRESHOLD_FILTERED_CLUSTERID+PARAVER_OFFSET);
    
    /* Cluster for noise bursts */
    fprintf(OutputTraceFile, "\"user event value definition\" { 90000001, ");
    fprintf(OutputTraceFile,
            " %d, \"Noise\"};;\n",
            NOISE_CLUSTERID+PARAVER_OFFSET);

    
    for (size_t i = 1; i < NumberOfClusters; i++)
    {
      /*
      if (ClusterInformationVector[i]->GetClusterId() + 
          DataPoint::CLUSTERS_OFFSET ==  DataPoint::DURATION_FILTERED)
      {
        break;
      }
      */

      fprintf(OutputTraceFile,
              "\"user event value definition\" { 90000001, ");
      fprintf(OutputTraceFile,
              "%2d, \"Cluster %d\"};;\n",
              i+PARAVER_OFFSET,
              i);
    }

    fprintf(OutputTraceFile, "\n");
    fflush(OutputTraceFile);

    if (PrintClusterBlocks)
    {
      fprintf(
        OutputTraceFile,
        "\"block definition\" { %d, \"Missing Data Bursts\", \"Cluster\", 0, 0 };;\n",
        MISSING_DATA_CLUSTERID+PARAVER_OFFSET);

      fprintf(
        OutputTraceFile,
        "\"block definition\" { %d, \"Duration Filtered Bursts\", \"Cluster\", 0, 0 };;\n",
        DURATION_FILTERED_CLUSTERID+PARAVER_OFFSET);

      fprintf(
        OutputTraceFile,
        "\"block definition\" { %d, \"Threshold Filtered Bursts\", \"Cluster\", 0, 0 };;\n",
        THRESHOLD_FILTERED_CLUSTERID+PARAVER_OFFSET);
      
      fprintf(
        OutputTraceFile,
        "\"block definition\" { %d, \"Noise Bursts\", \"Cluster\", 0, 0 };;\n",
        NOISE_CLUSTERID+PARAVER_OFFSET);
      
      for (size_t i = 1; i < NumberOfClusters; i++)
      {

        fprintf(
          OutputTraceFile,
          "\"block definition\" { %2d, \"Cluster %d\", \"Cluster\", 0, 0 };;\n",
          i+PARAVER_OFFSET,
          i);
      }

      fprintf(OutputTraceFile, "\n");
      fflush(OutputTraceFile);
    }
  }

  return Result;
}

/* PrintClusteredBurst *******************************************************/
bool
ClusteredTRFGenerator::PrintClusteredBurst(FILE        *Trace,
                                           long         TaskId,
                                           long         ThreadId,
                                           double       BurstDuration,
                                           cluster_id_t ClusterId,
                                           bool         ReplaceDurations)
{

  if (fprintf(Trace,
              "\"user event\" { %ld, %ld, 90000001, %ld };;\n",
              TaskId,
              ThreadId,
              (long) ClusterId) < 0)
  {
    SetError(true);
    SetErrorMessage("error writing opening cluster event on output trace");
    return false;
  }

  if (PrintClusterBlocks)
  {
    if (fprintf(Trace,
                "\"block begin\" { %ld, %ld, %ld };;\n",
                TaskId,
                ThreadId,
                (long) ClusterId) < 0)
    {
      SetError(true);
      SetErrorMessage("error writing cluster block begin on output trace");
      return false;
    }
  }

  if (ReplaceDurations)
  {
    if (fprintf(Trace,
                "\"CPU burst\" { %ld, %ld, %ld };;\n",
                TaskId,
                ThreadId,
                (long) ClusterId) < 0)
    {
      SetError(true);
      SetErrorMessage("error writing CPU burst on output trace");
      return false;
    }
  }
  else
  {
    if (fprintf(Trace,
                "\"CPU burst\" { %ld, %ld, %.06f };;\n",
                TaskId,
                ThreadId,
                BurstDuration) < 0)
    {
      SetError(true);
      SetErrorMessage("error writing CPU burst on output trace");
      return false;
    }
  }

  if (PrintClusterBlocks)
  {
    if (fprintf(Trace,
                "\"block end\" { %ld, %ld, %ld };;\n",
                TaskId,
                ThreadId,
                (long) ClusterId) < 0)
    {
      SetError(true);
      SetErrorMessage("error writing cluster block end on output trace");
      return false;
    }
  }

  if (fprintf(Trace,
              "\"user event\" { %ld, %ld, 90000001, 0 };;\n",
              TaskId,
              ThreadId) < 0)
  {
    SetError(true);
    SetErrorMessage("error writing closing cluster event on output trace");
    return false;
  }

  return true;
}
