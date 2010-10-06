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

#include <SystemMessages.hpp>

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

using std::make_pair;

const string DBSCAN::EPSILON_STRING = "epsilon";
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
  
  ostringstream                       ClusterName;
  /* vector<ClusterInformation*>         ClusterInfoVector; */

  if (Data.size() == 0)
  {
    return true;
  }

  /* DEBUG */
  cout << "Running DBSCAN" << endl;
  vector<cluster_id_t>& ClusterAssignmentVector = DataPartition.GetAssignmentVector();

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
 
  /* Obtain Noise and Threshold Filtered clusters information objects
  NoiseClusterInfo             = InputDataManager->GetNoiseClusterInfo();
  */

  /*
  ThresholdFilteredClusterInfo = 
    InputDataManager->GetThresholdFilteredClusterInfo();
  */

  /* Add the first (candidate) cluster information to ClustersInformation 
  ClusterName.str("");
  ClusterName << "Cluster " << ClusterId;
  
  CurrentClusterInformation =
    InputDataManager->NewClusterInformation(ClusterId, ClusterName.str());
  */

  system_messages::show_progress(stdout, "Clustering points", 0, (int) Data.size());
  Index = 0; // Double counter: total points vs. clustering points!

  for (point_idx i = 0; i < Data.size(); i++)
  {
    system_messages::show_progress(stdout, "Clustering points", i, (int) Data.size());

    if (ClusterAssignmentVector[i] == UNCLASSIFIED)
    {
      if (ExpandCluster(Data, i, ClusterAssignmentVector, ClusterId))
      {
        ClusterId++;
        ResultingClusters++;
      }
    }
  }


  /*
  for (size_t i = 0; i < InputData->size(); i++)
  {
    
    if ((*InputData)[i]->WillBeClusterized())
    {
      show_progress(stdout, "Clustering points", Index, DataSize);
      CurrentPoint = (*Data)[i];

      if (!CurrentPoint->Classified())
      {
        if (ExpandCluster(InputData,
                          CurrentPoint,
                          ClusterId,
                          CurrentClusterInformation))
        {
          ClusterId++;
          ResultingClusters++; /* Defined on 'ClusteringAlgorithm.h' */

          /* Add new cluster information to ClusterInformationVector 
          ClustersInformation.push_back(CurrentClusterInformation);
          
          ClusterName.str("");
          ClusterName << "Cluster " << ClusterId;

          CurrentClusterInformation =
            InputDataManager->NewClusterInformation(ClusterId, ClusterName.str());
        }
      }
      
      Index++;
    }
  }
  */

  system_messages::show_progress_end(stdout, "Clustering points", (int) Data.size());

  /* Erase last cluster information if not needed 
  if (CurrentClusterInformation->GetDensity() == 0)
  {
    InputDataManager->DeleteClusterInformation(CurrentClusterInformation);
  }
  */
  
  /* Reorganize clusters 
  if (!SimpleRun)
  {
    SortResultingClusters(InputDataManager,
                          InputData);
  }
  */
          
  /* Set clustering ready
  ClusteringReady = true;
  Data->SetClusteringReady(true); */
  
  /* Set clusters information on DataManager 
  if (!GetClustersInformation(ClusterInfoVector))
  {
    SetError(true);
    SetErrorMessage("unable to get clusters information");
    return false;
  }
  */

  /* InputDataManager->SetClustersInformation(ClusterInfoVector); */
  DataPartition.SetNumberOfClusters (ResultingClusters);

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

bool DBSCAN::ComputeParamsApproximation(const vector<const Point*>& Data,
                                        INT32                       ParametersCount, ...)
{
  vector<INT32>* KNeighbours;
  char*          KNeighbourFileBaseName;
  va_list        Arguments;

  /* DataSet* Data = DataSet::GetInstance(); */

  va_start(Arguments, ParametersCount);

  if (ParametersCount != 2)
  {
    ostringstream ErrorMessage;

    ErrorMessage << "parameter approximation in DBSCAN only requires ";
    ErrorMessage << "2 and " << ParametersCount << " received";
    SetErrorMessage(ErrorMessage.str().c_str());
    return false;
  }

  KNeighbours            = va_arg(Arguments, vector<INT32>*);
  KNeighbourFileBaseName = va_arg(Arguments, char*);
  
  return ComputeKNeighbourhoods(Data,
                                KNeighbours,
                                string(KNeighbourFileBaseName));
}

bool
DBSCAN::ComputeParamsApproximation(const vector<const Point*>& Data,
                                   INT32                       MinPoints,
                                   vector<double>&             Distances)
{
  ComputeKNeighbourDistances(Data, MinPoints, Distances);

  return true;
}

bool DBSCAN::BuildKDTree(const vector<const Point*>& Data)
{
  assert(Data.size() > 0);

  size_t Dimensions = Data[0]->size();
  
#ifdef DEBUG
/*  cout << "Current clustering has " << Dimensions << " dimensions" << endl; */
#endif
  
  system_messages::show_progress(stdout, "Building data spatial index", 0, Data.size());
  
  ANNDataPoints = annAllocPts(Data.size(), Dimensions);

  for (size_t i = 0; i < Data.size(); i++)
  {
    system_messages::show_progress(stdout, "Building data spatial index", i, Data.size());
    ANNDataPoints[i] = ToANNPoint(Data[i]);
  }

  system_messages::show_progress_end(stdout, "Building data spatial index", Data.size());

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

  cout << "**** In EXPAND CLUSTER (BEGIN) ****" << endl;
  for (size_t i = 0; i < Data.size(); i++)
  {
    cout << "Clustering point " << i << " has " << Data[i]->size() << " dimensions" << endl;
  }
  
  ClusterAssignmentVector[CurrentPoint] = CurrentClusterId;

  EpsilonRangeQuery(Data[CurrentPoint], SeedList);

  cout << "**** In EXPAND CLUSTER (AFTER EPSILON RANGE QUERY) ****" << endl;
  for (size_t i = 0; i < Data.size(); i++)
  {
    cout << "Clustering point " << i << " has " << Data[i]->size() << " dimensions" << endl;
  }
  /* DEBUG */
  cout << "SeedList.size() = " << SeedList.size() << endl;

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

  cout << "**** In EXPAND CLUSTER (END) ****" << endl;
  for (size_t i = 0; i < Data.size(); i++)
  {
    cout << "Clustering point " << i << " has " << Data[i]->size() << " dimensions" << endl;
  }
  
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
                               vector<INT32>              *KNeighbours,
                               string                      KNeighbourFileBaseName)
{
  vector<vector< double> > ResultingDistances;
  vector<string>           KNeighbourFileNames;
  vector<FILE*>            KNeighbourDataFiles (KNeighbours->size());
  string                   KNeighbourPlotFileName;
  FILE*                    KNeighbourPlotFile;
  size_t                   DataSize;
  INT64                    CurrentIndex;


  if (Data.size() == 0)
  { /* NO DATA! */
    return true;
  }
  
  /* Generate all neighbours data file names */
  for (INT32 i = 0; i < KNeighbours->size(); i++)
  {
    ostringstream KNeighbourDataFileName;
    
    KNeighbourDataFileName << KNeighbourFileBaseName;
    KNeighbourDataFileName << "." << KNeighbours->at(i) << "Pts.neighbour_data";
    
    if ((KNeighbourDataFiles[i] = 
           fopen(KNeighbourDataFileName.str().c_str(), "w")) == NULL)
    {
      ostringstream ErrorMessage;

      ErrorMessage << "Unable to open ";
      ErrorMessage << (*KNeighbours)[i];
      ErrorMessage << "-Neighbour distance data file \"";
      ErrorMessage << KNeighbourDataFileName;
      ErrorMessage << "\"";

      SetError(true);
      SetErrorMessage(ErrorMessage.str());
      return false;
    }

    KNeighbourFileNames.push_back(KNeighbourDataFileName.str());
  }

  KNeighbourPlotFileName = KNeighbourFileBaseName+".neighbour.plot";
  
  if ((KNeighbourPlotFile = fopen(KNeighbourPlotFileName.c_str(), "w")) == NULL)
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

/* DEBUG */
  for (INT32 i = 0; i < KNeighbours->size(); i++)
  {
    ResultingDistances.push_back(vector<double>());

    cout << "Computing K-neighbour distances k=" << (*KNeighbours)[i] << endl;
    
    ComputeKNeighbourDistances(Data,
                               (*KNeighbours)[i],
                               ResultingDistances[i]);
  }

/*

#ifndef DEBUG
  show_progress(stdout,
                "Computing K-Neighbour distance",
                0,
                DataSize);
#endif

  CurrentIndex = 0;
  for (INT32 i = 0; i < InputData->size(); i++)
  {
#ifdef EXTRA_DEBUG
    cout << "Searching for K-Neigbour of point (" << i << "): ";
    cout << *InputData[i] << endl;
#endif
    
    if ((*InputData)[i]->WillBeClusterized())
    {
      for (INT32 j = 0; j < KNeighbours->size(); j++)
      {
        ResultingDistances.push_back(vector<double> (DataSize));

        ResultingDistances[j][CurrentIndex] = 
          ComputeKNeighbourDistance((*InputData)[i], (*KNeighbours)[j]);
      }
      CurrentIndex++;
    }


#ifndef DEBUG
    show_progress(stdout,
                  "Computing K-Neighbour distance",
                  CurrentIndex,
                  DataSize);
#endif
  }

*/
  
#ifndef DEBUG
  system_messages::show_progress_end(stdout,
                                     "Computing K-Neighbour distance",
                                     Data.size());
#endif

  /* Sort distances */
  if (!system_messages::verbose)
    cout << "Sorting distances... ";

  for (INT32 i = 0; i < KNeighbours->size(); i++)
  {
    sort(ResultingDistances[i].begin(), ResultingDistances[i].end());
  }

  if (!system_messages::verbose)
    cout << "DONE!" << endl;
  
  /* Flush files */
  if (!system_messages::verbose)
    cout << "Flushing sorted data output files... ";

  for (INT32 j = 0; j < KNeighbours->size(); j++)
  {
    for (INT32 i = ResultingDistances[j].size()-1; i >= 0; i--)
    {

      if (
      fprintf(KNeighbourDataFiles[j], "%.06f\n", ResultingDistances[j][i]) < 0)
      {
        if (!system_messages::verbose)
          cout << "NOK!" << endl;
        SetError(true);
        SetErrorMessage("problem writing value to K-Neighbour distance data file",
                        strerror(errno));
        return false;
      }
    }
  }
  
  if (!system_messages::verbose)
    cout << "DONE! " << endl;

  /* Generate plot script */
  if (!system_messages::verbose)
    cout << "Generating neighbour GNUPLot script... ";
  
  fprintf(KNeighbourPlotFile, 
          "set title \"Neighbour distances\" font \",13\"\n");
  /* There is always one 'k' value */
  fprintf(KNeighbourPlotFile,
            "plot \"%s\" title 'k = %d'",
            KNeighbourFileNames[0].c_str(),
            (*KNeighbours)[0]);
  
  if (KNeighbours->size() > 1)
  {
    for (INT32 i = 1; i < KNeighbours->size(); i++)
    {
      fprintf(KNeighbourPlotFile,
              ",\\\n\"%s\" title 'k = %d'",
              KNeighbourFileNames[i].c_str(),
              (*KNeighbours)[i]);
    }
  }
  fprintf(KNeighbourPlotFile,
          "\npause -1 \"Hit return to continue...\"\n");
  
  /* Closing files */
  for (INT32 i = 0; i < KNeighbours->size(); i++)
    fclose(KNeighbourDataFiles[i]);
  
  fclose(KNeighbourPlotFile);
  
  if (!system_messages::verbose)
    cout << "DONE!" << endl;
  
  return true;
}

bool DBSCAN::ComputeKNeighbourDistances(const vector<const Point*>& Data,
                                   INT32                            k,
                                   vector<double>&                  Distances)
{
  
#ifndef DEBUG
  system_messages::show_progress(stdout,
                                 "Computing K-Neighbour distance",
                                 0,
                                 Data.size());
#endif

  for (size_t i = 0; i < Data.size(); i++)
  {

#ifdef EXTRA_DEBUG
    cout << "Searching for K-Neigbour of point (" << i << "): ";
    // cout << (*InputData)[i] << endl;
#endif
    

    Distances.push_back(ComputeKNeighbourDistance(Data[i], k));




#ifndef DEBUG
    system_messages::show_progress(stdout,
                                   "Computing K-Neighbour distance",
                                   i,
                                   Data.size());
#endif
  }

  return true;
}

double
DBSCAN::ComputeKNeighbourDistance(const Point* QueryPoint, INT32 k)
{
  ANNpoint     ANNQueryPoint = ToANNPoint(QueryPoint);
  ANNidxArray  ResultPoints  = new ANNidx[k+1];
  ANNdistArray Distances     = new ANNdist[k+1];
  double       Result;

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
  
  SpatialIndex->annkSearch(ANNQueryPoint, k+1, ResultPoints, Distances);
  
#ifdef EXTRA_DEBUG
  for (INT32 i = 0; i < k+1; i++)
  {
    cout << "[" << i << "]: " << Distances[i] << " ";
  }
  cout << endl;
#endif
  
  Result = ANN_ROOT(Distances[k]);
  
  delete    QueryPoint;
  delete [] ResultPoints;
  delete [] Distances;
  
  return Result;
}


/*
void
DBSCAN::SortResultingClusters(DataManager*   InputDataManager,
                              DBSCANDataSet* InputData)
{
  ostringstream                       ClusterName;
  INT32                               i;
  UINT64                              TotalBurstDuration = 0;
  list<ClusterInformation*>::iterator ClusterInformationIteraror;
  
  /* Reorganize clusters based on cluster duration 
  ClustersInformation.sort(ClusterInformationCompare());
  
  ClusterIdTranslation = vector<INT32> (ClustersInformation.size()+1);

  /* Filter clusters with duration less than x% of total durations and 
   * rename the clusters in terms of total duration 
  for (ClusterInformationIteraror  = ClustersInformation.begin();
       ClusterInformationIteraror != ClustersInformation.end();
       ClusterInformationIteraror++)
  {
    TotalBurstDuration += (*ClusterInformationIteraror)->GetDurationSum();
  }
  
  for (ClusterInformationIteraror  = ClustersInformation.begin(), 
       i                           = MIN_CLUSTERID ;
       ClusterInformationIteraror != ClustersInformation.end();
       ClusterInformationIteraror++,
       i++)
  {
    /* Original position is changed with the new position 
    INT32 OriginalClusterId =
      (*ClusterInformationIteraror)->GetClusterId();
    
    ClusterIdTranslation[OriginalClusterId-CLUSTERID_OFFSET] = i;
    
    (*ClusterInformationIteraror)->ComputeDurationPercentage(TotalBurstDuration);
    
    if ((*ClusterInformationIteraror)->GetDurationPercentage() < FilterThreshold)
    {
      ClusterIdTranslation[OriginalClusterId-CLUSTERID_OFFSET] = 
        THRESHOLD_FILTERED_CLUSTERID;

      (*ClusterInformationIteraror)->SetThresholdFiltered(true);
      
      ClusterName.str("");
      ClusterName << (*ClusterInformationIteraror)->GetClusterName() << "F";
    }
    else
    {
      ClusterName.str("");
      ClusterName << "Cluster " << i-CLUSTERID_OFFSET;
    }
    
    (*ClusterInformationIteraror)->SetClusterName(ClusterName.str());
    (*ClusterInformationIteraror)->SetClusterId(i);
  }
  

  /* Translate the clusterId of each point 

  show_progress(stdout, "Translating clusters", 0, InputData->size());
  for (INT32 i = 0; i < InputData->size(); i++)
  {
    show_progress(stdout, "Translating clusters", i, InputData->size());
    
    if ((*InputData)[i]->WillBeClusterized() && !(*InputData)[i]->IsNoise())
    {
      INT32 NewClusterId = 
        ClusterIdTranslation[(*InputData)[i]->GetClusterId()-CLUSTERID_OFFSET];
      
      (*InputData)[i]->SetClusterId(NewClusterId);
    }
  }
  show_progress_end(stdout, "Translating clusters", InputData->size());
  
  /* Merge all threshold filtered clusters
  if (ClustersInformation.size() != 0)
  {
    while(true)
    {
      ClusterInformation* CurrentClusterInformation;
      
      CurrentClusterInformation = ClustersInformation.back();
      
      if (CurrentClusterInformation->IsThresholdFiltered())
      {
        ThresholdFilteredClusterInfo->Merge(CurrentClusterInformation);
        InputDataManager->DeleteClusterInformation(CurrentClusterInformation);
        
        ClustersInformation.pop_back();
      }
      else
      {
        break;
      }
    }
  }
  
  return;
}
*/
