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

#include <SystemMessages.hpp>
using cepba_tools::system_messages;

#include "ClusteredStatesPRVGenerator.hpp"
#include <ParaverColors.h>

#include <unistd.h>
#include <cstring>

#include <algorithm>
using std::sort;
using std::merge;

#include <iostream>
#include <fstream>
#include <sstream>
using std::ifstream;
using std::ofstream;
using std::ostringstream;

// #define DEBUG_PRV_OUTPUT 0

#define RUNNING_STATE 1
#define CLUSTERS_EVENTS_TXT \
"\n"\
"EVENT_TYPE\n"\
"9\t90000001\tCluster ID\n"\
"VALUES\n"\
"0\tEnd\n"\
"1\tMissing Data\n"

ClusteredStatesPRVGenerator::ClusteredStatesPRVGenerator(string  InputTraceName,
                                                       string  OutputTraceName)
:ClusteredTraceGenerator(InputTraceName, OutputTraceName)
{
  string::size_type SubstrPos;

  if (GetError())
    return;

  TraceParser = new ParaverTraceParser(InputTraceName, InputTraceFile);

  /* Search for a possible PCF and ROW files */
  PCFPresent = false;
  ROWPresent = false;

  /* Check the PCF/ROW file name */
  SubstrPos = InputTraceName.rfind(".prv");

  if (SubstrPos == string::npos)
  {
    InputPCFName = InputTraceName+".pcf";
    InputROWName = InputTraceName+".row";
  }
  else
  {
    string InputBaseName;

    InputBaseName = InputTraceName.substr(0, SubstrPos);
    InputPCFName  = InputBaseName+".pcf";
    InputROWName  = InputBaseName+".row";
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
      string OutputBaseName;

      OutputBaseName = OutputTraceName.substr(0, SubstrPos);
      OutputPCFName  = OutputBaseName+".pcf";
    }

    if ((OutputPCFFile = fopen(OutputPCFName.c_str(), "w")) == NULL)
    {
      /* We are unable to open the output PCF file. The PCF managment is
         disabled */
      fclose(InputPCFFile);
      PCFPresent = false;
    }
  }

  FILE* InputROWFile, *OutputROWFile;
  if ((InputROWFile = fopen(InputROWName.c_str(), "r" )) != NULL)
  {
    ROWPresent = true;

    SubstrPos = OutputTraceName.rfind(".prv");

    if (SubstrPos == string::npos)
    {
      OutputROWName = InputTraceName+".row";
    }
    else
    {
      string OutputBaseName;

      OutputBaseName = OutputTraceName.substr(0, SubstrPos);
      OutputROWName  = OutputBaseName+".row";
    }

    if ((OutputROWFile = fopen(OutputROWName.c_str(), "w")) == NULL)
    {
      /* We are unable to open the output PCF file. The PCF managment is
         disabled */
      ROWPresent = false;
    }
  }

  /* DEBUG
  if (PCFPresent)
  {
    printf("PCF FOUND!!!!\n");
  }

  if (ROWPresent)
  {
    printf("ROW FOUND!!!!\n");
  }
  */

  /*
  printf("Input PCF = %s\nOutput PCF = %s\n",
           InputPCFName.c_str(),
           OutputPCFName.c_str());

  printf("Input ROW = %s\nOutput ROW = %s\n",
           InputROWName.c_str(),
           OutputROWName.c_str());
  */

  return;
}

ClusteredStatesPRVGenerator::~ClusteredStatesPRVGenerator(void)
{
  if ( InputPCFFile != NULL)
    fclose(InputPCFFile);

  if ( OutputPCFFile != NULL)
    fclose(OutputPCFFile);

  delete TraceParser;
}

bool ClusteredStatesPRVGenerator::SetEventsToDealWith (set<event_type_t>& EventsToDealWith,
                                                       bool               ConsecutiveEvts)
{
  SetError(true);
  SetErrorMessage("trying to use a semantic guided reconstructor to an event guided parsing");

  return false;
}


bool ClusteredStatesPRVGenerator::Run(vector<CPUBurst*>&    Bursts,
                                      vector<cluster_id_t>& IDs,
                                      set<cluster_id_t>&    DifferentIDs,
                                      bool                  MinimizeInformation)
{
  /* Sort all burst in terms of instance number, to guarantee the positional
     assignment in the IDs vector */
  sort (Bursts.begin(), Bursts.end(), InstanceNumCompare() );

  return Run(Bursts.begin(), Bursts.end(), IDs, DifferentIDs, MinimizeInformation);
}

bool ClusteredStatesPRVGenerator::GenerateOutputPCF(set<cluster_id_t>& DifferentIDs)
{
  char   Buffer[256], AuxBuffer[256];
  size_t BufferSize = sizeof(Buffer);
  bool   ColorsSectionRead     = false;
  bool   ColorsSectionFinished = false;
  bool   FlushBuffer = false;

  vector<cluster_id_t> ClusterIDs;

  cluster_id_t MaxIDUsed;
  PrepareClusterIDsVector(ClusterIDs, DifferentIDs, MaxIDUsed);

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

        /* Print all state colors up to maximum cluster ID */
        for (size_t i = 0; i < MaxIDUsed+PARAVER_OFFSET; i++)
        {
          if (i >= DEF_NB_COLOR_STATE)
          {
            cout << "Warning! Number of clusters greater than available colors" << endl;
            break;
          }
          if (fprintf(OutputPCFFile,
                      "%lu\t{%d,%d,%d}\n",
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

  if (fprintf(OutputPCFFile,              "") < 0)
  {
    unlink(OutputPCFName.c_str());
    return false;
  }

  for (size_t i = 0; i < ClusterIDs.size(); i++)
  {
    fprintf(OutputPCFFile,
            "%d\t%s\n",
            ClusterIDs[i]+PARAVER_OFFSET, /* It needs the offset, because it uses internal numbering */
            GetClusterName(ClusterIDs[i]).c_str());
  }

  return true;
}

bool ClusteredStatesPRVGenerator::BurstCloseAndOpenRecord(ParaverRecord* CurrentRecord,
                                                          cluster_id_t&  ID)
{
  ostringstream Object;

  Object << CurrentRecord->GetTaskId() << "." << CurrentRecord->GetThreadId();

  if ( BurstsToPrint.find(Object.str()) == BurstsToPrint.end())
  {
    return false;
  }

  if (BurstsToPrint[Object.str()].empty())
  {
    return false;
  }

  if (BurstsToPrint[Object.str()].front()->EndTime == CurrentRecord->GetTimestamp())
  {
    BurstInfo *FirstBurstInfo  = BurstsToPrint[Object.str()].front();
    BurstsToPrint[Object.str()].pop_front();

    if (BurstsToPrint[Object.str()].empty())
    {
      BurstsToPrint[Object.str()].push_front(FirstBurstInfo);
      return false;
    }

    BurstInfo *SecondBurstInfo = BurstsToPrint[Object.str()].front();

    if (SecondBurstInfo->BeginTime == CurrentRecord->GetTimestamp())
    {
      delete FirstBurstInfo;
      ID = BurstsToPrint[Object.str()].front()->ID;
      return true;
    }

    BurstsToPrint[Object.str()].push_front(FirstBurstInfo);
  }

  return false;
}

bool ClusteredStatesPRVGenerator::BurstOpeningRecord(ParaverRecord* CurrentRecord, cluster_id_t& ID)
{
  ostringstream Object;

  Object << CurrentRecord->GetTaskId() << "." << CurrentRecord->GetThreadId();

  if ( BurstsToPrint.find(Object.str()) == BurstsToPrint.end())
  {
    return false;
  }

  if (BurstsToPrint[Object.str()].empty())
  {
    return false;
  }

  if (BurstsToPrint[Object.str()].front()->BeginTime == CurrentRecord->GetTimestamp() &&
      !BurstsToPrint[Object.str()].front()->InUse)
  {
    BurstsToPrint[Object.str()].front()->InUse = true;
    ID = BurstsToPrint[Object.str()].front()->ID;
    return true;
  }

  return false;
}

bool ClusteredStatesPRVGenerator::BurstClosingRecord(ParaverRecord* CurrentRecord)
{
  ostringstream Object;

  Object << CurrentRecord->GetTaskId() << "." << CurrentRecord->GetThreadId();

  if ( BurstsToPrint.find(Object.str()) == BurstsToPrint.end())
  {
    return false;
  }

  if (BurstsToPrint[Object.str()].empty())
  {
    return false;
  }

  if (BurstsToPrint[Object.str()].front()->EndTime == CurrentRecord->GetTimestamp())
  {
    BurstInfo *FrontBurstInfo = BurstsToPrint[Object.str()].front();
    delete FrontBurstInfo;
    BurstsToPrint[Object.str()].pop_front();

    return true;
  }

  return false;
}

bool ClusteredStatesPRVGenerator::DuplicatedOpening(
  string                                              TraceObject,
  map<string, std::pair<timestamp_t, cluster_id_t> >& LastBurstsPrinted,
  timestamp_t                                         CurrentTimestamp,
  cluster_id_t                                        CurrentID)
{
  std::pair<timestamp_t, cluster_id_t> LastBurstInformation;
  map<string, std::pair<timestamp_t, cluster_id_t> >::iterator it;


  it = LastBurstsPrinted.find(TraceObject);

  if (it != LastBurstsPrinted.end())
  {
    LastBurstInformation = it->second;

    if (CurrentTimestamp == LastBurstInformation.first &&
        CurrentID        == LastBurstInformation.second)
    {
      return true;
    }
    else
    {
      return false;
    }
  }
  else
  {
    return false;
  }
}

bool ClusteredStatesPRVGenerator::CopyROWFile(void)
{
  ifstream InputROW(InputROWName.c_str(), std::ios::binary);
  if (!InputROW)
  {
    ostringstream WarningMessage;

    WarningMessage << "unable to open input .row file '" << InputROWName << "'";
    SetWarning(true);
    SetWarningMessage(WarningMessage.str());
    return false;
  }

  ofstream OutputROW(OutputROWName.c_str(), std::ios::binary);
  if (!OutputROW)
  {
    ostringstream WarningMessage;

    WarningMessage << "unable to open output .row file '" << OutputROWName << "'";
    SetWarningMessage(WarningMessage.str());
    return false;
  }

  OutputROW << InputROW.rdbuf();

  OutputROW.close();
  InputROW.close();

  return true;
}

void ClusteredStatesPRVGenerator::PrepareClusterIDsVector(vector<cluster_id_t>& ClusterIDs,
                                                          set<cluster_id_t>&    DifferentIDs,
                                                          cluster_id_t&         MaxIDUsed)
{
  set<cluster_id_t>::iterator DifferentIDsIterator;

  MaxIDUsed = NOISE_CLUSTERID;

  if (DifferentIDs.count(DURATION_FILTERED_CLUSTERID) == 0)
  {
    ClusterIDs.push_back(DURATION_FILTERED_CLUSTERID);
  }

  if (DifferentIDs.count(RANGE_FILTERED_CLUSTERID) == 0)
  {
    ClusterIDs.push_back(RANGE_FILTERED_CLUSTERID);
  }

  if (DifferentIDs.count(THRESHOLD_FILTERED_CLUSTERID) == 0)
  {
    ClusterIDs.push_back(THRESHOLD_FILTERED_CLUSTERID);
  }

  if (DifferentIDs.count(NOISE_CLUSTERID) == 0)
  {
    ClusterIDs.push_back(NOISE_CLUSTERID);
  }

  for (DifferentIDsIterator  = DifferentIDs.begin();
       DifferentIDsIterator != DifferentIDs.end();
       ++DifferentIDsIterator)
  {
    ClusterIDs.push_back((*DifferentIDsIterator));
    if ((*DifferentIDsIterator) > MaxIDUsed)
    {
      MaxIDUsed = (*DifferentIDsIterator);
    }
  }
  sort(ClusterIDs.begin(), ClusterIDs.end());

  return;
}

/**
 * Returns the cluster name using the ID
 *
 * \param ID ID value to generate the cluster name
 *
 * \return The name of the cluster, taking into account the special cluster ids
 */
string ClusteredStatesPRVGenerator::GetClusterName(cluster_id_t ID)
{
  ostringstream ClusterName;

  switch (ID)
  {
    case UNCLASSIFIED:
      ClusterName << "Unclassified";
      break;
    case DURATION_FILTERED_CLUSTERID:
      ClusterName << "Duration Filtered";
      break;
    case RANGE_FILTERED_CLUSTERID:
      ClusterName << "Range Filtered";
      break;
    case THRESHOLD_FILTERED_CLUSTERID:
      ClusterName << "Threshold Filtered";
      break;
    case NOISE_CLUSTERID:
      ClusterName << "Noise";
      break;
    default:
      ClusterName << "Cluster " << ID;
      break;
  }

  return ClusterName.str();
}
