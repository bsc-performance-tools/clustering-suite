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

#include <ClusteringConfiguration.hpp>
#include "DataExtractor.hpp"
#include "DataExtractorFactory.hpp"
#include "ClusteringStatistics.hpp"
#include "ClusteredTraceGenerator.hpp"
#include "ClusteredStatesPRVGenerator.hpp"
#include "ClusteredEventsPRVGenerator.hpp"
#include "ClusteredTRFGenerator.hpp"
#include "PlottingManager.hpp"

#include "ConvexHullModel.hpp"
#include "ConvexHullClassifier.hpp"
#include "DBSCAN.hpp"

#include <SystemMessages.hpp>
using cepba_tools::system_messages;
#include <FileNameManipulator.hpp>
using cepba_tools::FileNameManipulator;

#include <fstream>
using std::ofstream;
using std::ios_base;

#include <sstream>
using std::ostringstream;

/****************************************************************************
 * PUBLIC METHODS
 ***************************************************************************/

/**
 * Empty constructor
 */
libDistributedClusteringImplementation::libDistributedClusteringImplementation(int verbose)
{
  system_messages::distributed = true;

  switch(verbose)
  {
    case SILENT:
      system_messages::verbose = false;
      break;
    case VERBOSE:
      system_messages::verbose                 = true;
      system_messages::messages_from_all_ranks = false;
      break;
    case VVERBOSE:
      system_messages::verbose                 = true;
      system_messages::messages_from_all_ranks = true;
    default:
      system_messages::verbose                 = false;
      break;
  }
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
bool libDistributedClusteringImplementation::InitClustering(string ClusteringDefinitionXML,
                                                            double Epsilon,
                                                            INT32  MinPoints,
                                                            bool   Root,
                                                            INT32  MyRank,
                                                            INT32  TotalRanks)
{
  ClusteringConfiguration *ConfigurationManager;


  ConfigurationManager = ClusteringConfiguration::GetInstance();

  if (!ConfigurationManager->Initialize(ClusteringDefinitionXML))
  {
    SetError(true);
    SetErrorMessage(ConfigurationManager->GetLastError());
    return false;
  }

  this->Epsilon   = Epsilon;
  this->MinPoints = MinPoints;

  return CommonInitialization(ConfigurationManager,
                              Root,
                              MyRank,
                              TotalRanks);
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
bool libDistributedClusteringImplementation::InitClustering(string ClusteringDefinitionXML,
                                                            bool   Root,
                                                            INT32  MyRank,
                                                            INT32  TotalRanks)
{
  ClusteringConfiguration      *ConfigurationManager;
  string                        ClusteringAlgorithmName;
  map<string, string>           ClusteringAlgorithmParameters;
  map<string, string>::iterator ParametersIterator;

  ConfigurationManager = ClusteringConfiguration::GetInstance();

  if (!ConfigurationManager->Initialize(ClusteringDefinitionXML))
  {
    SetError(true);
    SetErrorMessage(ConfigurationManager->GetLastError());
    return false;
  }

  ClusteringAlgorithmName = ConfigurationManager->GetClusteringAlgorithmName();

  if (ClusteringAlgorithmName.compare(DBSCAN::NAME) != 0)
  {
    ostringstream ErrorMessage;

    ErrorMessage << "Algorithm '" << ClusteringAlgorithmName << "' not supported ";
    ErrorMessage << "in distribution version, please use 'DBSCAN'";

    SetError(true);
    SetErrorMessage(ErrorMessage.str());
    return false;
  }

  ClusteringAlgorithmParameters = ConfigurationManager->GetClusteringAlgorithmParameters();

  /* Epsilon */
  ParametersIterator = ClusteringAlgorithmParameters.find(DBSCAN::EPSILON_STRING);
  if (ParametersIterator == ClusteringAlgorithmParameters.end())
  {
    string ErrorMessage;
    ErrorMessage = "parameter '" + DBSCAN::EPSILON_STRING + "' not found in DBSCAN definition";

    SetErrorMessage(ErrorMessage);
    SetError(true);
    return false;
  }
  else
  {
    char* err;
    Epsilon = strtod(ParametersIterator->second.c_str(), &err);

    if (*err)
    {
      string ErrorMessage;
      ErrorMessage = "incorrect value for DBSCAN parameter '"+ DBSCAN::EPSILON_STRING + "'";

      SetErrorMessage(ErrorMessage);
      SetError(true);
      return false;
    }
  }

  /* MinPoints */
  ParametersIterator = ClusteringAlgorithmParameters.find(DBSCAN::MIN_POINTS_STRING);
  if (ParametersIterator == ClusteringAlgorithmParameters.end())
  {
    string ErrorMessage;
    ErrorMessage = "parameter '" + DBSCAN::MIN_POINTS_STRING + "' not found in DBSCAN definition";

    SetErrorMessage(ErrorMessage);
    SetError(true);
    return false;
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
      return false;
    }
  }

  if (!ConfigurationManager->Initialize(ClusteringDefinitionXML))
  {
    SetError(true);
    SetErrorMessage(ConfigurationManager->GetLastError());
    return false;
  }

  return CommonInitialization(ConfigurationManager,
                              Root,
                              MyRank,
                              TotalRanks);

  return true;
}

/**
 * Returns the Epsilon value used in the library
 *
 * \return Epsilon used in the library
 */
double libDistributedClusteringImplementation::GetEpsilon(void)
{
  return this->Epsilon;
}

/**
 * Returns the MinPoints value used in the library
 *
 * \return MinPoints used in the library
 */

int libDistributedClusteringImplementation::GetMinPoints(void)
{
  return this->MinPoints;
}

/**
 * Performs the extraction of the data from the trace file whose name is
 * received by parameter
 *
 * \param InputFileName    Name of the trace file where the data will be
 *                         extracted
 * \param TasksToRead      Set of TaskIDs to be read by this analysis
 * \param EventsToDealWith Events to extract (UNUSED IN THIS VERSION)
 *
 * \return True if the data extraction was performed correctly, false otherwise
 */
bool libDistributedClusteringImplementation::ExtractData(string            InputFileName,
                                                         set<int>&         TasksToRead,
                                                         set<event_type_t> EventsToDealWith)
{
  DataExtractorFactory*    ExtractorFactory;
  DataExtractor*           Extractor;

  /* Get the container */
  Data = TraceData::GetInstance();

  /* Set the subset to read */
  Data->SetMaster(Root);
  Data->SetTasksToRead (TasksToRead);

  /* Get input file extractor */
  ExtractorFactory = DataExtractorFactory::GetInstance();

  if (EventsToDealWith.size() > 0)
  {
    PRVEventsParsing       = true;
    this->EventsToDealWith = EventsToDealWith;
  }
  else
  {
    PRVEventsParsing = false;
  }

  if (!ExtractorFactory->GetExtractor(InputFileName,
                                      Extractor,
                                      PRVEventsParsing))
  {
    SetError(true);
    SetErrorMessage(ExtractorFactory->GetLastError());
    return false;
  }

  this->InputFileName = InputFileName;
  InputFileType       = Extractor->GetFileType();

  if (PRVEventsParsing)
  {
    if (!Extractor->SetEventsToDealWith(EventsToDealWith))
    {
      SetError(true);
      SetErrorMessage(Extractor->GetLastError());
      return false;
    }
  }

  if (!Extractor->ExtractData(Data))
  {
    SetError(true);
    SetErrorMessage(Extractor->GetLastError());
    return false;
  }

  return true;
}

/**
 * Performs the extraction of the data from the trace file whose name is
 * received by parameter
 *
 * \param InputFileName Name of the trace file where the data will be extracted
 * \param EventsToDealWith Events to extract (UNUSED IN THIS VERSION)
 *
 * \return True if the data extraction was performed correctly, false otherwise
 */
bool libDistributedClusteringImplementation::ExtractData(string            InputFileName,
                                                         set<event_type_t> EventsToDealWith)
{
  DataExtractorFactory*    ExtractorFactory;
  DataExtractor*           Extractor;

  /* Get the container */
  Data = TraceData::GetInstance();

  /* Set the subset to read */
  Data->SetMaster(Root);

  /* Get input file extractor */
  ExtractorFactory = DataExtractorFactory::GetInstance();

  if (EventsToDealWith.size() > 0)
  {
    PRVEventsParsing       = true;
    this->EventsToDealWith = EventsToDealWith;
  }
  else
  {
    PRVEventsParsing = false;
  }

  if (!ExtractorFactory->GetExtractor(InputFileName,
                                      Extractor,
                                      PRVEventsParsing))
  {
    SetError(true);
    SetErrorMessage(ExtractorFactory->GetLastError());
    return false;
  }

  this->InputFileName = InputFileName;
  InputFileType       = Extractor->GetFileType();

  if (PRVEventsParsing)
  {
    if (!Extractor->SetEventsToDealWith(EventsToDealWith))
    {
      SetError(true);
      SetErrorMessage(Extractor->GetLastError());
      return false;
    }
  }

  if (!Extractor->ExtractData(Data))
  {
    SetError(true);
    SetErrorMessage(Extractor->GetLastError());
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
size_t libDistributedClusteringImplementation::GetNumberOfPoints(void)
{
  if (Data == NULL)
  {
    return (size_t) 0;
  }
  else
  {
    if (UsingExternalData)
    {
      return ExternalData.size();
    }
    else
    {
      return Data->GetClusteringBurstsSize();
    }
  }
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
bool libDistributedClusteringImplementation::ClusterAnalysis(vector<HullModel*>& ClusterModels)
{
  ClusteringConfiguration* ConfigurationManager;
  ClusteringStatistics     Statistics;
  size_t                   ExtrapolationMetrics;
  ostringstream            Messages;

  if (Data == NULL)
  {
    SetErrorMessage("data not initialized");
    return false;
  }

  ConfigurationManager = ClusteringConfiguration::GetInstance();
  if (!ConfigurationManager->IsInitialized())
  { /* Should never happen! */
    SetErrorMessage("configuration not initialized");
    return false;
  }

  ExtrapolationMetrics = ConfigurationManager->GetExtrapolationParametersNames().size();

  vector<const Point*>& ClusteringPoints = Data->GetClusteringPoints();


  /* DEBUG
  Messages << "Clustering Points size = " << Data->GetClusteringPoints().size() << endl;
  system_messages::information(Messages.str().c_str());
  Messages.str("");
  Messages << "Complete Points size = " << Data->GetCompleteBursts().size() << endl;
  system_messages::information(Messages.str().c_str());
  */

  /* 'ClusteringCore' has been initialized in 'InitTraceClustering' method */
  if (!ClusteringCore->ExecuteClustering(ClusteringPoints, LastPartition))
  {
    SetErrorMessage(ClusteringCore->GetErrorMessage());
    return false;
  }

  if (!GenerateClusterModels(ClusterModels))
  { /* Error message will be generated in the private method */
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
bool libDistributedClusteringImplementation::ClassifyData(vector<HullModel*>& ClusterModels)
{
  ostringstream Messages;

  vector<const Point*>& CompletePoints = ( UsingExternalData ? ExternalData : (vector<const Point*>&) Data->GetCompleteBursts() );
  vector<ConvexHullModel> InternalHulls;

  for (size_t i = 0; i < ClusterModels.size(); i++)
  {
    ConvexHullModel* CurrentConvexHull = ClusterModels[i]->Model();
    InternalHulls.push_back(*CurrentConvexHull);
  }

  ConvexHullClassifier ClassifierCore(InternalHulls, Epsilon, MinPoints);

  /* DEBUG
  Messages << "Hulls used to classify points = " << ClusterModels.size() << endl;
  system_messages::information(Messages.str().c_str());
  */

  ClassifierCore.Classify(CompletePoints, ClassificationPartition);

  GenerateStatistics(true); // true = UseClassificationPartition

  return true;
}

/**
 * Generates the statistics obtained from the selected partition
 *
 * \param UseClassificationPartition True if the statistics come from the
 *                                   classification partition, false if we
 *                                   want to use the local partition
 *
 * \return True if the statistics were correctly generated, false otherwise
 */
bool libDistributedClusteringImplementation::GenerateStatistics (bool UseClassificationPartition)
{
  ClusteringConfiguration* ConfigurationManager;
  ParametersManager*       Parameters;


  ConfigurationManager = ClusteringConfiguration::GetInstance();
  if (!ConfigurationManager->IsInitialized())
  { /* Should never happen! */
    SetErrorMessage("configuration not initialized");
    return false;
  }

  /* Statistics */

  Parameters = ParametersManager::GetInstance();

  if (UseClassificationPartition)
  {
    Statistics.InitStatistics(ClassificationPartition.GetIDs(),
                              Parameters->GetClusteringParametersNames(),
                              Parameters->GetClusteringParametersPrecision(),
                              Parameters->GetExtrapolationParametersNames(),
                              Parameters->GetExtrapolationParametersPrecision());

    if (!Statistics.ComputeStatistics(Data->GetCompleteBursts(),
                                      ClassificationPartition.GetAssignmentVector()))
    {
      SetErrorMessage(Statistics.GetLastError());
      return false;
    }
    Statistics.TranslatedIDs(ClassificationPartition.GetAssignmentVector());
  }
  else
  {
    Statistics.InitStatistics(LastPartition.GetIDs(),
                              Parameters->GetClusteringParametersNames(),
                              Parameters->GetClusteringParametersPrecision(),
                              Parameters->GetExtrapolationParametersNames(),
                              Parameters->GetExtrapolationParametersPrecision());

    if (!Statistics.ComputeStatistics(Data->GetClusteringBursts(),
                                      LastPartition.GetAssignmentVector()))
    {
      SetErrorMessage(Statistics.GetLastError());
      return false;
    }
    Statistics.TranslatedIDs(LastPartition.GetAssignmentVector());
  }

  /*
  vector<cluster_id_t>& AssignmentVector = CurrentPartition.GetAssignmentVector();

  ostringstream Messages;
  for (size_t i = 0; i < AssignmentVector.size(); i++)
  {
    Messages.str("");
    Messages << "Point[" << i << "] ID = " << AssignmentVector[i] << endl;
    system_messages::information(Messages.str().c_str());
  }
  */

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
bool libDistributedClusteringImplementation::ReconstructInputTrace(string OutputTraceName)
{
  ClusteredTraceGenerator* TraceReconstructor;

  if (ClassificationPartition.NumberOfClusters() == 0)
  {
    SetError(true);
    SetErrorMessage("classification not performed");
  }

  vector<cluster_id_t>& IDs = ClassificationPartition.GetAssignmentVector();

  if (IDs.size() == 0)
  {
    SetErrorMessage("no cluster analysis data available to reconstruct the input trace");
    return false;
  }

  switch(InputFileType)
  {
    case ParaverTrace:
      if (PRVEventsParsing)
      {
        TraceReconstructor = new ClusteredEventsPRVGenerator(InputFileName, OutputTraceName);
        TraceReconstructor->SetEventsToDealWith (EventsToDealWith);
      }
      else
      {
        TraceReconstructor = new ClusteredStatesPRVGenerator(InputFileName, OutputTraceName);
      }
      break;
    case DimemasTrace:
      /* TBI -> Dimemas Cluster blocks not implemented */
      TraceReconstructor = new ClusteredTRFGenerator (InputFileName, OutputTraceName);
      break;
    default:
      SetErrorMessage("unable to reconstruct an input file which is not a trace");
      return false;
  }

  if (TraceReconstructor->GetError())
  {
    SetErrorMessage(TraceReconstructor->GetLastError());
    return false;
  }



  if (!TraceReconstructor->Run(Data->GetAllBursts(),
                               ClassificationPartition.GetAssignmentVector(),
                               ClassificationPartition.GetIDs()))
  {
    SetErrorMessage(TraceReconstructor->GetLastError());
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
bool libDistributedClusteringImplementation::FlushClustersInformation(string OutputClustersInfoFileName)
{
  ofstream OutputStream (OutputClustersInfoFileName.c_str(), ios_base::trunc);

  if (ClassificationPartition.NumberOfClusters() == 0)
  {
    SetError(true);
    SetErrorMessage("classification not performed");
  }

  vector<cluster_id_t>& IDs = ClassificationPartition.GetAssignmentVector();

  if (IDs.size() == 0)
  {
    SetErrorMessage("no cluster analysis data available to generate information file");
    return false;
  }

  if (!Statistics.Flush(OutputStream))
  {
    SetErrorMessage(Statistics.GetLastError());
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
bool libDistributedClusteringImplementation::InitClustering(double Epsilon,
                                                            int    MinPoints)
{
  string                   ClusteringAlgorithmName;
  map<string, string>      ClusteringAlgorithmParameters;
  ostringstream            Converter;

  /* Check if clustering library could be correctly initialized */
  ClusteringCore = new libClustering();

  /* Ignore the clustering algorithm defined in the XML and force it to DBSCAN */
  ClusteringAlgorithmName = DBSCAN::NAME;

  Converter << Epsilon;
  ClusteringAlgorithmParameters.insert(std::make_pair(DBSCAN::EPSILON_STRING, string(Converter.str())));
  Converter.str("");
  Converter << MinPoints;
  ClusteringAlgorithmParameters.insert(std::make_pair(DBSCAN::MIN_POINTS_STRING, string(Converter.str())));

  if (!ClusteringCore->InitClustering (ClusteringAlgorithmName,
                                       ClusteringAlgorithmParameters))
  {
    SetError(true);
    SetErrorMessage(ClusteringCore->GetErrorMessage());
    return false;
  }
  this->Epsilon   = Epsilon;
  this->MinPoints = MinPoints;

  UsingExternalData = true;

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
bool libDistributedClusteringImplementation::ClusterAnalysis(const vector<const Point*> &Points,
                                                             vector<HullModel*>         &ClusterModels)
{
  /* 'ClusteringCore' has been initialized in 'InitTraceClustering' method */
  ExternalData = Points;

  if (!ClusteringCore->ExecuteClustering(ExternalData, LastPartition))
  {
    SetErrorMessage(ClusteringCore->GetErrorMessage());
    return false;
  }

  if (!GenerateClusterModels(ClusterModels))
  { /* Error message will be generated in the private method */
    return false;
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
bool libDistributedClusteringImplementation::GetNoisePoints(vector<const Point*>& NoisePoints)
{
  vector<cluster_id_t>& IDs = LastPartition.GetAssignmentVector();

  if (IDs.size() == 0)
  {
#if 0 // OR CLUSTERED AN EMPTY VECTOR OF NOISE POINTS!!!
    SetErrorMessage("cluster analysis not executed");
    return false;
#else
    NoisePoints.clear();
    return true;
#endif
  }

  vector<const Point*>& DataPoints = GetDataPoints();

  NoisePoints.clear();

  for (size_t i = 0; i < IDs.size(); i++)
  {
//std::cout << "GetNoisePoints IDs[" << i << "] = " << IDs[i] << " (NOISE_CLUSTERID=" << NOISE_CLUSTERID << ")" << endl;
    if (IDs[i] == NOISE_CLUSTERID)
    {
      NoisePoints.push_back(DataPoints[i]);
    }
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
bool libDistributedClusteringImplementation::GetFullBurstsInformation(vector<Point*>&       Points,
                                                                      vector<task_id_t>&    TaskIDs,
                                                                      vector<thread_id_t>&  ThreadIDs,
                                                                      vector<cluster_id_t>& ClusterIDs)
{
  vector<CPUBurst*>&    Bursts = Data->GetClusteringBursts();
  vector<cluster_id_t>& IDs    = ClassificationPartition.GetAssignmentVector();
  size_t Dimensions;

  if (UsingExternalData)
  {
    SetErrorMessage("unable to retrieve information from external data source");
    return false;
  }

  if (IDs.size() == 0)
  {
    SetErrorMessage("data needs to be classfied before being retrieved");
  }

  if (Bursts.size() != IDs.size())
  {
    ostringstream ErrorMessage;

    ErrorMessage << "different number of bursts (" << Bursts.size() << ") ";
    ErrorMessage << "than cluster IDs (" << IDs.size() << ")";

    SetErrorMessage(ErrorMessage.str());
    return false;
  }

  Points.clear();
  TaskIDs.clear();
  ThreadIDs.clear();
  ClusterIDs.clear();

  Dimensions = Data->GetClusteringDimensionsCount();

  for (size_t i = 0; i < Bursts.size(); i++)
  {
    Point* NewPoint = new Point(Bursts[i]->GetRawDimensions());

    Points.push_back(NewPoint);
    TaskIDs.push_back(Bursts[i]->GetTaskId());
    ThreadIDs.push_back(Bursts[i]->GetThreadId());
    ClusterIDs.push_back(IDs[i]);
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
bool libDistributedClusteringImplementation::PrintPlotScripts(string DataFileName,
                                                              string ScriptsFileNamePrefix,
                                                              bool   LocalPartition)
{
  PlottingManager *Plots;
  string Prefix;

  Plots = PlottingManager::GetInstance();

  Partition& UsedPartition = (LocalPartition ? LastPartition : ClassificationPartition);

  if (ScriptsFileNamePrefix.compare("") == 0)
  {
    FileNameManipulator Manipulator(DataFileName, "csv");
    Prefix = Manipulator.GetChoppedFileName();
  }
  else
  {
    Prefix = ScriptsFileNamePrefix;
  }

  if (!FlushData(DataFileName, LocalPartition))
  {
    return false;
  }

  if (!Plots->PrintPlots(DataFileName,
                         Prefix,
                         ClusteringCore->GetClusteringAlgorithmName(),
                         UsedPartition.GetIDs()))
  {
    SetError(true);
    SetErrorMessage(Plots->GetLastError());
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
 * \param Title                 Title to be used in GNUplot script
 *
 * \return True if the plot scripts and data files were written correctly,
 *         false otherwise
 */
bool libDistributedClusteringImplementation::PrintModels(vector<HullModel*> &ClustersModels,
                                                         string              ModelsFileName,
                                                         string              ScriptsFileNamePrefix,
                                                         string              Title)
{
  PlottingManager *Plots;
  string Prefix;

  set<cluster_id_t> DifferentIDs;

  ofstream OutputStream;
  ostringstream Messages;

  if (ScriptsFileNamePrefix.compare("") == 0)
  {
    FileNameManipulator Manipulator(ModelsFileName, "csv");
    Prefix = Manipulator.GetChoppedFileName();
  }
  else
  {
    Prefix = ScriptsFileNamePrefix;
  }

  OutputStream.open(ModelsFileName.c_str(), ios_base::trunc);
  if (!OutputStream)
  {
    SetError(true);
    SetErrorMessage("unable to open models data file name", strerror(errno));
    return false;
  }

  for (size_t i = 0; i < ClustersModels.size(); i++)
  {
    ConvexHullModel* Hull = ClustersModels[i]->Model();
    /* DEBUG
    Messages << "**** Printing Hull " << i << " to file " << ModelsFileName << endl;
    system_messages::information(Messages.str().c_str());
    Messages.str("");
    */
    Hull->Flush(OutputStream, MIN_CLUSTERID+i+PARAVER_OFFSET); // +1 because there is no noise!
    DifferentIDs.insert(i+1);
  }
  OutputStream.close();

  /* Print Model GNUPlot Script */
  if (ScriptsFileNamePrefix.compare("") == 0)
  { /* No need to print scripts */
    return true;
  }

  /* Plot definition */
  Plots = PlottingManager::GetInstance();

  if (!Plots->PrintPlots(ModelsFileName, ScriptsFileNamePrefix, Title, DifferentIDs, true )) // true == Printing Models
  {
    SetError(true);
    SetErrorMessage(Plots->GetLastError());
    return false;
  }

  return true;
}

/****************************************************************************
 * PRIVATE METHODS
 ***************************************************************************/

/**
 * Common part of the initialization of the library
 *
 * \param ConfigurationManager    Manager of the clustering parametrs
 * \param Root                    boolean to express if current task is
 *                                the root of the analysis
 * \param MyRank                  Rank identifier
 * \param TotalRanks              Total number of analysis tasks
 *
 * \return  True if the initialization was performed correctly, false otherwise
 */
bool libDistributedClusteringImplementation::CommonInitialization(
  ClusteringConfiguration*& ConfigurationManager,
  bool   Root,
  INT32  MyRank,
  INT32  TotalRanks)
{
  ParametersManager       *Parameters;
  string                   ClusteringAlgorithmName;
  map<string, string>      ClusteringAlgorithmParameters;
  PlottingManager         *Plots;
  ostringstream            Converter;
  string                   EpsilonStr, MinPointsStr;

  /* Set that we work in a distributed environment */
  ConfigurationManager->SetDistributed(true);
  ConfigurationManager->SetMyRank(MyRank);
  ConfigurationManager->SetTotalRanks(TotalRanks);

  /* Check if parameters have been read properly */
  Parameters = ParametersManager::GetInstance();
  if (Parameters->GetError())
  {
    SetError(true);
    SetErrorMessage(Parameters->GetLastError());
//    printf("ERROR IN THE PARAMETERS MANAGER %s", Parameters->GetLastError());
    return false;
  }

  if (Parameters->GetWarning())
  {
    SetWarning(true);
    SetWarningMessage (Parameters->GetLastWarning());
  }

  /* Check if clustering library could be correctly initialized */
  ClusteringCore = new libClustering();

  /* Ignore the clustering algorithm defined in the XML and force it to DBSCAN */
  ClusteringAlgorithmName = DBSCAN::NAME;

  Converter << this->Epsilon;
  ClusteringAlgorithmParameters.insert(std::make_pair(DBSCAN::EPSILON_STRING, string(Converter.str())));
  Converter.str("");
  Converter << this->MinPoints;
  ClusteringAlgorithmParameters.insert(std::make_pair(DBSCAN::MIN_POINTS_STRING, string(Converter.str())));
  if (!ClusteringCore->InitClustering (ClusteringAlgorithmName,
                                       ClusteringAlgorithmParameters))
  {
    SetError(true);
    SetErrorMessage(ClusteringCore->GetErrorMessage());
    return false;
  }

  Plots = PlottingManager::GetInstance(false);

  if (Plots->GetError())
  {
    SetError(true);
    SetErrorMessage(Plots->GetLastError());
    return false;
  }

  system_messages::my_rank  = MyRank;
  this->Root                = Root;

  UsingExternalData = false;

  return true;
}

/**
 * Generates the Convex Hull models of the clusters determined by the Last
 * Partition
 *
 * \param Models I/O vector where the clusters models are stored
 *
 * \return True if the models were correctly generated, false otherwise
 */
bool libDistributedClusteringImplementation::GenerateClusterModels(vector<HullModel*>& Models)
{
  vector<cluster_id_t>& AssignmentVector = LastPartition.GetAssignmentVector();

  vector<const Point*>& ClusteringPoints = ( UsingExternalData ? ExternalData : Data->GetClusteringPoints() ) ;

  vector<vector<const Point*> > PointsPerCluster (LastPartition.NumberOfClusters ());

  /* DEBUG
  ostringstream Messages;
  Messages << "Creating Hulls. Number of clusters = " << LastPartition.NumberOfClusters() << endl;
  system_messages::information(Messages.str().c_str()); */

  Models.clear();

  /* DEBUG
  Messages.str("");
  Messages << " Models vector size = " << Models.size() << endl;
  system_messages::information(Messages.str().c_str()); */

  unsigned int ClusterPointsSize = ( UsingExternalData ? ExternalData.size() : Data->GetClusteringBurstsSize() );
  if (AssignmentVector.size() != ClusterPointsSize)
  {
    SetError(true);
    SetErrorMessage("partition elements and cluster points differ");
    return false;
  }

  for (size_t i = 0; i < ClusteringPoints.size(); i++)
  {
    PointsPerCluster[AssignmentVector[i]].push_back(ClusteringPoints[i]);
  }

  for (size_t i = NOISE_CLUSTERID+1; i < PointsPerCluster.size(); i++)
  {
    /*ConvexHullModel NewHull(PointsPerCluster[i]);
    NewHull.Print();
    Models.push_back(NewHull);*/

    Models.push_back(new HullModel(new ConvexHullModel(PointsPerCluster[i])));
  }

  return true;
}

/**
 * Returns a vector reference where the data is store
 *
 * \return A reference to the vector that contains the actual data;
 */
vector<const Point*>& libDistributedClusteringImplementation::GetDataPoints(void)
{
  if (UsingExternalData)
  {
    return ExternalData;
  }
  else
  {
    return Data->GetClusteringPoints();
  }
}

/**
 * Writes on disk the data stored in memory, adding the cluster information
 * obtained from a cluster analysis
 *
 * \param DataFileName   Name of the file where the data will be stored
 * \param LocalPartition Boolean to indicate if the partition to be printed
 *                       corresponds to the local one, or the final
 *                       classification
 *
 * \return True if the data were stored correctly, false otherwise
 */
bool libDistributedClusteringImplementation::FlushData(string DataFileName, bool LocalPartition)
{
  ofstream OutputStream (DataFileName.c_str(), ios_base::trunc);

  if (Data == NULL)
  {
    SetErrorMessage("data not initialized");
    return false;
  }

  if (!OutputStream)
  {
    SetError(true);
    SetErrorMessage("unable to open output file", strerror(errno));
    return false;
  }

  if (LocalPartition)
  {
    /* This won't work!!!!!!! */
    if (!Data->FlushPoints(OutputStream, LastPartition.GetAssignmentVector(), false)) // false = Not All Data!!
    {
      SetError(true);
      SetErrorMessage("error flushing local points", Data->GetLastError());
      return false;
    }
  }
  else
  {
    if (!Data->FlushPoints(OutputStream, ClassificationPartition.GetAssignmentVector(), false)) // false = Not All Data!!
    {
      SetError(true);
      SetErrorMessage("error flushing global points", Data->GetLastError());
      return false;
    }
  }

  OutputStream.close();

  return true;
}
