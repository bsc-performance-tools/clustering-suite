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

#include "ClusteredTraceGenerator.hpp"


#include <climits>
#include <cstdio>
#include <cstring>
#include <cerrno>

#include <string>

#include <vector>
using std::vector;

class ClusteredTRFGenerator: public ClusteredTraceGenerator
{
  public:
    static const INT32 NO_MORE_CLUSTERS             = INT_MIN;
    static const INT32 READ_ERROR                   = INT_MIN + 1;

  private:
    bool          PrintClusterBlocks;

  public:
    ClusteredTRFGenerator(string  InputTraceName,
                          string  OutputTraceName,
                          bool    PrintClusterBlocks = false);

    ~ClusteredTRFGenerator(void);

    ReconstructorType GetType(void) { return TRF; };

    void SetPrintClusterBlocks (bool PrintClusterBlocks)
    {
      this->PrintClusterBlocks = PrintClusterBlocks;
    }

    bool SetEventsToDealWith (set<event_type_t>& EventsToDealWith);

    template <typename T>
    bool Run (T                     begin,
              T                     end,
              vector<cluster_id_t>& IDs,
              set<cluster_id_t>&    DifferentIDs,
              bool                  MinimizeInformation = false);

    bool Run(vector<CPUBurst*>&    ClusteringBursts,
             vector<cluster_id_t>& IDs,
             set<cluster_id_t>&    DifferentIDs,
             bool                  MinimizeInformation = false);

  private:
    INT32 GetNextClusterId(void);

    bool CheckFirstBlockDefinition(char*              Buffer,
                                   set<cluster_id_t>& DifferentIDs);


    bool PrintClusteredBurst(FILE        *Trace,
                             long         TaskId,
                             long         ThreadId,
                             double       BurstDuration,
                             cluster_id_t ClusterId,
                             bool         ReplaceDurations = false);

    void PrepareClusterIDsVector(vector<cluster_id_t>& ClusterIDs,
                                 set<cluster_id_t>&    DifferentIDs,
                                 cluster_id_t&         MaxIDUsed);

    string GetClusterName(cluster_id_t ID);
};

typedef ClusteredTRFGenerator* ClusteredTRFGenerator_t;

template <typename T>
bool ClusteredTRFGenerator::Run (T                     begin,
                                 T                     end,
                                 vector<cluster_id_t>& IDs,
                                 set<cluster_id_t>&    DifferentIDs,
                                 bool                  MinimizeInformation)
{
  bool               InIdleBlock = false;
  char               Buffer[256], AuxBuffer[256];
  size_t             BufferSize = sizeof(Buffer);
  INT32              TaskId, ThreadId, FirstsBursts = -1;
  double             BurstDuration;
  UINT64             InternalBurstDuration;
  line_t             CurrentLine;
  size_t             CurrentBurst, TotalBursts;
  bool               FirstBlockDefinitionFound = false;

  /* This map contains all the information required to the reconstruction.
     It is indexed by line to guarantee the trace ordering */
  map<line_t, std::pair<cluster_id_t, timestamp_t> >           CompleteIDs;
  /* Iterator values:

     + 'first'         : the line (not use, just for the map ordering)

     + 'second.first'  : the Cluster ID associated to the region (in this case
                          the parser is an events parser, not burst parser)

     + 'second.second' : the End Time of the current region
   */
  map<line_t, std::pair<cluster_id_t, timestamp_t> >::iterator BurstsInfo;

  if (fseeko(this->InputTraceFile, 0, SEEK_SET) == -1)
  {
    SetError(true);
    SetErrorMessage("unable to seek initial position on input trace file",
                    strerror(errno));
    return false;
  }

  /* Create the single cluster IDs map */
  size_t CurrentClusteringBurst = 0;

  for (T Burst = begin; Burst != end; ++Burst)
  {
    switch ( (*Burst)->GetBurstType() )
    {
      case CompleteBurst:
        // CompleteIDs.push_back (IDs[CurrentClusteringBurst] + PARAVER_OFFSET);
        CompleteIDs[ (*Burst)->GetLine() ] =
          std::make_pair (IDs[CurrentClusteringBurst] + PARAVER_OFFSET,
                          (*Burst)->GetEndTime());
        CurrentClusteringBurst++;
        break;
      case DurationFilteredBurst:
        // CompleteIDs.push_back (DURATION_FILTERED_CLUSTERID + PARAVER_OFFSET);
        CompleteIDs[ (*Burst)->GetLine() ] =
          std::make_pair (DURATION_FILTERED_CLUSTERID + PARAVER_OFFSET,
                          (*Burst)->GetEndTime());
        break;
      case RangeFilteredBurst:
        // CompleteIDs.push_back (RANGE_FILTERED_CLUSTERID + PARAVER_OFFSET);
        CompleteIDs[ (*Burst)->GetLine() ] =
          std::make_pair (RANGE_FILTERED_CLUSTERID + PARAVER_OFFSET,
                          (*Burst)->GetEndTime());
        break;
      case MissingDataBurst:
        // CompleteIDs.push_back (MISSING_DATA_CLUSTERID + PARAVER_OFFSET);
        CompleteIDs[ (*Burst)->GetLine() ] =
          std::make_pair (MISSING_DATA_CLUSTERID + PARAVER_OFFSET,
                          (*Burst)->GetEndTime());
        break;
      default:
        /* This option should never happen */
        SetErrorMessage ("incorrect burst type when generating Paraver trace");
        SetError (false);
        return false;
    }
#ifdef HAVE_SQLITE3
    delete (*Burst);
#endif
  }

  // CurrentBurst = 0;
  BurstsInfo  = CompleteIDs.begin();
  CurrentLine = 0;
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
                                                             DifferentIDs);

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

        /* DEBUG
        cout << "CurrentLine = " << CurrentLine << " - ";
        cout << "CurrentClusterLine = " << Bursts[CurrentBurst]->GetLine() << endl;
        */
        if (BurstsInfo  == CompleteIDs.end())
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
          if (CurrentLine != (*BurstsInfo).first)
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
                                     (*BurstsInfo).second.first+PARAVER_OFFSET))
            {
              return false;
            }
            ++BurstsInfo;
          }
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
