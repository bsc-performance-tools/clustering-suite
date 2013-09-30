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

#include "libDistributedClustering.hpp"
#include "libDistributedClusteringImplementation.hpp"

#include <SystemMessages.hpp>
using cepba_tools::system_messages;

/**
 * Empty constructor
 */
libDistributedClustering::libDistributedClustering(int         verbose,
                                                   const char* msg_prefix)
{
  Implementation  = new libDistributedClusteringImplementation(verbose,
                                                               msg_prefix);
  Error = Warning = false;
}

/**
 * Full initialization of the library. To be used in the leaf nodes. It
 * contains the definition of the events to be extracted as well as the
 * identification of the ranks
 *
 * \param ClusteringDefinitionXML XML to define the data extraction
 * \param Epsilon                 Epsilon parameter for DBSCAN
 * \param MinPoints               MinPoints parameter for DBSCAN
 * \param                         Root boolean to express if current task is
 *                                the root of the analysis
 * \param MyRank                  Rank identifier
 * \param TotalRanks              Total number of analysis tasks
 *
 * \return  True if the initialization was performed correctly, false otherwise
 */
bool libDistributedClustering::InitClustering(string ClusteringDefinitionXML,
                                              double Epsilon,
                                              int    MinPoints,
                                              bool   Root,
                                              int    MyRank,
                                              int    TotalRanks)
{
  this->Root = Root;

  if (!Implementation->InitClustering(ClusteringDefinitionXML,
                                      Epsilon,
                                      MinPoints,
                                      Root,
                                      MyRank,
                                      TotalRanks))
  {
    Error        = true;
    ErrorMessage = Implementation->GetLastError();
    return false;
  }

  if (Implementation->GetWarning())
  {
    Warning = true;
    WarningMessage = Implementation->GetLastWarning();
  }

  return true;
}



/**
 * Full initialization of the library excluding the DBSCAN parameters. This
 * parameters will be obtained from the XML
 *
 * \param ClusteringDefinitionXML XML to define the data extraction
 * \param                         Root boolean to express if current task is
 *                                the root of the analysis
 * \param MyRank                  Rank identifier
 * \param TotalRanks              Total number of analysis tasks
 *
 * \return  True if the initialization was performed correctly, false otherwise
 */
bool libDistributedClustering::InitClustering(string ClusteringDefinitionXML,
                                              bool   Root,
                                              int    MyRank,
                                              int    TotalRanks)
{
  this->Root = Root;

  if (!Implementation->InitClustering(ClusteringDefinitionXML,
                                      Root,
                                      MyRank,
                                      TotalRanks))
  {
    Error        = true;
    ErrorMessage = Implementation->GetLastError();
    return false;
  }

  if (Implementation->GetWarning())
  {
    Warning = true;
    WarningMessage = Implementation->GetLastWarning();
  }

  return true;
}

/**
 * Returns the Epsilon value used in the library
 *
 * \return Epsilon used in the library
 */
double libDistributedClustering::GetEpsilon(void)
{
  return Implementation->GetEpsilon();
}

/**
 * Returns the MinPoints value used in the library
 *
 * \return MinPoints used in the library
 */

int libDistributedClustering::GetMinPoints(void)
{
  return Implementation->GetMinPoints();
}

/**
 * Performs the extraction of the data from the trace file whose name is
 * received by parameter
 *
 * \param InputFileName Name of the trace file where the data will be extracted
 * \param TasksToRead   Set of TaskIDs to be read by this analysis
 *
 * \return True if the data extraction was performed correctly, false otherwise
 */
bool libDistributedClustering::ExtractData(string    InputFileName,
                                           set<int> &TasksToRead)
{
  if (!Implementation->ExtractData(InputFileName, TasksToRead))
  {
    Error        = true;
    ErrorMessage = Implementation->GetLastError();
    return false;
  }

  return true;
}

/**
 * Performs the extraction of the data from the trace file whose name is
 * received by parameter
 *
 * \param InputFileName Name of the trace file where the data will be extracted
 *
 * \return True if the data extraction was performed correctly, false otherwise
 */
bool libDistributedClustering::ExtractData(string InputFileName)
{
  if (!Implementation->ExtractData(InputFileName))
  {
    Error        = true;
    ErrorMessage = Implementation->GetLastError();
    return false;
  }

  return true;
}

/**
 * Returns the number of points to be processed locally
 *
 *
 * \return The total number of points to be analyzed
 */
size_t libDistributedClustering::GetNumberOfPoints(void)
{
  return Implementation->GetNumberOfPoints();
}

/**
 * Performs the extraction of the data from the trace file whose name is
 * received by parameter, just loading the bursts from those Tasks indicated
 *
 * \param InputFileName Name of the trace file where the data will be extracted
 * \param TasksToRead   Set of TaskIDs to be read by this analysis
 *
 * \return True if the data extraction was performed correctly, false otherwise
 */
bool libDistributedClustering::ClusterAnalysis(vector<HullModel*> &ClusterModels)
{
  if (!Implementation->ClusterAnalysis(ClusterModels))
  {
    Error        = true;
    ErrorMessage = Implementation->GetLastError();
    return false;
  }

  return true;
}

/**
 * Classifies the loaded data using the Convex Hull models received
 *
 * \param ClusterModes Vector of Convex Hull models per cluster
  *
 * \return True if the data classification was performed correctly, false
 *         otherwise
 */
bool libDistributedClustering::ClassifyData(vector<HullModel*> &ClusterModels)
{
  if (!Implementation->ClassifyData(ClusterModels))
  {
    Error = true;
    ErrorMessage = Implementation->GetLastError();
    return false;
  }

  return true;
}

/**
 * Reconstruct the input trace adding the data partition computed. Only
 * executes in the Root node
 *
 * \param OutputTraceName Name of the output trace with the cluster information
 *
 * \return True if the analysis task is not root or the trace has been
 *         generated correctly, false otherwise
 */
bool libDistributedClustering::ReconstructInputTrace(string OutputTraceName)
{
  if (!Root)
  {
    return true;
  }

  if (!Implementation->ReconstructInputTrace(OutputTraceName))
  {
    Error = true;
    ErrorMessage = Implementation->GetLastError();
    return false;
  }

  return true;
}

/**
 * Write clusters information to an output file
 *
 * \param OutputClustersInfoFileName Name of the output file where clusters information will be written
 *
 * \result True if output file is written correctly, false otherwise
 */
bool libDistributedClustering::FlushClustersInformation(string OutputClustersInfoFileName)
{
  if (!Root)
  {
    return true;
  }

  if (!Implementation->FlushClustersInformation(OutputClustersInfoFileName))
  {
    Error        = true;
    ErrorMessage = Implementation->GetLastError();
    return false;
  }

  return true;
}

/**
 * Initialization of the library to be used in the filter nodes. This
 * initialization is intended to be used supplying the data set, not using
 * any data extraction
 *
 * \param Epsilon                 Epsilon parameter for DBSCAN
 * \param MinPoints               MinPoints parameter for DBSCAN
 *
 * \return True if the initialization was performed correctly, false otherwise
 */
bool libDistributedClustering::InitClustering(double Epsilon,
                                              int    MinPoints)
{
  if (!Implementation->InitClustering(Epsilon,
                                      MinPoints))
  {
    Error        = true;
    ErrorMessage = Implementation->GetLastError();
    return false;
  }

  if (Implementation->GetWarning())
  {
    Warning = true;
    WarningMessage = Implementation->GetLastWarning();
  }

  return true;
}

/**
 * Performs a cluster analysis using the vector of points supplied by the user.
 * This method is intended to be used in the filter nodes, so as to determine
 * clusters in the noise points of the leaves
 *
 * \param Points        Vector of points to be clustered
 * \param ClusterModels Convex Hull models resulting from the cluster analysis
 *
 * \return True if the cluster analysis was performed correctly, false
 *         otherwise
 */
bool libDistributedClustering::ClusterAnalysis(const vector<const Point*> &Points,
                                               const vector<long long>    &Durations,
                                               vector<HullModel*>         &ClusterModels)
{
  if (!Implementation->ClusterAnalysis(Points,
                                       Durations,
                                       ClusterModels))
  {
    Error        = true;
    ErrorMessage = Implementation->GetLastError();
    return false;
  }

  if (Implementation->GetWarning())
  {
    Warning = true;
    WarningMessage = Implementation->GetLastWarning();
  }

  return true;
}

/**
 * Returns the noise points resulting from a cluster analysis
 *
 * \param I/O vector where the noise points will be stored
 *
 * \return True if the points were correctly returned, false otherwise
 */
bool libDistributedClustering::GetNoisePoints(vector<const Point*>& NoisePoints, vector<long long>& NoiseDurations)
{
  if (!Implementation->GetNoisePoints(NoisePoints, NoiseDurations))
  {
    Error        = true;
    ErrorMessage = Implementation->GetLastError();
    return false;
  }

  if (Implementation->GetWarning())
  {
    Warning = true;
    WarningMessage = Implementation->GetLastWarning();
  }

  return true;
}

/**
 * Adds a new burst to the 'TraceData' container, extracted from external
 * sources
 *
 * \param TaskId     Task identifier of the burst added
 * \param ThreadId   Thread identifier of the burst added
 * \param Line       Line in the tracefile UNUSED
 * \param BeginTime  Initial time where the burst appears in the application
 *                   execution
 * \param EndTime    Final time of the current burst
 * \param Duration   Duration of the burst
 * \param EventsData Map containing the event type/value pairs related to the
 *                   current burst
 *
 * \return True if the burst was correctly added to the Data container, false
 *         otherwise
 */
bool libDistributedClustering::NewBurst(task_id_t                         TaskId,
                                        thread_id_t                       ThreadId,
                                        line_t                            Line,
                                        timestamp_t                       BeginTime,
                                        timestamp_t                       EndTime,
                                        duration_t                        BurstDuration,
                                        map<event_type_t, event_value_t>& EventsData)
{
  if (!Implementation->NewBurst(TaskId,
                                ThreadId,
                                Line,
                                BeginTime,
                                EndTime,
                                BurstDuration,
                                EventsData))
  {
    Error    = true;
    ErrorMessage = Implementation->GetLastError();
    return false;
  }

  return true;
}

/**
 * Returns the ranges of the clustering parameters observed in the current
 * instance of the library
 *
 * \param MinValues Output vector containing the minimum values observed in
 *                  the clustering parameters of the current instance
 * \param MaxValues Output vector containing the maximum values observed in
 *                  the clustering parameters of the current instance
 *
 */
void libDistributedClustering::GetParameterRanges(vector<double>& MinValues,
                                                  vector<double>& MaxValues)
{
  Implementation->GetParameterRanges(MinValues, MaxValues);

  return;
}

/**
 * Normalize clustering dimension of the data bursts stored in the current
 * instance of the library using the ranges provided
 *
 * \param MinValues Minimum values of the clustering parameters
 * \param MaxValues Maximum values of the clustering parameters
 *
 */
void libDistributedClustering::NormalizeData(vector<double>& MinValues,
                                             vector<double>& MaxValues)
{
  Implementation->NormalizeData(MinValues, MaxValues);

  return;
}

/**
 * Returns all the average statistics accumulated on each cluster obtained
 * after classifying the data (the final partition), so as to aggregate the
 * results with the values of all workers
 *
 * \param Statistics I/O vector containing the ClusterStatistics objects that
 *                   aggregate the average statistics
 *
 * \return True on success, false otherwise
 */
bool libDistributedClustering::GetClusterStatistics(vector<ClusterStatistics*>& Statistics)
{
  if (!Implementation->GetClusterStatistics(Statistics))
  {
    Error    = true;
    ErrorMessage = Implementation->GetLastError();
    return false;
  }

  return true;
}

/**
 * Returns all information of the bursts analyzed, in a simplistic way, so as
 * to perform external analysis. All parameters are I/O
 *
 * \param Points     Points (bursts) used in the cluster analysis using raw
 *                   dimensions
 * \param TaskIDs    TaskID from the input trace associated to each burst
 * \param ThreadIDs  ThreadID from the input trace associated to each burst
 * \param ClusterIDs Cluster IDs obtained after the classification of the data
 *                   (considering that the classification provides the
 *                   definitive partition
 *
 * \return True, if the information was succesfully retrieved, false otherwise
 */
bool libDistributedClustering::GetFullBurstsInformation(vector<Point*>       &Points,
/*
                                                        vector<task_id_t>    &TaskIDs,
                                                        vector<thread_id_t>  &ThreadIDs,
*/
                                                        vector<timestamp_t>  &BeginTimes,
                                                        vector<timestamp_t>  &EndTimes,
                                                        vector<cluster_id_t> &ClusterIDs)
{
  if (!Implementation->GetFullBurstsInformation(Points, /* TaskIDs, ThreadIDs, */ BeginTimes, EndTimes, ClusterIDs))
  {
    Error        = true;
    ErrorMessage = Implementation->GetLastError();
    return false;
  }

  if (Implementation->GetWarning())
  {
    Warning        = true;
    WarningMessage = Implementation->GetLastWarning();
  }

  return true;
}

/**
 * Returns the Cluster IDs obtained in the last cluster analys
 *
 * \param ClusterIDs I/O vector containing the resulting cluster IDs
 *
 * \result True if the operation finished succesfuly, false otherwise
 */
bool libDistributedClustering::GetClusterIDs(vector<cluster_id_t> &ClusterIDs)
{
  if (!Implementation->GetClusterIDs(ClusterIDs))
  {
    Error        = true;
    ErrorMessage = Implementation->GetLastError();
    return false;
  }

  if (Implementation->GetWarning())
  {
    Warning        = true;
    WarningMessage = Implementation->GetLastWarning();
  }

  return true;
}


/**
 * Generates the scripts and the data files to display the scatter plots of the
 * data using GNUplot
 *
 * \param DataFileName          Name of the file where the data will be stored
 * \param ScriptsFileNamePrefix Base name to be used in the different GNUplot
 *                              scripts
 * \param LocalPartition        Boolean to indicate if the data to be printed
 *                              is the global partition or just the local
 *                              analysis
 *
 * \return True if the plots scripts and data files were written correctly,
 *         false otherwise
 */
bool libDistributedClustering::PrintPlotScripts(string DataFileName,
                                                string ScriptsFileNamePrefix,
                                                bool   LocalPartition)
{
  if (!Implementation->PrintPlotScripts (DataFileName,
                                         ScriptsFileNamePrefix,
                                         LocalPartition))
  {
    Error = true;
    ErrorMessage = Implementation->GetLastError();
    return false;
  }

  return true;
}

/**
 * Generates the scripts that print the scatters plots merging all the local
 * partitions of the backends
 *
 * \param DataFileName          String with the filenames where the data is
 *                              stored
 * \param ScriptsFileNamePrefix Base name to be used in the different GNUplot
 *                              scripts
 * \param ClustersCounts        Number of different clusters detected by
 *                              the cluster algorithm
 * \param PrintingModels        True when printing cluster models instead of
 *                              data points
 *
 * \return True if the plots scripts and data files were written correctly,
 *         false otherwise
 */
bool libDistributedClustering::PrintGlobalPlotScripts(string       DataFileName,
                                                      string       ScriptsFileNamePrefix,
                                                      unsigned int ClustersCount,
                                                      bool         PrintingModels)
{
  if (!Implementation->PrintGlobalPlotScripts (DataFileName,
                                               ScriptsFileNamePrefix,
                                               ClustersCount,
                                               PrintingModels))
  {
    Error = true;
    ErrorMessage = Implementation->GetLastError();
    return false;
  }

  return true;
}

/**
 * Prints the data files and GNUplot scripts of the Convex Hull models supplied
 *
 * \param ClusterModels         Vector of Convex Hull models of the clusters
 *                              that will be printed
 * \param ModelsFileName        Name of the file where the models will be
 *                              stored
 * \param ScriptsFileNamePrefix Base name to be used in the different GNUplot
 *                              scripts
 *
* \param Title                 Title to be used in GNUplot script
 *
 * \return True if the plot scripts and data files were written correctly,
 *         false otherwise
 */
bool libDistributedClustering::PrintModels(vector<HullModel*> &ClusterModels,
                                           string              ModelsFileName,
                                           string              ScriptsFileNamePrefix,
                                           string              Title)
{

  if (!Implementation->PrintModels(ClusterModels,
                                   ModelsFileName,
                                   ScriptsFileNamePrefix,
                                   Title))
  {
    Error        = true;
    ErrorMessage = Implementation->GetLastError();
    return false;
  }
  return true;
}

/**
 * Returns the string containing the last error message
 *
 * \return Last error message
 */
string libDistributedClustering::GetErrorMessage(void)
{
  return ErrorMessage;
}

/**
 * Returns the string containing the last warning message
 *
 * \return Last warning message
 */
string libDistributedClustering::GetWarningMessage(void)
{
  return WarningMessage;
}
