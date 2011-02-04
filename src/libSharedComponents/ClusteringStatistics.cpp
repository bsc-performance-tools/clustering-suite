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

  $URL::                                                                   $:

  $Rev::                            $:  Revision of last commit
  $Author::                         $:  Author of last commit
  $Date::                           $:  Date of last commit

\* -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=- */

#include "ClusteringStatistics.hpp"

#include <sstream>
using std::ostringstream;

/******************************************************************************
 * CLASS 'ExtrapolationDataContainer'
 *****************************************************************************/

/**
 * Parametrized constructor. Sets sizes of all vectors
 *
 * \param ExtrapolationMetrics Number of metrics to extrapolate
 *
 */
ExtrapolationDataContainer::ExtrapolationDataContainer(size_t ExtrapolationMetrics)
{
  Individuals = vector<size_t>(ExtrapolationMetrics, 0);
  Mean        = vector<double>(ExtrapolationMetrics, 0.0);
  M2          = vector<double>(ExtrapolationMetrics, 0.0);
  StdDev_2    = vector<double>(ExtrapolationMetrics, 0.0);
}

/**
 * Updates the statistics of the extrapolation metrics
 *
 * \param ExtrapolationData Hash containing the extrapolation data froma CPU Burst
 *
 */
void ExtrapolationDataContainer::Update(const map<size_t, double>& ExtrapolationData)
{
  map<size_t, double>::const_iterator Iterator;

  for (Iterator  = ExtrapolationData.begin();
       Iterator != ExtrapolationData.end();
       ++Iterator)
  {
    double Delta;
    size_t i = Iterator->first;  // position of the current event
    double x = Iterator->second; // value of the current event

    Individuals[i] = Individuals[i] + 1;
    Delta          = x - Mean[i];
    Mean[i]        = Mean[i] + (Delta/Individuals[i]);
    M2[i]          = M2[i] + Delta * (x - Mean[i]);

    StdDev_2[i]    = M2[i]/(Individuals[i]-1);
  }
}

/******************************************************************************
 * CLASS 'ClusteringStatistics'
 *****************************************************************************/

/**
 * Parametrized constructor. Sets sizes of all vectors
 *
 * \param NumberOfCluster Clusters obtained in the cluster analysis
 * \param ExtrapolationMetrics Number of metrics to extrapolate
 *
 */
ClusteringStatistics::ClusteringStatistics(size_t NumberOfClusters,
                                           size_t ExtrapolationMetrics)
{
  InitStatistics(NumberOfClusters, ExtrapolationMetrics);
}

/**
 * Initialization of all containers. Used by the parametrized constructor of by itself
 *
 * \param NumberOfCluster Clusters obtained in the cluster analysis
 * \param ExtrapolationMetrics Number of metrics to extrapolate
 *
 */
void ClusteringStatistics::InitStatistics(size_t NumberOfClusters,
                                          size_t ExtrapolationMetrics)
{
  Individuals      = vector<size_t> (NumberOfClusters, 0);
  TotalDuration    = vector<double> (NumberOfClusters, 0.0);
  DurationMean     = vector<double> (NumberOfClusters, 0.0);
  DurationM2       = vector<double> (NumberOfClusters, 0.0);
  DurationStdDev_2 = vector<double> (NumberOfClusters, 0.0);;
  Extrapolation    = vector<ExtrapolationDataContainer> (NumberOfClusters, ExtrapolationDataContainer(ExtrapolationMetrics));
}

/**
 * Updates the statistics of the container
 *
 * \param Bursts The set of burst used in the cluster analysis
 * \param IDs The set cluster ids of the bursts analyzed
 *
 * \return True if the statistics has been computed correctly, false otherwise
 */
bool ClusteringStatistics::ComputeStatistics(const vector<CPUBurst*>&    Bursts,
                                             const vector<cluster_id_t>& IDs)
{
  if (Bursts.size() != IDs.size())
  {
    SetError(true);
    SetErrorMessage("number of bursts different to number of IDs when computing statistics");
    return false;
  }

  for (size_t i = 0; i < Bursts.size(); i++)
  {
    if (!NewIndividual(Bursts[i], IDs[i]))
    {
      return false;
    }
  }
  
  return true;
}

/**
 * Returns the translation of the cluster IDs, in terms of cluster aggregate duration
 *
 * \param NewIDs In/Out vector that contains the original IDs as input and the translation as output
 *
 */
void ClusteringStatistics::TranslatedIDs(vector<cluster_id_t>& NewIDs)
{
  map<double, cluster_id_t>                       SortingMap;
  map<double, cluster_id_t>::const_iterator       SortingMapIterator;
  map<cluster_id_t, cluster_id_t>                 TranslationMap;
  map<cluster_id_t, cluster_id_t>::const_iterator TranslationMapIterator;
  cluster_id_t TranslatedClusterID;

  for (size_t i = 1; i < TotalDuration.size(); i++)
  {
    SortingMap.insert(std::make_pair(TotalDuration[i], i));
  }

  /*
  cout << "NewIDs vectors size = " << NewIDs.size() << endl;
  */

  TranslationMap[NOISE_CLUSTERID] = NOISE_CLUSTERID;
  SortingMapIterator = SortingMap.end();

  
  TranslatedClusterID = MIN_CLUSTERID;
  do
  {
    --SortingMapIterator;
    
    cluster_id_t OriginalClusterID = (*SortingMapIterator).second;
    double       Duration          = (*SortingMapIterator).first;
    /* DEBUG 
    cout << "CLUSTER << " << OriginalClusterID << " ";
    cout << "Total Duration = " << Duration << endl; 
    */
    
    TranslationMap[OriginalClusterID] = TranslatedClusterID;
    TranslatedClusterID++;
  } while (SortingMapIterator != SortingMap.begin());

  /* DEBUG 
  for (TranslationMapIterator  = TranslationMap.begin();
       TranslationMapIterator != TranslationMap.end();
       TranslationMapIterator++)
  {
    cout << "OLD CLUSTER " << TranslationMapIterator->first;
    cout << " --> " << TranslationMapIterator->second << endl;
  }
  */

  for (size_t i = 0; i < NewIDs.size(); i++)
  {
    // cout << "Point[" << i << "] OLD ID = " << NewIDs[i] << " ";
    NewIDs[i] = TranslationMap[NewIDs[i]];
    // cout << "NEW ID = " << NewIDs[i] << endl;
  }

  return;
}

/**
 * Updates the statistics of a given cluster, with the data contained in the CPU burst
 *
 * \param Burst The CPU burst to take into consideration
 * \param ID The cluster id of the new burst
 *
 * \return True if the update was done correctly, false otherwise
 */
bool ClusteringStatistics::NewIndividual(CPUBurst* Burst, cluster_id_t ID)
{
  double Delta;
  size_t i;
  double x;
  
  if (ID < 0 || ID >= Individuals.size() || Burst == NULL)
  { /* ID out of range || Burst invalid */
    ostringstream ErrorMessage;
    ErrorMessage << "incorrect cluster ID (" << ID << ") when computing statistics";
    
    SetError(true);
    SetErrorMessage(ErrorMessage.str());
    return false;
  }

  /* Update Duration Sum/Mean/StdDeviation */
  i = ID;
  x = Burst->GetDuration();
  
  Individuals[i]      = Individuals[i] + 1;
  TotalDuration[i]    = TotalDuration[i] + x;
  Delta               = x - DurationMean[i];
  DurationMean[i]     = DurationMean[i] + (Delta/Individuals[i]);
  DurationM2[i]       = DurationM2[i] + Delta * (x - DurationMean[i]);

  DurationStdDev_2[i] = DurationM2[i]/(Individuals[i]-1);

  /* Update cluster extrapolation metrics */
  Extrapolation[i].Update(Burst->GetExtrapolationDimensions());

  return true;
}
