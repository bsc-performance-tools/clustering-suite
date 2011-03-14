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
using std::scientific;

/******************************************************************************
 * CLASS 'MetricContainer'
 *****************************************************************************/

/**
 * Parametrized constructor. Sets sizes of all vectors
 *
 * \param Name Name of the metric
 * \param HighPrecision True if the metric is obtained deriving two metrics
 * \param NumberOfClusters Number of metrics to extrapolate
 *
 */
MetricContainer::MetricContainer(string Name, 
                                 bool   HighPrecision,
                                 size_t NumberOfClusters)
{
  this->Name          = Name;
  this->HighPrecision = HighPrecision;
  Individuals         = vector<size_t>(NumberOfClusters, 0);
  Mean                = vector<double>(NumberOfClusters, 0.0);
  M2                  = vector<double>(NumberOfClusters, 0.0);
  StdDev_2            = vector<double>(NumberOfClusters, 0.0);
}

/**
 * Updates the statistics of the extrapolation metrics
 *
 * \param ExtrapolationData Hash containing the extrapolation data froma CPU Burst
 *
 */
void MetricContainer::Update(cluster_id_t Cluster, double NewValue)
{
  double Delta;
  Individuals[Cluster] = Individuals[Cluster] + 1;
  Delta                = NewValue - Mean[Cluster];
  Mean[Cluster]        = Mean[Cluster] + (Delta/Individuals[Cluster]);
  M2[Cluster]          = M2[Cluster] + Delta * (NewValue - Mean[Cluster]);
  StdDev_2[Cluster]    = M2[Cluster]/(Individuals[Cluster]-1);
}

/**
 * Print the metric mean of all clusters present in the container
 *
 * \param str Stream where the data will be flush
 * \param HasNoise True if the cluster_id noise should be printed
 * \param TranslationMap Renaming of clusters
 * 
 * \return True if the metric means where correctly writen, false otherwise
 *
 */
bool MetricContainer::Flush(ostream&              str,
                            bool                  HasNoise,
                            vector<cluster_id_t>& SortedClusters)
{
  size_t NumberOfClusters = Individuals.size();
  
  str << Name;
  
  if (HighPrecision)
  {
    str.precision(5);
  }
  else
  {
    str.precision(0);
  }
  str << fixed;
  
  if (HasNoise)
  {
    str << ", " << Mean[SortedClusters[NOISE_CLUSTERID]];
  }

  for (size_t Cluster = 1; Cluster < NumberOfClusters; Cluster++)
  {
    str << ", ";
    if (Individuals[SortedClusters[Cluster]] != 0)
    {
       str << Mean[SortedClusters[Cluster]]; // DEBUG << " (" << Individuals[SortedClusters[Cluster]] << ")";
    }
    else
    {
      str << "nan";
    }
  }

  str << '\n';
  
  return true;
}

/******************************************************************************
 * CLASS 'ClusteringStatistics'
 *****************************************************************************/

/**
 * Parametrized constructor. Sets sizes of all vectors
 *
 * \param NumberOfCluster Clusters obtained in the cluster analysis
 * \param ClusteringMetrics Number of parameters used in the cluster analysis
 * \param ExtrapolationMetrics Number of metrics to extrapolate
 *
 */
ClusteringStatistics::ClusteringStatistics(size_t         NumberOfClusters,
                                           bool           HasNoise,
                                           vector<string> ClusteringParametersNames,
                                           vector<bool>   ClusteringParametersPrecision,
                                           vector<string> ExtrapolationMetricsNames,
                                           vector<bool>   ExtrapolationMetricsPrecision)
{
  InitStatistics(NumberOfClusters,
                 HasNoise,
                 ClusteringParametersNames,
                 ClusteringParametersPrecision,
                 ExtrapolationMetricsNames,
                 ExtrapolationMetricsPrecision);
}

/**
 * Initialization of all containers. Used by the parametrized constructor of by itself
 *
 * \param NumberOfCluster Clusters obtained in the cluster analysis
 * \param ClusteringMetrics Number of parameters used in the cluster analysis
 * \param ExtrapolationMetrics Number of metrics to extrapolate
 *
 */
void ClusteringStatistics::InitStatistics(size_t         NumberOfClusters,
                                          bool           HasNoise,
                                          vector<string> ClusteringParametersNames,
                                          vector<bool>   ClusteringParametersPrecision,
                                          vector<string> ExtrapolationMetricsNames,
                                          vector<bool>   ExtrapolationMetricsPrecision)
{
  this->NumberOfClusters     = NumberOfClusters;
  this->HasNoise             = HasNoise;

  Individuals      = vector<size_t> (NumberOfClusters, 0);
  TotalDuration    = vector<double> (NumberOfClusters, 0.0);
  DurationMean     = vector<double> (NumberOfClusters, 0.0);
  DurationM2       = vector<double> (NumberOfClusters, 0.0);
  DurationStdDev_2 = vector<double> (NumberOfClusters, 0.0);;

  // Initialization of Clustering Metrics accounting
  for (size_t i = 0; i < ClusteringParametersNames.size(); i++)
  {
    ClusteringMetrics.push_back(MetricContainer(ClusteringParametersNames[i],
                                                ClusteringParametersPrecision[i],
                                                NumberOfClusters));
  }

  // Initialization of Extrapolation Metrics accounting
  for (size_t i = 0; i < ExtrapolationMetricsNames.size(); i++)
  {
    ExtrapolationMetrics.push_back(MetricContainer(ExtrapolationMetricsNames[i],
                                                   ExtrapolationMetricsPrecision[i],
                                                   NumberOfClusters));
  }
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
  map<double, cluster_id_t>                         SortingMap;
  map<double, cluster_id_t>::const_reverse_iterator SortingMapIterator;
  map<cluster_id_t, cluster_id_t>::iterator TranslationMapIterator;
  cluster_id_t TranslatedClusterID;

  for (size_t i = 1; i < TotalDuration.size(); i++)
  {
    SortingMap.insert(std::make_pair(TotalDuration[i], i));
  }

  /*
  cout << "NewIDs vectors size = " << NewIDs.size() << endl;
  */

  TranslationMap[NOISE_CLUSTERID] = NOISE_CLUSTERID;

  
  TranslatedClusterID = MIN_CLUSTERID;
  for (SortingMapIterator  = SortingMap.rbegin();
       SortingMapIterator != SortingMap.rend();
       ++SortingMapIterator)
  {
    // --SortingMapIterator;
    
    cluster_id_t OriginalClusterID = (*SortingMapIterator).second;
    double       Duration          = (*SortingMapIterator).first;
    /* DEBUG
    cout << "CLUSTER << " << OriginalClusterID << " ";
    cout << "Total Duration = " << Duration << endl; */
    
    TranslationMap[OriginalClusterID] = TranslatedClusterID;
    TranslatedClusterID++;
  }
  //while (SortingMapIterator != SortingMap.begin());

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
 * Flush the statistics in the indicated stream
 *
 * \param str The stream where the information will be flushed
 *
 * \return True if the flush was done correctly, false otherwise
 */
bool ClusteringStatistics::Flush(ostream& str)
{
  double AllClustersDuration = 0.0;

  map<cluster_id_t, cluster_id_t>::iterator TranslationMapIt;
  vector<cluster_id_t> SortedClusters (NumberOfClusters, NOISE_CLUSTERID);
  
  if (TranslationMap.size() == 0)
  {
    for (size_t i = 0; i < NumberOfClusters; i++)
    {
      SortedClusters[i] = (cluster_id_t) i;
    }
  }
  else
  {
    for (TranslationMapIt  = TranslationMap.begin();
         TranslationMapIt != TranslationMap.end();
         ++TranslationMapIt)
    {
      SortedClusters[TranslationMapIt->second] = TranslationMapIt->first;
      /* DEBUG
      cout << "Original ID = " << TranslationMapIt->first;
      cout << " --> " << TranslationMapIt->second << endl; */
    }
  }

  /* Header */
  str << "Cluster Name";

  if (HasNoise)
  {
    str << ", NOISE";
  }

  for (size_t Cluster = 1; Cluster < NumberOfClusters; Cluster++)
  {
    str << ", Cluster " << Cluster;
  }

  str << '\n'; // System aware endline

  /* Density */
  str << "Density";

  str.precision(0);
  str << fixed;
  
  if (HasNoise)
  {
    str << ", " << Individuals[SortedClusters[NOISE_CLUSTERID]];
  }

  for (size_t Cluster = 1; Cluster < NumberOfClusters; Cluster++)
  {
    str << ", " << Individuals[SortedClusters[Cluster]];
  }

  str << '\n';

  /* Total Duration */
  str << "Total Duration";

  // str.precision(3);
  // str << scientific;
  
  if (HasNoise)
  {
    str << ", ";
    
    str << TotalDuration[SortedClusters[NOISE_CLUSTERID]];
    AllClustersDuration += TotalDuration[SortedClusters[NOISE_CLUSTERID]];
  }

  for (size_t Cluster = 1; Cluster < NumberOfClusters; Cluster++)
  {
    str << ", ";
    
    str << TotalDuration[SortedClusters[Cluster]];
    AllClustersDuration += TotalDuration[SortedClusters[Cluster]];
  }

  str << '\n';

  /* Avg. Duration */
  str << "Avg. Duration";

  str.precision(0);
  str << fixed;
  if (HasNoise)
  {
    str << ", " << DurationMean[SortedClusters[NOISE_CLUSTERID]];
  }

  for (size_t Cluster = 1; Cluster < NumberOfClusters; Cluster++)
  {
    str << ", " << DurationMean[SortedClusters[Cluster]];
  }

  str << '\n';

  /* % Total Duration */
  str << "% Total Duration";

  str.precision(5);
  str << fixed;
  if (HasNoise)
  {
    str << ", " << TotalDuration[SortedClusters[NOISE_CLUSTERID]]/AllClustersDuration;
  }

  for (size_t Cluster = 1; Cluster < NumberOfClusters; Cluster++)
  {
    str << ", " << TotalDuration[SortedClusters[Cluster]]/AllClustersDuration;
  }

  str << '\n';

  /* Clustering Metrics: ORIGINAL 'clusters_info' file didn't have clustering metrics 
  for (size_t CurrentMetric = 0; CurrentMetric < ClusteringMetrics; CurrentMetric++)
  {
    str << ClusteringParameterNames[CurrentMetric];

    if (HasNoise)
    {
      vector<double>& NoiseClusteringMetrics = ClusteringParametersMean[TranslationMap[NOISE_CLUSTERID]];
      str << ", " << NoiseClusteringMetrics[CurrentMetric];
    }

    for (size_t Cluster = 1; Cluster < NumberOfClusters; Cluster++)
    {
      vector<double>& NoiseClusteringMetrics = ClusteringParametersMean[TranslationMap[Cluster]];
      str << ", " << NoiseClusteringMetrics[CurrentMetric];
    }

    str << '\n';
  }
  */

  /* Extrapolation Metrics */
  for (size_t CurrentMetric = 0; CurrentMetric < ExtrapolationMetrics.size(); CurrentMetric++)
  {
    if (!ExtrapolationMetrics[CurrentMetric].Flush(str, HasNoise, SortedClusters))
    {
      SetErrorMessage("writing extrapolation metric");
      return false;
    }
  }

  return true;
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
  
  if (ID < 0 || ID >= NumberOfClusters || Burst == NULL)
  { /* ID out of range || Burst invalid */
    ostringstream ErrorMessage;
    ErrorMessage << "incorrect cluster ID (" << ID << ") when computing statistics";
    
    SetError(true);
    SetErrorMessage(ErrorMessage.str());
    return false;
  }

  /* Update Duration Sum/Mean/StdDeviation */
  x = Burst->GetDuration();
  
  Individuals[ID]      = Individuals[ID] + 1;
  TotalDuration[ID]    = TotalDuration[ID] + x;
  Delta                = x - DurationMean[ID];
  DurationMean[ID]     = DurationMean[ID] + (Delta/Individuals[ID]);
  DurationM2[ID]       = DurationM2[ID] + Delta * (x - DurationMean[ID]);

  DurationStdDev_2[ID] = DurationM2[ID]/(Individuals[ID]-1);

  /* Update Clustering Metrics values */
  for (size_t CurrentMetric = 0;  CurrentMetric < ClusteringMetrics.size(); CurrentMetric++)
  {
    ClusteringMetrics[CurrentMetric].Update(ID, Burst->GetRawDimension(CurrentMetric));
  }
  
  /* Update Extrapolation Metrics values */
  map<size_t, double>::const_iterator Iterator;

  for (Iterator  = Burst->GetExtrapolationDimensions().begin();
       Iterator != Burst->GetExtrapolationDimensions().end();
       ++Iterator)
  {
    size_t MetricPosition = Iterator->first;
    double MetricValue   = Iterator->second;

    /*
    if (ExtrapolationMetrics[MetricPosition].GetMetricName().compare("PM_CMPLU_STALL_FDIV") == 0)
    {
      cout << "Task = " << Burst->GetTaskId() << "Time = " << Burst->GetBeginTime () << " ID = " << ID << " FDIV NEW VALUE = ";
      cout.precision(0);
      cout << fixed<< MetricValue << endl;
    }
    */
    
    ExtrapolationMetrics[MetricPosition].Update(ID, MetricValue);
  }
  
  return true;
}

