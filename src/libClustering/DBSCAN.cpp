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
using cepba_tools::system_messages;

#include "DBSCAN.hpp"

#include "Point.hpp"
#include "Partition.hpp"

#include <cstring>
#include <cmath>
#include <cassert>
#include <cerrno>

#include <cstdarg>

#include <algorithm>
using std::sort;

#include <iostream>
using std::cout;
using std::endl;

#include <fstream>
using std::ofstream;

using std::make_pair;

const string DBSCAN::NAME              = "DBSCAN";
const string DBSCAN::EPSILON_STRING    = "epsilon";
const string DBSCAN::MIN_POINTS_STRING = "min_points";

/*****************************************************************************
 * class DBSCAN implementation                                               *
 ****************************************************************************/

DBSCAN::DBSCAN(map<string, string> ClusteringParameters)
{
  map<string, string>::iterator ParametersIterator;

  /* Epsilon */
  ParametersIterator = ClusteringParameters.find(DBSCAN::EPSILON_STRING);
  if (ParametersIterator == ClusteringParameters.end())
  {
    string ErrorMessage;
    ErrorMessage = "parameter '" + DBSCAN::EPSILON_STRING + "' not found in DBSCAN definition";
    
    SetErrorMessage(ErrorMessage);
    SetError(true);
    return;
  }
  else
  {
    char* err;
    Eps = strtod(ParametersIterator->second.c_str(), &err);

    if (*err)
    {
      string ErrorMessage;
      ErrorMessage = "incorrect value for DBSCAN parameter '"+ DBSCAN::EPSILON_STRING + "'";

      SetErrorMessage(ErrorMessage);
      SetError(true);
      return;
    }
  }

  /* MinPoints */
  ParametersIterator = ClusteringParameters.find(DBSCAN::MIN_POINTS_STRING);
  if (ParametersIterator == ClusteringParameters.end())
  {
    string ErrorMessage;
    ErrorMessage = "parameter '" + DBSCAN::MIN_POINTS_STRING + "' not found in DBSCAN definition";
    
    SetErrorMessage(ErrorMessage);
    SetError(true);
    return;
  }
  else
  {
    char* err;
    MinPoints = strtol(ParametersIterator->second.c_str(), &err, 0);

    if (*err)
    {
      string ErrorMessage;
      ErrorMessage = "incorrect value for DBSCAN parameter '"+ DBSCAN::MIN_POINTS_STRING + "'";

      SetErrorMessage(ErrorMessage);
      SetError(true);
      return;
    }
  }
  
  return;
}

bool DBSCAN::Run(const vector<const Point*>& Data,
                 Partition&                  DataPartition,
                 bool                        SimpleRun)
{
  cluster_id_t   ClusterId = MIN_CLUSTERID;
  Point*         CurrentPoint;
  INT32          CurrentPos, DimensionsCount;
  size_t         ResultingClusters = 0;

  INT64          DataSize, Index;

  if (Data.size() == 0)
  {
    return true;
  }

  /* DEBUG */
  vector<cluster_id_t>& ClusterAssignmentVector = DataPartition.GetAssignmentVector();
  set<cluster_id_t>& DifferentIDs = DataPartition.GetIDs();

  DifferentIDs.insert(NOISE_CLUSTERID);

  if (Data.size() != ClusterAssignmentVector.size())
  {
    ClusterAssignmentVector.clear();

    for (size_t i = 0; i < Data.size(); i++)
    {
      ClusterAssignmentVector.push_back(UNCLASSIFIED);
    }
  }
  
  /* Build KD-Tree */
  BuildKDTree(Data);

  system_messages::show_progress("Clustering points", 0, (int) Data.size());
  Index = 0; // Double counter: total points vs. clustering points!

  for (point_idx i = 0; i < Data.size(); i++)
  {
    system_messages::show_progress("Clustering points", i, (int) Data.size());

    if (ClusterAssignmentVector[i] == UNCLASSIFIED)
    {
      if (ExpandCluster(Data, i, ClusterAssignmentVector, ClusterId))
      {
        DifferentIDs.insert(ClusterId);
        ClusterId++;
      }
    }
  }

  system_messages::show_progress_end("Clustering points", (int) Data.size());

  /* NOISE cluster is not accounted as a cluster */
  DataPartition.NumberOfClusters (DifferentIDs.size()-1);
  DataPartition.HasNoise(true);

  return true;
}

string DBSCAN::GetClusteringAlgorithmName(void) const
{
  ostringstream Result;
  Result << "DBSCAN (Eps=" << Eps << ", MinPoints=" << MinPoints << ")";

  return Result.str();
}

string DBSCAN::GetClusteringAlgorithmNameFile(void) const
{
  ostringstream Result;
  Result << "DBSCAN_Eps_" << Eps << "_MinPoints_" << MinPoints;

  return Result.str();
}

/*
bool
DBSCAN::GetClustersInformation(vector<ClusterInformation_t>& ClusterInfoVector)
{
  list<ClusterInformation_t>::iterator ClusterInformationIteraror;

  if (!ClusteringReady)
  {
    SetError(true);
    SetErrorMessage("clustering not ready");
    return false;
  }

  // ClusterInfoVector.push_back(NoiseClusterInformation);

  for (ClusterInformationIteraror  = ClustersInformation.begin();
       ClusterInformationIteraror != ClustersInformation.end();
       ClusterInformationIteraror++)
  {
    ClusterInfoVector.push_back(*ClusterInformationIteraror);
  }

  return true;
}
*/

/*
DataSet*
DBSCAN::NewDataSet(void)
{
  return new DBSCANDataSet();
}
*/
const string DBSCAN::PARAMETER_K_BEGIN = "k_begin";
const string DBSCAN::PARAMETER_K_END   = "k_end";

bool DBSCAN::ParametersApproximation(const vector<const Point*>& Data,
                                     map<string, string>&        Parameters,
                                     string                      OutputFileNamePrefix)
{
  INT32  k_begin, k_end;
  map<string, string>::iterator ParametersIterator;

  if (Data.size() == 0)
  {
    SetErrorMessage("no data to compute the parameter approximation");
    SetError(true);
    return false;
  }

  /* k_begin */
  ParametersIterator = Parameters.find(DBSCAN::PARAMETER_K_BEGIN);
  if (ParametersIterator == Parameters.end())
  {
    string ErrorMessage;
    ErrorMessage = "value of '" + DBSCAN::PARAMETER_K_BEGIN + "' not found when approximating DBSCAN parameters";
    
    SetErrorMessage(ErrorMessage);
    SetError(true);
    return false;
  }
  else
  {
    char* err;
    k_begin = strtol(ParametersIterator->second.c_str(), &err, 0);

    if (*err)
    {
      string ErrorMessage;
      ErrorMessage = "incorrect value for '"+ DBSCAN::PARAMETER_K_BEGIN + "'";

      SetErrorMessage(ErrorMessage);
      SetError(true);
      return false;
    }
  }
  /* k_end */
  ParametersIterator = Parameters.find(DBSCAN::PARAMETER_K_END);
  if (ParametersIterator == Parameters.end())
  {
    k_end = k_begin;
  }
  else
  {
    char* err;
    k_end = strtol(ParametersIterator->second.c_str(), &err, 0);

    if (*err)
    {
      string ErrorMessage;
      ErrorMessage = "incorrect value for '"+ DBSCAN::PARAMETER_K_BEGIN + "'";

      SetErrorMessage(ErrorMessage);
      SetError(true);
      return false;
    }

    if (k_begin > k_end)
    {
      string ErrorMessage;
      ErrorMessage = "'k_begin' value bigger than 'k_end' when approximating DBSCAN parameters";

      SetErrorMessage(ErrorMessage);
      SetError(true);
      return false;
    }
  }

  
  return ComputeKNeighbourhoods(Data, k_begin, k_end, OutputFileNamePrefix);
}

bool DBSCAN::BuildKDTree(const vector<const Point*>& Data)
{
  assert(Data.size() > 0);

  size_t Dimensions = Data[0]->size();
  
#ifdef DEBUG
/*  cout << "Current clustering has " << Dimensions << " dimensions" << endl; */
#endif
  
  system_messages::show_progress("Building data spatial index", 0, Data.size());
  
  ANNDataPoints = annAllocPts(Data.size(), Dimensions);

  for (size_t i = 0; i < Data.size(); i++)
  {
    system_messages::show_progress("Building data spatial index", i, Data.size());
    ANNDataPoints[i] = ToANNPoint(Data[i]);
  }

  system_messages::show_progress_end("Building data spatial index", Data.size());

  SpatialIndex = new ANNkd_tree(ANNDataPoints,
                                Data.size(),
                                Dimensions);
  
  return true;
}

/*
bool
DBSCAN::ExpandCluster(DBSCANDataSet      *InputData,
                      DBSCANDataPoint    *Point,
                      INT32               CurrentClusterId,
                      ClusterInformation *CurrentClusterInformation)
*/
bool DBSCAN::ExpandCluster(const vector<const Point*>& Data,
                           point_idx                   CurrentPoint,
                           vector<cluster_id_t>&       ClusterAssignmentVector,
                           cluster_id_t                CurrentClusterId)
{
  list<point_idx> SeedList;
  list<point_idx> NeighbourSeedList;

  list<point_idx>::iterator SeedListIterator;
  list<point_idx>::iterator NeighbourSeedListIterator;

  /* DEBUG
  cout << "**** In EXPAND CLUSTER (BEGIN) ****" << endl;
  for (size_t i = 0; i < Data.size(); i++)
  {
    cout << "Clustering point " << i << " has " << Data[i]->size() << " dimensions" << endl;
  }
  */
  
  ClusterAssignmentVector[CurrentPoint] = CurrentClusterId;

  EpsilonRangeQuery(Data[CurrentPoint], SeedList);

  /* DEBUG
  cout << "**** In EXPAND CLUSTER (AFTER EPSILON RANGE QUERY) ****" << endl;
  for (size_t i = 0; i < Data.size(); i++)
  {
    cout << "Clustering point " << i << " has " << Data[i]->size() << " dimensions" << endl;
  }

  cout << "SeedList.size() = " << SeedList.size() << endl;
  */

  /* Point is NO core object */
  if (SeedList.size() < MinPoints)
  {
    ClusterAssignmentVector[CurrentPoint] = NOISE_CLUSTERID;

    return false;
  }

  SeedListIterator = SeedList.begin();
  while (SeedListIterator != SeedList.end())
  {
    point_idx CurrentNeighbour = (*SeedListIterator);
    ClusterAssignmentVector[CurrentNeighbour] = CurrentClusterId;

    if (CurrentNeighbour == CurrentPoint)
      SeedListIterator = SeedList.erase(SeedListIterator);
    else
      SeedListIterator++;
  }

  /* Iterate into the SeedList of Point */
  for (SeedListIterator  = SeedList.begin();
       SeedListIterator != SeedList.end();
       SeedListIterator++)
  {
    point_idx CurrentNeighbour = (*SeedListIterator);

    EpsilonRangeQuery(Data[CurrentNeighbour], NeighbourSeedList);
    /* DEBUG 
    cout << "NeighbourSeedList.size() = " << SeedList.size() << endl; */

    /* CurrentNeighbour is a core object */
    if (NeighbourSeedList.size() >= MinPoints)
    {
      for (NeighbourSeedListIterator = NeighbourSeedList.begin();
           NeighbourSeedListIterator != NeighbourSeedList.end();
           NeighbourSeedListIterator++)
      {
        point_idx CurrentNeighbourNeighbour = (*NeighbourSeedListIterator);

        if (ClusterAssignmentVector[CurrentNeighbourNeighbour] == UNCLASSIFIED  ||
            ClusterAssignmentVector[CurrentNeighbourNeighbour] == NOISE_CLUSTERID)
        {
          if (ClusterAssignmentVector[CurrentNeighbourNeighbour] == UNCLASSIFIED)
          {
            SeedList.push_back(CurrentNeighbourNeighbour);
          }

          ClusterAssignmentVector[CurrentNeighbourNeighbour] = CurrentClusterId;

        }
      }
    }
    NeighbourSeedList.clear();
  }

  /* DEBUG
  cout << "**** In EXPAND CLUSTER (END) ****" << endl;
  for (size_t i = 0; i < Data.size(); i++)
  {
    cout << "Clustering point " << i << " has " << Data[i]->size() << " dimensions" << endl;
  }
  */
  
  return true;
}

/* Computes the eps-neighbourhood from the given point */

/*
void
DBSCAN::EpsilonRangeQuery(DBSCANDataSet_t          InputData,
                          DBSCANDataPoint_t        Point,
                          list<DBSCANDataPoint_t>& SeedList) */
void DBSCAN::EpsilonRangeQuery(const Point* const QueryPoint,
                               list<size_t>&      SeedList)
{
  ANNpoint    ANNQueryPoint;
  ANNidxArray Results;
  size_t      ResultSize;

  ANNQueryPoint = ToANNPoint(QueryPoint);
  
  ResultSize = SpatialIndex->annkFRSearch(ANNQueryPoint, pow(Eps, 2.0), 0);
  
  Results = new ANNidx[ResultSize];
  
  ResultSize = SpatialIndex->annkFRSearch(ANNQueryPoint,
                                          pow(Eps, 2.0),
                                          ResultSize,
                                          Results);

  for (INT32 i = 0; i < ResultSize; i++)
  {
    SeedList.push_back(Results[i]);
  }
  
  delete [] Results;
}

ANNpoint DBSCAN::ToANNPoint(const Point* InputPoint)
{
  ANNpoint Result = annAllocPt(InputPoint->size());

  for (size_t i = 0; i < InputPoint->size(); i++)
  {
    Result[i] = (*InputPoint)[i];
  }

  return Result;
}

bool
DBSCAN::ComputeKNeighbourhoods(const vector<const Point*>& Data,
                               INT32                       k_begin,
                               INT32                       k_end,
                               string                      OutputFileNamePrefix)
{
  vector<vector< double> > ResultingDistances (k_end - k_begin + 1, vector<double> ());
  vector<string>           KNeighbourFileNames;
  vector<ofstream*>        KNeighbourDataStreams;
  string                   KNeighbourPlotFileName;
  ofstream                 KNeighbourPlotStream;
  size_t                   DataSize;

  /* Generate all neighbours data file names and streams */
  for (size_t i = 0; i <= (k_end - k_begin); i++)
  {
    ostringstream KNeighbourDataFileName;
    
    KNeighbourDataFileName << OutputFileNamePrefix;
    KNeighbourDataFileName << "." << k_begin+i << "Pts.neighbour_data";

    KNeighbourDataStreams.push_back(new ofstream(KNeighbourDataFileName.str().c_str(), std::ios_base::trunc));
    
    if (!KNeighbourDataStreams[i])
    {
      ostringstream ErrorMessage;

      ErrorMessage << "Unable to open ";
      ErrorMessage << k_begin+i;
      ErrorMessage << "-Neighbour distance data file \"";
      ErrorMessage << KNeighbourDataFileName;
      ErrorMessage << "\"";

      SetError(true);
      SetErrorMessage(ErrorMessage.str());
      return false;
    }

    KNeighbourFileNames.push_back(KNeighbourDataFileName.str());
    
    (*KNeighbourDataStreams[i]).precision(6);
    (*KNeighbourDataStreams[i]) << std::fixed;

    
  }

  /* Generate the plot script file and stream */
  KNeighbourPlotFileName = OutputFileNamePrefix+".neighbour.plot";
  KNeighbourPlotStream.open(KNeighbourPlotFileName.c_str(),  std::ios_base::trunc);
  
  if (!KNeighbourPlotStream)
  {
    string ErrorMessage;

    ErrorMessage = "Unable to open K-Neighbour distance plot file \"";
    ErrorMessage += KNeighbourPlotFileName;
    ErrorMessage += "\"";

    SetError(true);
    SetErrorMessage(ErrorMessage);
    return false;
  }
  
  /* Build KD tree */
  BuildKDTree(Data);

  /* Compute the distances for all points */

  system_messages::show_progress("Computing K-Neighbour distance",
                                 0,
                                 Data.size());


  for (size_t i = 0; i < Data.size(); i++)
  {
    
#ifdef EXTRA_DEBUG
    cout << "Searching for K-Neigbour of point (" << i << "): ";
    cout << *InputData[i] << endl;
#endif

    ComputeNeighboursDistance(Data[i], k_begin, k_end, ResultingDistances);

    system_messages::show_progress("Computing K-Neighbour distance", i, Data.size());
  }

  system_messages::show_progress_end("Computing K-Neighbour distance",
                                     Data.size());

  /* Sort distances */
  for (size_t i = 0; i <= (k_end - k_begin); i++)
  {
    sort(ResultingDistances[i].rbegin(), ResultingDistances[i].rend());
  }
  
  /* Flush files */

  for (size_t i = 0; i <= (k_end - k_begin); i++)
  {
    for (size_t j = 0; j < Data.size(); j++)
    {
      (*KNeighbourDataStreams[i]) << ResultingDistances[i][j] << '\n';
    }
  }

  /* Generate plot script */
  if (!system_messages::verbose)
    cout << "Generating neighbour GNUPLot script... ";

  KNeighbourPlotStream << "set title \"Neighbour distances\" font \",13\"" << '\n';

  /* There is always one 'k' value */
  KNeighbourPlotStream << "plot \"" << KNeighbourFileNames[0] << "\" title 'k = ";
  KNeighbourPlotStream << k_begin << "'";

  for (size_t i = 1; i <= (k_end - k_begin); i++)
  {
    KNeighbourPlotStream << ", " << '\n' << "\\"  ;
    KNeighbourPlotStream << "\"" << KNeighbourFileNames[i] << "\" title 'k = ";
    KNeighbourPlotStream << k_begin+i << "'";
  }
  
  KNeighbourPlotStream << '\n';
  KNeighbourPlotStream << "pause -1 \"Hit return to continue...\"" << '\n';
  
  /* Closing files */
  for (size_t i = 0; i <= (k_end - k_begin); i++)
  {
    (*KNeighbourDataStreams[i]).close();
  }
  
  KNeighbourPlotStream.close();
  
  if (!system_messages::verbose)
    cout << "DONE!" << endl;
  
  return true;
}

void DBSCAN::ComputeNeighboursDistance(const Point*             QueryPoint,
                                       size_t                   k_begin,
                                       size_t                   k_end,
                                       vector<vector<double> >& ResultingDistances)
{
  ANNpoint       ANNQueryPoint = ToANNPoint(QueryPoint);
  ANNidxArray    ResultPoints  = new ANNidx[k_end+1];
  ANNdistArray   Distances     = new ANNdist[k_end+1];
  vector<double> Result;

#ifdef EXTRA_DEBUG
  cout << __FUNCTION__  << " QueryPoint [";
  for (INT32 i = 0; i < QueryPoint->size(); i++)
  {
    cout << QueryPoint[i];
    if (i < QueryPoint->size()-1)
      cout << ",";
  }
  cout << "]" << endl;
#endif
  
  SpatialIndex->annkSearch(ANNQueryPoint, k_end+1, ResultPoints, Distances);
  
#ifdef EXTRA_DEBUG
  for (INT32 i = 0; i < k+1; i++)
  {
    cout << "[" << i << "]: " << Distances[i] << " ";
  }
  cout << endl;
#endif

  for (size_t i = 0; i <= (k_end - k_begin); i++)
  {
    ResultingDistances[i].push_back(ANN_ROOT(Distances[k_begin+i]));
  }
  
  delete    QueryPoint;
  delete [] ResultPoints;
  delete [] Distances;
}
