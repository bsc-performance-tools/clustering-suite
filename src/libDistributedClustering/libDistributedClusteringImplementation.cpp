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
#include "SemanticGuidedPRVGenerator.hpp"
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
libDistributedClusteringImplementation::libDistributedClusteringImplementation(int         verbose,
                                                                               const char* msg_prefix)
{
  system_messages::set_rank_prefix(msg_prefix);

  system_messages::distributed = true;
  TraceData::distributed       = true;

  switch(verbose)
  {
    case SILENT:
      system_messages::verbose = false;
      break;
    case VERBOSE:
      system_messages::print_timers            = true;
      system_messages::verbose                 = true;
      system_messages::messages_from_all_ranks = false;
      break;
    case VVERBOSE:
      system_messages::print_timers            = true;
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

  if (!ConfigurationManager->Initialize(ClusteringDefinitionXML, "", false, false))
  {
    SetError(true);
    SetErrorMessage(ConfigurationManager->GetLastError());
    return false;
  }

  ConfigurationManager->SetDistributed(true);

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

  if (!ConfigurationManager->Initialize(ClusteringDefinitionXML, "", false, false))
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

  /*
  if (!ConfigurationManager->Initialize(ClusteringDefinitionXML, "", false, false))
  {
    SetError(true);
    SetErrorMessage(ConfigurationManager->GetLastError());
    return false;
  }
  */

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
 * \param ConsecutiveEvts  True if events any event to deal defines an entry/
 *                         exit of a region (UNUSED IN THIS VERSION)
 *
 * \return True if the data extraction was performed correctly, false otherwise
 */
bool libDistributedClusteringImplementation::ExtractData(string            InputFileName,
                                                         set<int>&         TasksToRead,
                                                         set<event_type_t> EventsToDealWith,
                                                         bool              ConsecutiveEvts)
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
    this->ConsecutiveEvts  = ConsecutiveEvts;
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
    if (!Extractor->SetEventsToDealWith(EventsToDealWith, ConsecutiveEvts))
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
                                                         set<event_type_t> EventsToDealWith,
                                                         bool              ConsecutiveEvts)
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
    this->ConsecutiveEvts  = ConsecutiveEvts;
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
    if (!Extractor->SetEventsToDealWith(EventsToDealWith, ConsecutiveEvts))
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


  for (size_t i = 0; i < ClusterModels.size(); i++)
  {
    ConvexHullModel* CurrentConvexHull = ClusterModels[i]->Model();
    GlobalModelHulls.push_back(*CurrentConvexHull);
  }

  ConvexHullClassifier ClassifierCore(GlobalModelHulls, Epsilon, MinPoints);

  /* DEBUG
  ostringstream Messages;
  Messages << "Hulls used to classify points = " << ClusterModels.size() << endl;
  system_messages::information(Messages.str().c_str());
  */

  if (UsingExternalData)
  {
    ClassifierCore.Classify(ExternalData.begin(),
                            ExternalData.end(),
                            ExternalData.size(),
                            ClassificationPartition);
  }
  else
  {
#ifdef HAVE_SQLITE3

    // ostringstream Messages;
    // Messages << "Points to classify: " << Data->GetCompleteBurstsSize() << endl;
    // system_messages::information(Messages.str());

    ClassifierCore.Classify(Data->GetCompleteBursts_begin(),
                            Data->GetCompleteBursts_end(),
                            Data->GetCompleteBurstsSize(),
                            ClassificationPartition);
#else

    ClassifierCore.Classify(Data->GetClusteringBursts().begin(),
                            Data->GetClusteringBursts().end(),
                            Data->GetClusteringBursts().size(),
                            ClassificationPartition);
#endif
  }

  if (!GenerateStatistics(true)) // true = UseClassificationPartition
  {
    return false;
  }

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
#ifdef HAVE_SQLITE3
    if (!Statistics.ComputeStatistics(Data->GetCompleteBursts_begin(),
                                      Data->GetCompleteBursts_end(),
                                      Data->GetCompleteBurstsSize(),
                                      ClassificationPartition.GetAssignmentVector()))
    {
      SetErrorMessage(Statistics.GetLastError());
      return false;
    }
#else


    /* The statistics only take into account the ClusteringBursts
    if (!Statistics.ComputeStatistics(Data->GetCompleteBursts(),
                                      ClassificationPartition.GetAssignmentVector()))
    {
      SetErrorMessage(Statistics.GetLastError());
      return false;
    }
    */
    if (!Statistics.ComputeStatistics(Data->GetClusteringBursts(),
                                      ClassificationPartition.GetAssignmentVector()))
    {
      SetErrorMessage(Statistics.GetLastError());
      return false;
    }
#endif

    // Statistics.TranslatedIDs(ClassificationPartition.GetAssignmentVector());
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
    // *Statistics.TranslatedIDs(LastPartition.GetAssignmentVector());
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
  ConvexHullClassifier ClassifierCore(GlobalModelHulls, Epsilon, MinPoints);

  /* Only the "root worker" may run the trace reconstruction! */
  if (!Root)
  {
    return false;
  }

  /* First, classify the 'CompleteBursts' collection */
  ClassifierCore.Classify(Data->GetCompleteBursts().begin(),
                          Data->GetCompleteBursts().end(),
                          Data->GetCompleteBursts().size(),
                          TraceReconstructionPartition);

  if (TraceReconstructionPartition.NumberOfClusters() == 0)
  {
    SetError(true);
    SetErrorMessage("number of clusters equals to 0, input trace reconstruction not available");
  }

  if (TraceReconstructionPartition.GetAssignmentVector().size() == 0)
  {
    SetErrorMessage("no cluster analysis data available to reconstruct the input trace");
    return false;
  }

  switch(InputFileType)
  {
    case ParaverTrace:
    {
      if (PRVEventsParsing)
      {
        ClusteredEventsPRVGenerator* TraceReconstructor =
          new ClusteredEventsPRVGenerator(InputFileName, OutputTraceName);

        TraceReconstructor->SetEventsToDealWith (EventsToDealWith, ConsecutiveEvts);

#ifdef HAVE_SQLITE3
        if (!TraceReconstructor->Run(Data->GetAllBursts_begin(),
                                     Data->GetAllBursts_end(),
                                     ClassificationPartition.GetAssignmentVector(),
                                     ClassificationPartition.GetIDs()))
        {
          SetErrorMessage(TraceReconstructor->GetLastError());
          return false;
        }
#else
        if (!TraceReconstructor->Run(Data->GetAllBursts().begin(),
                                     Data->GetAllBursts().end(),
                                     TraceReconstructionPartition.GetAssignmentVector(),
                                     TraceReconstructionPartition.GetIDs()))
        {
          SetErrorMessage(TraceReconstructor->GetLastError());
          return false;
        }
#endif

      }
      else
      {
        ClusteredStatesPRVGenerator* TraceReconstructor =
          new ClusteredStatesPRVGenerator(InputFileName, OutputTraceName);

#ifdef HAVE_SQLITE3
        if (!TraceReconstructor->Run(Data->GetAllBursts_begin(),
                                     Data->GetAllBursts_end(),
                                     ClassificationPartition.GetAssignmentVector(),
                                     ClassificationPartition.GetIDs()))
        {
          SetErrorMessage(TraceReconstructor->GetLastError());
          return false;
        }
#else
        if (!TraceReconstructor->Run(Data->GetAllBursts().begin(),
                                     Data->GetAllBursts().end(),
                                     TraceReconstructionPartition.GetAssignmentVector(),
                                     TraceReconstructionPartition.GetIDs()))
        {
          SetErrorMessage(TraceReconstructor->GetLastError());
          return false;
        }
#endif

      }
      break;
    }
    case DimemasTrace:
    { /* TBI -> Dimemas Cluster blocks not implemented */
      ClusteredTRFGenerator* TraceReconstructor =
        new ClusteredTRFGenerator (InputFileName, OutputTraceName);

#ifdef HAVE_SQLITE3
      if (!TraceReconstructor->Run(Data->GetAllBursts_begin(),
                                   Data->GetAllBursts_end(),
                                   ClassificationPartition.GetAssignmentVector(),
                                   ClassificationPartition.GetIDs()))
      {
        SetErrorMessage(TraceReconstructor->GetLastError());
        return false;
      }
#else
      if (!TraceReconstructor->Run(Data->GetAllBursts().begin(),
                                   Data->GetAllBursts().end(),
                                   TraceReconstructionPartition.GetAssignmentVector(),
                                   TraceReconstructionPartition.GetIDs()))
      {
        SetErrorMessage(TraceReconstructor->GetLastError());
        return false;
      }
#endif
      break;
    }
    case SematicGuided:
    {
      SemanticGuidedPRVGenerator* TraceReconstructor =
        new SemanticGuidedPRVGenerator(InputFileName, OutputTraceName);

      if (!TraceReconstructor->Run(Data->GetAllBursts().begin(),
                                   Data->GetAllBursts().end(),
                                   TraceReconstructionPartition.GetAssignmentVector(),
                                   TraceReconstructionPartition.GetIDs()))
      {
        SetErrorMessage(TraceReconstructor->GetLastError());
        return false;
      }
      break;
    }
    default:
    {
      SetErrorMessage("unable to reconstruct an input file which is not a trace");
      return false;
    }
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
                                                             const vector<long long>    &Durations,
                                                             vector<HullModel*>         &ClusterModels)
{
  /* 'ClusteringCore' has been initialized in 'InitTraceClustering' method */
  ExternalData          = Points;
  ExternalDataDurations = Durations;

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
bool libDistributedClusteringImplementation::GetNoisePoints(vector<const Point*>& NoisePoints, vector<long long> &NoiseDurations)
{
  vector<const Point*>  DataPoints;
  vector<long long>     DataDurations;
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

  GetDataPoints(DataPoints, DataDurations);

  NoisePoints.clear();

  for (size_t i = 0; i < IDs.size(); i++)
  {
//std::cout << "GetNoisePoints IDs[" << i << "] = " << IDs[i] << " (NOISE_CLUSTERID=" << NOISE_CLUSTERID << ")" << endl;
    if (IDs[i] == NOISE_CLUSTERID)
    {
      NoisePoints.push_back(DataPoints[i]);
      NoiseDurations.push_back(DataDurations[i]);
    }
  }

  return true;
}

/* Methods to be used in the ON-LINE implementation of the algorithm. In
     * this case, the data comes from buffers present in the data extraction
     * library. For this reason, we have to expose the data manipulation
     * routines */

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
bool libDistributedClusteringImplementation::NewBurst(task_id_t                         TaskId,
                                                      thread_id_t                       ThreadId,
                                                      line_t                            Line,
                                                      timestamp_t                       BeginTime,
                                                      timestamp_t                       EndTime,
                                                      duration_t                        BurstDuration,
                                                      map<event_type_t, event_value_t>& EventsData)
{
  /* Get the container */
  Data = TraceData::GetInstance();

  /* Set the subset to read */
  Data->SetMaster(Root);

  if (!Data->NewBurst(TaskId,
                      ThreadId,
                      Line,
                      BeginTime,
                      EndTime,
                      BurstDuration,
                      EventsData))
  {
    SetError(true);
    SetErrorMessage("error storing burst data",
                    Data->GetLastError());
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
void libDistributedClusteringImplementation::GetParameterRanges(vector<double>& MinValues,
                                                                vector<double>& MaxValues)
{
  MinValues = Data->GetMinValues();
  MaxValues = Data->GetMaxValues();

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
void libDistributedClusteringImplementation::NormalizeData(vector<double>& MinValues,
                                                           vector<double>& MaxValues)
{
  Data->Normalize(MinValues, MaxValues);

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
bool libDistributedClusteringImplementation::GetClusterStatistics(vector<ClusterStatistics*>& ExternalStatistics)
{
  ParametersManager* Parameters = ParametersManager::GetInstance();;

  vector<string> ExtrapolationParametersNames = Parameters->GetExtrapolationParametersNames();

  set<cluster_id_t> IDs = ClassificationPartition.GetIDs();

  if (IDs.size() == 0)
  {
    SetErrorMessage("unable to retrieve statistics (empty classification partition)");
    SetError(true);
    return false;
  }

  ExternalStatistics.clear();

  for (set<cluster_id_t>::iterator IDsIt  = IDs.begin();
                                   IDsIt != IDs.end();
                                   ++IDsIt)
  {
    StatisticsContainer CurrentClusterStatistics;
    ClusterStatistics*  NewClusterStatistics;

    if (!Statistics.GetClusterStatistics(*IDsIt, CurrentClusterStatistics))
    {
      ostringstream Message;
      Message << "unable to retrieve statistics from current partition (ID : ";
      Message << *IDsIt << ")";

      SetErrorMessage(Message.str());
      SetError(true);
      return false;
    }

    NewClusterStatistics = new ClusterStatistics(*IDsIt,
                                                 CurrentClusterStatistics.GetIndividuals(),
                                                 CurrentClusterStatistics.GetTotalDuration(),
                                                 CurrentClusterStatistics.GetDurationMean(),
                                                 CurrentClusterStatistics.GetDurationStdDev_2());

    ExternalStatistics.push_back(NewClusterStatistics);

    for (vector<string>::size_type i = 0;
         i < ExtrapolationParametersNames.size();
         i++)
    {
      NewClusterStatistics->AddMetric(ExtrapolationParametersNames[i],
                                      CurrentClusterStatistics.GetExtrapolationMetricAggregate(i),
                                      CurrentClusterStatistics.GetExtrapolationMetricIndividuals(i),
                                      CurrentClusterStatistics.GetExtrapolationMetricMean(i),
                                      CurrentClusterStatistics.GetExtrapolationMetricStdDev_2(i));
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
/*
                                                                      vector<task_id_t>&    TaskIDs,
                                                                      vector<thread_id_t>&  ThreadIDs,
*/
                                                                      vector<timestamp_t>&  BeginTimes,
                                                                      vector<timestamp_t>&  EndTimes,
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
/*
  TaskIDs.clear();
  ThreadIDs.clear();
*/
  BeginTimes.clear();
  EndTimes.clear();
  ClusterIDs.clear();

  Dimensions = Data->GetClusteringDimensionsCount();

  for (size_t i = 0; i < Bursts.size(); i++)
  {
    Point* NewPoint = new Point(Bursts[i]->GetRawDimensions());

    Points.push_back(NewPoint);
/*
    TaskIDs.push_back(Bursts[i]->GetTaskId());
    ThreadIDs.push_back(Bursts[i]->GetThreadId());
*/
    BeginTimes.push_back(Bursts[i]->GetBeginTime());
    EndTimes.push_back(Bursts[i]->GetEndTime());
    ClusterIDs.push_back(IDs[i]);
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
bool libDistributedClusteringImplementation::GetClusterIDs(vector<cluster_id_t> &ClusterIDs)
{
  vector<const Point*>& ClusteringPoints = ( UsingExternalData ? ExternalData : Data->GetClusteringPoints() );
  vector<cluster_id_t>& IDs    = ClassificationPartition.GetAssignmentVector();

  if (IDs.size() == 0)
  {
    SetErrorMessage("data needs to be classfied before being retrieved");
  }

  if (ClusteringPoints.size() != IDs.size())
  {
    ostringstream ErrorMessage;

    ErrorMessage << "different number of points (" << ClusteringPoints.size() << ") ";
    ErrorMessage << "than cluster IDs (" << IDs.size() << ")";

    SetErrorMessage(ErrorMessage.str());
    return false;
  }

  ClusterIDs.clear();

  for (size_t i = 0; i < ClusteringPoints.size(); i++)
  {
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
 * \param UseLocalPartition        Boolean to indicate if the data to be printed
 *                              is the global partition or just the local
 *                              analysis
 *
 * \return True if the plots scripts and data files were written correctly,
 *         false otherwise
 */
bool libDistributedClusteringImplementation::PrintPlotScripts(string DataFileName,
                                                              string ScriptsFileNamePrefix,
                                                              bool   UseLocalPartition)
{
  PlottingManager *Plots;
  string Prefix;
  ostringstream PlotTitle;

  Plots = PlottingManager::GetInstance();

  Partition& UsedPartition = (UseLocalPartition ? LastPartition : ClassificationPartition);

  if (ScriptsFileNamePrefix.compare("") == 0)
  {
    FileNameManipulator Manipulator(DataFileName, "csv");
    Prefix = Manipulator.GetChoppedFileName();
  }
  else
  {
    Prefix = ScriptsFileNamePrefix;
  }

  if (!FlushData(DataFileName, UseLocalPartition))
  {
    SetError(true);
    return false;
  }

  PlotTitle << ClusteringCore->GetClusteringAlgorithmName() << "\\n";

  if (UseLocalPartition)
  {
    PlotTitle << "LOCAL CLUSTERING WORKER " << system_messages::my_rank;
  }
  else
  {
    PlotTitle << "GLOBAL CLUSTERING WORKER " << system_messages::my_rank;
  }

  if (!Plots->PrintPlots(DataFileName,
                         Prefix,
                         PlotTitle.str(),
                         UsedPartition.GetIDs()))
  {
    SetError(true);
    SetErrorMessage(Plots->GetLastError());
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
bool libDistributedClusteringImplementation::PrintGlobalPlotScripts(string       DataFileName,
                                                                    string       ScriptsFileNamePrefix,
                                                                    unsigned int ClustersCount,
                                                                    bool         PrintingModels)
{
  PlottingManager *Plots;
  ostringstream PlotTitle;

  Plots = PlottingManager::GetInstance();

  set<cluster_id_t> IDsUsed;

  PlotTitle << ClusteringCore->GetClusteringAlgorithmName() << "\\n";
  PlotTitle << "GLOBAL CLUSTERING";

  for (size_t i = 0; i <= ClustersCount; i++)
  {
    IDsUsed.insert(i);
  }

  if (!Plots->PrintPlots(DataFileName,
                         ScriptsFileNamePrefix,
                         PlotTitle.str(),
                         IDsUsed,
                         PrintingModels))
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
  vector<long long>     BurstsDurations;

  vector<vector<const Point*> > PointsPerCluster (LastPartition.NumberOfClusters ());
  vector<long long>             DurationPerCluster (LastPartition.NumberOfClusters (), 0);

  if (!UsingExternalData)
  {
    vector<CPUBurst*>& ClusteringBursts = Data->GetClusteringBursts();
    for (size_t i = 0; i < ClusteringBursts.size(); i++)
    {
      BurstsDurations.push_back(ClusteringBursts[i]->GetDuration());
    }
  }

  vector<long long>& PointsDurations = ( UsingExternalData ? ExternalDataDurations : BurstsDurations );

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
  // unsigned int ClusterPointsSize = ClusteringBursts.size();

  if (AssignmentVector.size() != ClusterPointsSize)
  {
    SetError(true);
    SetErrorMessage("partition elements and cluster points differ");
    return false;
  }

  for (size_t i = 0; i < ClusteringPoints.size(); i++)
  {
    PointsPerCluster[AssignmentVector[i]].push_back(ClusteringPoints[i]);
    DurationPerCluster[AssignmentVector[i]] += PointsDurations[i];
  }

  for (size_t i = NOISE_CLUSTERID+1; i < PointsPerCluster.size(); i++)
  {
    HullModel *NewHull = new HullModel(new ConvexHullModel(PointsPerCluster[i], DurationPerCluster[i]));

    Models.push_back(NewHull);

    /* DEBUG
    if (NewHull->Size() < 3)
    {
      // MAY HAPPEN WHEN CLUSTERING POINTS ARE 1-DIMENSIONAL
      std::cout << "[DEBUG] Hull points=" << NewHull->Size() << " Cluster points=" << PointsPerCluster[i].size() << std::endl;
      std::cout << "[DEBUG] Dumping cluster..." << endl;
      for (int k=0; k<PointsPerCluster[i].size(); k++)
      {
        Point *p = (Point*)PointsPerCluster[i][k];
        p->PrintPoint();
      }
      std::cout << "[DEBUG] Dumping hull..." << endl;
      NewHull->Flush();
    } */

  }

  return true;
}

/**
 * Returns a vector reference where the data is store
 *
 * \return A reference to the vector that contains the actual data;
 */
void libDistributedClusteringImplementation::GetDataPoints(vector<const Point *>&Points, vector<long long>&Durations)
{
  if (UsingExternalData)
  {
    Points    = ExternalData;
    Durations = ExternalDataDurations;
  }
  else
  {
    vector<CPUBurst *>& Bursts = Data->GetClusteringBursts();

    Points = Data->GetClusteringPoints();
    for (int i=0; i<Bursts.size(); i++)
    {
      Durations.push_back( Bursts[i]->GetDuration() );
    }
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
    if (!Data->FlushPoints(OutputStream,
                           LastPartition.GetAssignmentVector(),
                           PrintClusteringBursts))
    {
      SetError(true);
      SetErrorMessage("error flushing local points", Data->GetLastError());
      return false;
    }
  }
  else
  {
    DataPrintSet WhatToPrint;

    /*
    if (Root)
    {
      WhatToPrint = PrintCompleteBursts;
    }
    else
    {
      WhatToPrint = PrintClusteringBursts;
    }
    */

    if (!Data->FlushPoints(OutputStream,
                           ClassificationPartition.GetAssignmentVector(),
                           PrintClusteringBursts))
    {
      SetError(true);
      SetErrorMessage("error flushing global points", Data->GetLastError());
      return false;
    }
  }

  OutputStream.close();

  return true;
}
