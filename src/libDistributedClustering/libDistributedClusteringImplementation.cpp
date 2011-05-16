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

#include "libDistributedClustering.hpp"
#include "libDistributedClusteringImplementation.hpp"

#include <ClusteringConfiguration.hpp>
#include "DataExtractor.hpp"
#include "DataExtractorFactory.hpp"
#include "ClusteringStatistics.hpp"
#include "ClusteredTraceGenerator.hpp"
#include "ClusteredPRVGenerator.hpp"
#include "ClusteredTRFGenerator.hpp"
#include "PlottingManager.hpp"

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

bool libDistributedClusteringImplementation::InitClustering(string ClusteringDefinitionXML,
                                                            double Epsilon,
                                                            INT32  MinPoints,
                                                            bool   Root,
                                                            INT32  MyRank,
                                                            INT32  TotalRanks)
{
  ClusteringConfiguration *ConfigurationManager;
  ParametersManager       *Parameters;
  string                   ClusteringAlgorithmName;
  map<string, string>      ClusteringAlgorithmParameters;
  PlottingManager         *Plots;
  ostringstream            Converter;
  string                   EpsilonStr, MinPointsStr;
  
  ConfigurationManager = ClusteringConfiguration::GetInstance();

  if (!ConfigurationManager->Initialize(ClusteringDefinitionXML))
  {
    SetError(true);
    SetErrorMessage(ConfigurationManager->GetLastError());
    return false;
  }

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
  ClusteringAlgorithmName       = DBSCAN::NAME;

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

  Plots = PlottingManager::GetInstance(false);

  if (Plots->GetError())
  {
    SetError(true);
    SetErrorMessage(Plots->GetLastError());
    return false;
  }

  system_messages::my_rank     = MyRank; 
  
  this->Root = Root;
  
  return true;
}


bool libDistributedClusteringImplementation::ExtractData(string    InputFileName,
                                                         set<int>& TasksToRead)
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
  if (!ExtractorFactory->GetExtractor(InputFileName, Extractor))
  {
    SetError(true);
    SetErrorMessage(ExtractorFactory->GetLastError());
    return false;
  }

  this->InputFileName = InputFileName;
  InputFileType       = Extractor->GetFileType();
  
  if (!Extractor->ExtractData(Data))
  {
    SetError(true);
    SetErrorMessage(Extractor->GetLastError());
    return false;
  }
  
  return true;
}

bool libDistributedClusteringImplementation::ExtractData(string InputFileName)
{
  DataExtractorFactory*    ExtractorFactory;
  DataExtractor*           Extractor;

  /* Get the container */
  Data = TraceData::GetInstance();

  /* Set the subset to read */
  Data->SetMaster(Root);
  
  /* Get input file extractor */
  ExtractorFactory = DataExtractorFactory::GetInstance();
  if (!ExtractorFactory->GetExtractor(InputFileName, Extractor))
  {
    SetError(true);
    SetErrorMessage(ExtractorFactory->GetLastError());
    return false;
  }

  this->InputFileName = InputFileName;
  InputFileType       = Extractor->GetFileType();
  
  if (!Extractor->ExtractData(Data))
  {
    SetError(true);
    SetErrorMessage(Extractor->GetLastError());
    return false;
  }
  
  return true;
}

bool libDistributedClusteringImplementation::ClusterAnalysis(vector<ConvexHullModel>& ClusterModels)
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

bool libDistributedClusteringImplementation::PrintModels(vector<ConvexHullModel>& ClusterModels,
                                                         string                   ModelsFileName,
                                                         string                   ScriptsFileNamePrefix)
{
  string Prefix;

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
    SetErrorMessage("unable to open models file name", strerror(errno));
    return false;
  }

  for (size_t i = 0; i < ClusterModels.size(); i++)
  {
/* DEBUG
    Messages << "**** Printing Hull " << i << " to file " << ModelsFileName << endl;
    system_messages::information(Messages.str().c_str());
    Messages.str(""); */
    ClusterModels[i].Flush(OutputStream, MIN_CLUSTERID+i+PARAVER_OFFSET); // +1 because there is no noise!
  }

  OutputStream.close();

  /*
  if (!Plots->PrintPlots(DataFileName, Prefix, LastPartition.NumberOfClusters(), LastPartition.HasNoise()))
  {
    SetError(true);
    SetErrorMessage(Plots->GetLastError());
    return false;
  }
  */
  
  return true;
}

bool libDistributedClusteringImplementation::ClassifyData(vector<ConvexHullModel>& ClusterModels)
{
  ostringstream Messages;

  vector<const Point*>& CompletePoints = (vector<const Point*>&) Data->GetCompleteBursts();
  ConvexHullClassifier ClassifierCore(ClusterModels, Epsilon);

  /* DEBUG
  Messages << "Hulls used to classify points = " << ClusterModels.size() << endl;
  system_messages::information(Messages.str().c_str());
  */

  ClassifierCore.Classify(CompletePoints, ClassificationPartition);

  GenerateStatistics(true); // true = UseClassificationPartition
  
  return true;
}


bool libDistributedClusteringImplementation::GenerateStatistics (bool UseClassificationPartition)
{
  ClusteringConfiguration* ConfigurationManager;
  ClusteringStatistics     Statistics;
  ParametersManager*       Parameters;
 

  ConfigurationManager = ClusteringConfiguration::GetInstance();
  if (!ConfigurationManager->IsInitialized())
  { /* Should never happen! */
    SetErrorMessage("configuration not initialized");
    return false;
  }

  /* Statistics */

  Parameters = ParametersManager::GetInstance();
  Statistics.InitStatistics(LastPartition.NumberOfClusters(),
                            true, // We alway use DBSCAN 
                            Parameters->GetClusteringParametersNames(),
                            Parameters->GetClusteringParametersPrecision(),
                            Parameters->GetExtrapolationParametersNames(),
                            Parameters->GetExtrapolationParametersPrecision());
  
  if (UseClassificationPartition)
  {
    
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
      TraceReconstructor = new ClusteredPRVGenerator(InputFileName, OutputTraceName);
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
                               IDs,
                               ClassificationPartition.NumberOfClusters()))
  {
    SetErrorMessage(TraceReconstructor->GetLastError());
    return false;
  }
  
  return true;
}

bool libDistributedClusteringImplementation::PrintPlotScripts(string DataFileName,
                                                              string ScriptsFileNamePrefix,
                                                              bool   LocalPartition)
{
  PlottingManager *Plots;
  string Prefix;

  Plots = PlottingManager::GetInstance();
  
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
                         LastPartition.NumberOfClusters(),
                         LastPartition.HasNoise()))
  {
    SetError(true);
    SetErrorMessage(Plots->GetLastError());
    return false;
  }
  
  return true;

}

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
      SetErrorMessage(Data->GetLastError());
      return false;
    }
  }
  else
  {
    if (!Data->FlushPoints(OutputStream, ClassificationPartition.GetAssignmentVector(), false)) // false = Not All Data!!
    {
      SetError(true);
      SetErrorMessage(Data->GetLastError());
      return false;
    }
  }

  OutputStream.close();
  
  return true;
}

bool libDistributedClusteringImplementation::GenerateClusterModels(vector<ConvexHullModel>& Models)
{
  vector<cluster_id_t>& AssignmentVector = LastPartition.GetAssignmentVector();
  vector<const Point*>& ClusteringPoints = Data->GetClusteringPoints();
  vector<vector<const Point*> > PointsPerCluster (LastPartition.NumberOfClusters ());

  /* DEBUG
  ostringstream Messages;
  Messages << "Creating Hulls. Number of clusters = " << LastPartition.NumberOfClusters() << endl;
  system_messages::information(Messages.str().c_str()); 
  */

  Models.clear();

  /* DEBUG
  Messages.str("");
  Messages << " Models vector size = " << Models.size() << endl;
  system_messages::information(Messages.str().c_str());
  */

  if (AssignmentVector.size() != Data->GetClusteringBurstsSize())
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
    Models.push_back(ConvexHullModel(PointsPerCluster[i]));
  }

  return true;
}
