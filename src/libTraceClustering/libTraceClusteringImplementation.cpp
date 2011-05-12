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

  $URL:: https://svn.bsc.es/repos/ptools/prv2dim/                          $:

  $Rev:: 478                        $:  Revision of last commit
  $Author:: jgonzale                $:  Author of last commit
  $Date:: 2010-10-28 13:58:59 +0200 $:  Date of last commit

\* -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=- */

#include "libTraceClusteringImplementation.hpp"
#include "libTraceClustering.hpp"

#include <SystemMessages.hpp>
using cepba_tools::system_messages;
#include <FileNameManipulator.hpp>
using cepba_tools::FileNameManipulator;

#include <ClusteringConfiguration.hpp>
#include "DataExtractor.hpp"
#include "DataExtractorFactory.hpp"
#include "ClusteringStatistics.hpp"
#include "ClusteredTraceGenerator.hpp"
#include "ClusteredStatesPRVGenerator.hpp"
#include "ClusteredEventsPRVGenerator.hpp"
#include "ClusteredTRFGenerator.hpp"
#include "PlottingManager.hpp"

#ifdef HAVE_SEQAN
#include "SequenceScore.hpp"
#include "ClusteringRefinement.hpp"
#endif

#include <cerrno>
#include <cstring>

#include <fstream>
using std::ofstream;
using std::ios_base;

#include <sstream>
using std::ostringstream;

#ifdef HAVE_MPI
#include <mpi.h>

#define CLUSTER_SIZE_EXCHG_TAG  1
#define CLUSTER_LINES_EXCHG_TAG 2

#endif

/**
 * Empty constructor
 */
libTraceClusteringImplementation::libTraceClusteringImplementation(bool verbose)
{
  system_messages::verbose = verbose;
  ClusteringExecuted       = false;
  PRVEventsParsing         = false;
}

/**
 * Initialize the clustering library. Loads the XML and initializes the configuration
 * object, core of the application
 *
 * \param ClusteringDefinitionXML Name of the XML file where the XML is defined
 * \param ApplyCPIStack Boolean indicating if PPC970MP CPI stack counters should be extrapolated
 *
 * \return True if initialization has been done properly. False otherwise
 */
bool libTraceClusteringImplementation::InitTraceClustering(string ClusteringDefinitionXML,
                                                           unsigned char UseFlags)
{
  ClusteringConfiguration *ConfigurationManager;
  ParametersManager       *Parameters;
  PlottingManager         *Plots;

  ConfigurationManager = ClusteringConfiguration::GetInstance();

  if (!ConfigurationManager->Initialize(ClusteringDefinitionXML))
  {
    SetError(true);
    SetErrorMessage(ConfigurationManager->GetLastError());
    return false;
  }
  /* Set if MPI should be used */
  ConfigurationManager->SetDistributed(USE_MPI(UseFlags));

#if HAVE_MPI
  /* Set the current rank and total ranks using MPI */
  
#endif

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

  if (USE_CLUSTERING_REFINEMENT(UseFlags))
  {
    ClusteringRefinementExecution = true;
  }
  
  if (USE_CLUSTERING(UseFlags) || USE_PARAMETER_APPROXIMATION(UseFlags))
  { 
    string              ClusteringAlgorithmName;
    map<string, string> ClusteringAlgorithmParameters;
    
    /* Check if clustering defined in the XML is correct */
    if (ConfigurationManager->GetClusteringAlgorithmError())
    {
      SetErrorMessage(ConfigurationManager->GetClusteringAlgorithmErrorMessage());
      return false;
    }
    
    /* Check if clustering library could be correctly initialized */
    ClusteringCore = new libClustering();

    ClusteringAlgorithmName       = ConfigurationManager->GetClusteringAlgorithmName();
    ClusteringAlgorithmParameters = ConfigurationManager->GetClusteringAlgorithmParameters();

    if (!ClusteringCore->InitClustering (ClusteringAlgorithmName,
                                         ClusteringAlgorithmParameters))
    {
      SetError(true);
      SetErrorMessage(ClusteringCore->GetErrorMessage());
      return false;
    }

    if (USE_MPI(UseFlags))
    {
      if (!ClusteringCore->UsingADistributedAlgorithm())
      {
        string Message;
        Message = "\"" + ConfigurationManager->GetClusteringAlgorithmName() + "\"";
        Message += " can't be used in MPI environment";
        SetError(true);
        SetErrorMessage (Message);
        return false;
      }
    }
  }

  if (USE_PLOTS(UseFlags))
  { /* Check if GNUplots defined in the XML are correct */
    bool DataExtraction;
    
    if (!USE_CLUSTERING(UseFlags) && !USE_CLUSTERING_REFINEMENT(UseFlags))
    { /* Data Extraction mode */
      DataExtraction = true;
    }
    else
    { /* Clustering mode */
      DataExtraction = false;
    }

    Plots = PlottingManager::GetInstance(DataExtraction);

    if (Plots->GetError())
    {
      SetError(true);
      SetErrorMessage(Plots->GetLastError());
      return false;
    }
  }

  if (USE_MPI(UseFlags))
  {
    system_messages::distributed = true;
  }

  this->UseFlags = UseFlags;
  
  return true;
}

/**
 * Loads data from an input file. It could be a Paraver trace, Dimemas trace or
 * a previously generated CSV file. This method doesn't generate an output file
 * and should be used to later run an analysis.
 *
 * \param InputFileName The name of the input file where data is located
 * \param EventsToDealWith Set of parameters in case we want to do an event parsing
 *                         of a Paraver trace
 *
 * \return True if the data extraction work properly. False otherwise
 */
bool
libTraceClusteringImplementation::ExtractData(string            InputFileName,
                                              set<event_type_t> EventsToDealWith)
{
  DataExtractorFactory*    ExtractorFactory;
  DataExtractor*           Extractor;
  
  /* Get the container */
  Data = TraceData::GetInstance();

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
                                      PRVEventsParsing,
                                      USE_MPI(UseFlags)))
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
  
  /*
  DataExtractionManager* ExtractionManager;

  ExtractionManager = DataExtractionManager::GetInstance(InputFileName);


  if (ExtractionManager->GetError())
  {
    SetError(true);
    SetErrorMessage(ExtractionManager->GetLastError());
    return false;
  }
  else if (ExtractionManager->GetWarning())
  {
    SetWarning(true);
    SetWarningMessage(ExtractionManager->GetLastWarning());
  }

  if (!ExtractionManager->ExtractData())
  {
    SetError(true);
    SetErrorMessage(ExtractionManager->GetLastError());
    return false;
  }
  */
  
  return true;
}

/**
 * Generates a CSV file with the data present on the current data set load in
 * memory
 *
 * \param OutputFileName Name of the output file where data will be written
 *
 * \result True if data got written correctly, false otherwise
 */
bool
libTraceClusteringImplementation::FlushData(string OutputFileName)
{
  bool     AllData;
  ofstream OutputStream (OutputFileName.c_str(), ios_base::trunc);

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

  if (USE_CLUSTERING(UseFlags))
  {
    AllData = false;
  }
  else
  {
    AllData = true;
  }
  
  if (!Data->FlushPoints(OutputStream, LastPartition.GetAssignmentVector(), AllData))
  {
    SetError(true);
    SetErrorMessage(Data->GetLastError());
    return false;
  }

  OutputStream.close();
  
  return true;
}

/**
 * Performs a single cluster analysis, stores the results in LastPartition attribute
 *
 * \result True if the analysis finished correctly, false otherwise
 */
bool libTraceClusteringImplementation::ClusterAnalysis(void)
{
  ClusteringConfiguration* ConfigurationManager;
  ParametersManager*       Parameters;

  if (Data == NULL)
  {
    SetErrorMessage("data not initialized");
    return false;
  }

  if (ClusteringRefinementExecution)
  {
    SetError(true);
    SetErrorMessage("single cluster analysis not available when performing a cluster refinement");
    return false;
  }

  ConfigurationManager = ClusteringConfiguration::GetInstance();
  
  if (!ConfigurationManager->IsInitialized())
  { /* Should never happen! */
    SetErrorMessage("configuration not initialized");
    return false;
  }

  vector<const Point*>& ClusteringPoints = Data->GetClusteringPoints();

  /* DEBUG
  cout << "Clustering Points size = " << ClusteringPoints.size() << endl;
  */

  /* 'ClusteringCore' has been initialized in 'InitTraceClustering' method */
  if (!ClusteringCore->ExecuteClustering(ClusteringPoints, LastPartition))
  {
    SetErrorMessage(ClusteringCore->GetErrorMessage());
    return false;
  }

  ClusteringExecuted = true;
  
  if (USE_MPI(UseFlags))
  {
    if (!GatherMPIPartition())
    {
      // SetErrorMessage("Error gathering MPI information");
      return false;
    }
  }
  
  /* Statistics */
  Parameters = ParametersManager::GetInstance();
  
  Statistics.InitStatistics(LastPartition.GetIDs(),
                            ClusteringCore->HasNoise(),
                            Parameters->GetClusteringParametersNames(),
                            Parameters->GetClusteringParametersPrecision(),
                            Parameters->GetExtrapolationParametersNames(),
                            Parameters->GetExtrapolationParametersPrecision());
  
  if (!Statistics.ComputeStatistics(Data->GetCompleteBursts(),
                                    LastPartition.GetAssignmentVector()))
  {
    SetErrorMessage(Statistics.GetLastError());
    return false;
  }
  
  Statistics.TranslatedIDs(LastPartition.GetAssignmentVector());
  
  return true;
}

/**
 * Performs a DBSCAN cluster analysis with auto refinement based on sequence
 * score. The exploration range is guessed automatically
 *
 * \param OutputFileNamePrefix Prefix of the output files for each step data and plots
 *
 * \result True if the analysis finished correctly, false otherwise
 */
bool libTraceClusteringImplementation::ClusterRefinementAnalysis(string OutputFileNamePrefix)
{
  return true;
}

/**
 * Performs a DBSCAN cluster analysis with auto refinement based on sequence
 * score. The exploration range is provided by the user
 *
 * \param MinPoints Fixed MinPoints value used in all runs of DBSCAN
 * \param MaxEps Maximum value of Epsilon
 * \param MinEps Minimum value of Epsilon
 * \param Steps  Number of iterarions of the algorithm
 * \param OutputFileNamePrefix Prefix of the output files for each step data and plots
 *
 * \result True if the analysis finished correctly, false otherwise
 */
bool libTraceClusteringImplementation::ClusterRefinementAnalysis(int    MinPoints,
                                                                 double MaxEps,
                                                                 double MinEps,
                                                                 int    Steps,
                                                                 string OutputFileNamePrefix)
{
#ifdef HAVE_SEQAN

  ParametersManager       *Parameters;
  vector<Partition> PartitionsHierarchy;
  
  if (Data == NULL)
  {
    SetErrorMessage("data not initialized");
    return false;
  }
  
  ClusteringRefinement RefinementAnalyzer((INT32) MinPoints,
                                          MaxEps,
                                          MinEps,
                                          (size_t) Steps);

  if (!RefinementAnalyzer.Run(Data->GetClusteringBursts(),
                              PartitionsHierarchy,
                              OutputFileNamePrefix))
  {
    SetErrorMessage(RefinementAnalyzer.GetLastError());
    SetError(true);
    return false;
  }

  /* DEBUG 
  for (size_t i = 0; i < PartitionsHierarchy.size(); i++)
  {
    cout << "**** Step " << i+1 << " ****" << endl;
    cout << "-> Number of Clusters = " << PartitionsHierarchy[i].NumberOfClusters() << endl;

    vector<cluster_id_t>& IDs = PartitionsHierarchy[i].GetAssignmentVector();

    cout << "IDs = ";
    
    for (size_t j = 0; j < IDs.size(); j++)
    {
      cout << IDs[j] << " ";
    }
    cout << endl;
  }
  */

  LastPartition = PartitionsHierarchy[PartitionsHierarchy.size()-1];

  /* Statistics */
  Parameters = ParametersManager::GetInstance();
  
  Statistics.InitStatistics(LastPartition.GetIDs(),
                            LastPartition.HasNoise(),
                            Parameters->GetClusteringParametersNames(),
                            Parameters->GetClusteringParametersPrecision(),
                            Parameters->GetExtrapolationParametersNames(),
                            Parameters->GetExtrapolationParametersPrecision());
  
  if (!Statistics.ComputeStatistics(Data->GetCompleteBursts(),
                                    LastPartition.GetAssignmentVector()))
  {
    SetErrorMessage(Statistics.GetLastError());
    return false;
  }
  
  Statistics.TranslatedIDs(LastPartition.GetAssignmentVector());

  /* Generate all intermediate (event) traces */
  if (OutputFileNamePrefix.compare("") != 0)
  {
    for (size_t i = 0; i < PartitionsHierarchy.size()-1; i++)
    {
      ClusteredTraceGenerator* TraceGenerator;
      ostringstream OutputTraceName;

      OutputTraceName << OutputFileNamePrefix << ".STEP" << i+1 << ".prv";

      ClusteringStatistics Stats;

      /* Sort IDs */
      Stats.InitStatistics(PartitionsHierarchy[i].GetIDs(),
                           PartitionsHierarchy[i].HasNoise(),
                           Parameters->GetClusteringParametersNames(),
                           Parameters->GetClusteringParametersPrecision(),
                           Parameters->GetExtrapolationParametersNames(),
                           Parameters->GetExtrapolationParametersPrecision());
  
      if (!Stats.ComputeStatistics(Data->GetCompleteBursts(),
                                   PartitionsHierarchy[i].GetAssignmentVector()))
      {
        SetErrorMessage(Stats.GetLastError());
        return false;
      }
      
      Stats.TranslatedIDs(PartitionsHierarchy[i].GetAssignmentVector());


      if (PRVEventsParsing)
      {
        TraceGenerator = new ClusteredEventsPRVGenerator (InputFileName,
                                                          OutputTraceName.str());
      }
      else
      {
        TraceGenerator = new ClusteredStatesPRVGenerator (InputFileName,
                                                          OutputTraceName.str());
      }

      /* DEBUG
      cout << "All Burst size = " << Data->GetAllBursts().size() << endl;
      cout << "IDs size = " << PartitionsHierarchy[i].GetAssignmentVector().size() << endl; */

      bool verbose_state = system_messages::verbose;
      system_messages::verbose = false;

      if (!TraceGenerator->Run(Data->GetAllBursts(),
                               PartitionsHierarchy[i].GetAssignmentVector(),
                               PartitionsHierarchy[i].NumberOfClusters(),
                               true)) // Minimize information
      {
          system_messages::verbose = verbose_state;
          SetErrorMessage(TraceGenerator->GetLastError());
          return false;
      }
      system_messages::verbose = verbose_state;

      delete TraceGenerator;
    }
  }

  ClusteringExecuted = true;
  
  return true;
  
#else

  SetErrorMessage("Refinement analysis is not available due to the unavailability of SeqAn library");
  SetError(true);
  return false;
  
#endif

}

/**
 * Write clusters information to an output file
 *
 * \param OutputClustersInfoFileName Name of the output file where clusters information will be written
 *
 * \result True if output file is written correctly, false otherwise
 */
bool libTraceClusteringImplementation::FlushClustersInformation(string OutputClustersInfoFileName)
{
  ofstream OutputStream (OutputClustersInfoFileName.c_str(), ios_base::trunc);

  if (Data == NULL)
  {
    SetErrorMessage("data not initialized");
    return false;
  }

  if (!ClusteringExecuted)
  {
    SetErrorMessage("cluster analysis not executed");
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
 * Write clusters sequences to an output file
 *
 * \param OutputFilePrefix Output file prefix for the sequences file and the sequence score file
 * \param FASTASequenceFile True to generate a FASTA aminoacids sequences file
 *
 * \result True if sequences file is written correctly, false otherwise
 */
bool libTraceClusteringImplementation::ComputeSequenceScore(string OutputFilePrefix,
                                                            bool   FASTASequencesFile)
{
#ifdef HAVE_SEQAN
  vector<percentage_t>       PercentageDurations;
  SequenceScore              Scoring;
  vector<SequenceScoreValue> ScoresPerCluster;
  double                     GlobalScore;
  
  
  if (Data == NULL)
  {
    SetErrorMessage("data not initialized");
    return false;
  }

  // This assignement is needed pass the reference
  PercentageDurations = Statistics.GetPercentageDurations();
  
  if(!Scoring.ComputeScore(Data->GetClusteringBursts(),
                           LastPartition.GetAssignmentVector(),
                           PercentageDurations,
                           LastPartition.HasNoise(),
                           ScoresPerCluster,
                           GlobalScore,
                           OutputFilePrefix,
                           FASTASequencesFile))
  {
    SetError(true);
    SetErrorMessage("unable to compute sequences score");
    return false;
  }
  
  return true;
  
#else
  
  SetError(true);
  SetErrorMessage("SeqAn not available, sequence score could not be computed");
  return false;
  
#endif

}

/**
 * Reconstruct the input trace using the information of the clustering analysis
 * 
 * \param OutputTraceName Name of the output trace file
 *
 * \result True if the scripts where printed correctly, false otherwise
 */ 
bool libTraceClusteringImplementation::ReconstructInputTrace(string OutputTraceName)
{
  /* to lowercase a string
  std::transform(str.begin(), str.end(), str.begin(),
  std:tr_fun(std::tolower)); */

  ClusteredTraceGenerator* TraceReconstructor;

  vector<cluster_id_t>& IDs = LastPartition.GetAssignmentVector();

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
                               IDs,
                               LastPartition.NumberOfClusters()))
  {
    SetErrorMessage(TraceReconstructor->GetLastError());
    return false;
  }
  
  return true;
}

/**
 * Print the plot scripts for GNUPlot defined in the XML
 * 
 * \param DataFileName Name of the file containg the data to plot
 * \param ScriptsFileNamePrefix Prefix of the output scripts
 *
 * \result True if the scripts where printed correctly, false otherwise
 */
bool libTraceClusteringImplementation::PrintPlotScripts(string DataFileName,
                                                        string ScriptsFileNamePrefix)
{
  PlottingManager *Plots;
  string Prefix;
  string PlotTitle;

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

  if (USE_CLUSTERING(UseFlags))
  {
    PlotTitle = ClusteringCore->GetClusteringAlgorithmName();
  }
  else if (USE_CLUSTERING_REFINEMENT(UseFlags))
  {
    PlotTitle = "Clustering Refinement of trace \'"+InputFileName+"\'";
  }
  else
  {
    PlotTitle = "Data of trace \'"+InputFileName+"\'";
  }
  
  if (!Plots->PrintPlots(DataFileName,
                         Prefix,
                         PlotTitle,
                         LastPartition.NumberOfClusters(),
                         LastPartition.HasNoise()))
  {
    SetError(true);
    SetErrorMessage(Plots->GetLastError());
    return false;
  }
  
  return true;
}

/**
 * Generates a possible parameter approximation needed by the cluster algorithm
 *
 * \param OutputFileNamePrefix The prefix of the output files that will be generated
 * \param Parameters Map of key and value strings parameters of the approximation
 *
 * \result True if the approximation wero done correctly, false otherwise
 */
bool libTraceClusteringImplementation::ParametersApproximation(string              OutputFileNamePrefix,
                                                               map<string, string> Parameters)
{
  if (Data == NULL)
  {
    SetErrorMessage("data not initialized");
    return false;
  }

  vector<const Point*>& ClusteringPoints = Data->GetClusteringPoints();
  
  /* 'ClusteringCore' has been initialized in 'InitTraceClustering' method */
  if (!ClusteringCore->ParametersApproximation(ClusteringPoints,
                                               Parameters,
                                               OutputFileNamePrefix))
  {
    SetErrorMessage(ClusteringCore->GetErrorMessage());
    return false;
  }
  
  return true;
}

/**
 * When running the MPI version of the library, gathers the results distributed
 * accross the different tasks into the master task (rank 0 in MPI_COMM_WORLD).
 * After executing this operation, we can guarantee that the LastPartition of
 * the master node contains the information needed to reconstruct the whole
 * trace and generate the GNUPlot scripts.
 *
 * \result True if the gather of information finished correctly, false otherwise
 */
bool libTraceClusteringImplementation::GatherMPIPartition(void)
{
#ifdef HAVE_MPI
  INT32 Me;

  MPI_Comm_rank(MPI_COMM_WORLD, &Me);

  if (Me == 0)
  {
    if (!GatherMaster())
    {
      return false;
    }
    /* Reset the clustering points */
    // Data->ResetClusteringPoints();

    return true;
  }
  else
  {
    return GatherSlave();
  }
#else
  return false;
#endif
}

/**
 * Implementation of the master side of the gather operation
 *
 * \result True if the operation finished correctly, false otherwise
 */
bool libTraceClusteringImplementation::GatherMaster(void)
{
#ifdef HAVE_MPI
  /* Define storage structures: a vector with the number of clusters containing 
     a vector of the CPU burst lines */
  vector<vector<long> > LocalLinesPerCluster (LastPartition.NumberOfClusters ());
  vector<CPUBurst*>&    Bursts = Data->GetClusteringBursts();
  vector<cluster_id_t>& IDs    = LastPartition.GetAssignmentVector();
  
  vector<vector<vector<long> > > GlobalLinesPerCluster (LastPartition.NumberOfClusters());
  int TotalTasks;

  MPI_Comm_size(MPI_COMM_WORLD, &TotalTasks);

  /* Initialize the master information */
  for (size_t i = 0; i < Bursts.size(); i++)
  {
    LocalLinesPerCluster[IDs[i]].push_back(Bursts[i]->GetLine());
  }
  
  for (size_t Cluster = 0; Cluster < LastPartition.NumberOfClusters(); Cluster++)
  {
    GlobalLinesPerCluster[Cluster].push_back(LocalLinesPerCluster[Cluster]);
  }

  /* Receive the lines per cluster from each slave task */
  for (size_t Cluster = 0; Cluster < LastPartition.NumberOfClusters(); Cluster++)
  {
    for (size_t Task = 1; Task < TotalTasks; Task++)
    {
      long  CurrentClusterSize;
      long* CurrentClusterLines;
      ostringstream Messages;

      /* Size of the cluster for current Task */
      MPI_Recv(&CurrentClusterSize, 1, MPI_LONG, Task, CLUSTER_SIZE_EXCHG_TAG, MPI_COMM_WORLD, MPI_STATUS_IGNORE);

      /* Lines of the cluster for current Task */
      CurrentClusterLines = new long[CurrentClusterSize];

      Messages.str("");
      Messages << "Cluster " << Cluster << " Task " << Task << " CurrentClusterSize = " << CurrentClusterSize << endl;
      system_messages::information(Messages.str().c_str());
      
      MPI_Recv(CurrentClusterLines, CurrentClusterSize, MPI_LONG, Task, CLUSTER_LINES_EXCHG_TAG, MPI_COMM_WORLD, MPI_STATUS_IGNORE);

      /* Create the vector of current cluster lines */
      if (CurrentClusterLines != 0)
      {
        GlobalLinesPerCluster[Cluster].push_back(vector<long> (CurrentClusterLines, CurrentClusterLines + CurrentClusterSize));
      }
      else
      {
        GlobalLinesPerCluster[Cluster].push_back(vector<long> (0));
      }

      /* */
      Messages.str("");
      Messages << "Cluster " << Cluster << " Task " << Task << " Size = " << GlobalLinesPerCluster[Cluster][Task].size() << endl;
      system_messages::information(Messages.str().c_str());
    }
  }

  MPI_Barrier(MPI_COMM_WORLD);
  
  return ReconstructMasterPartition(GlobalLinesPerCluster);
#else
  return false;
#endif
}

/**
 * Implementation of the slave side of the gather operation
 *
 * \result True if the operation finished correctly, false otherwise
 */
bool libTraceClusteringImplementation::GatherSlave(void)
{
#ifdef HAVE_MPI
  /* Prepare vector of lines of each cluster */
  vector<vector<line_t> > LocalLinesPerCluster (LastPartition.NumberOfClusters ());
  
  vector<CPUBurst*>&    Bursts = Data->GetCompleteBursts();
  vector<cluster_id_t>& IDs    = LastPartition.GetAssignmentVector();

  for (size_t i = 0; i < Bursts.size(); i++)
  {
    LocalLinesPerCluster[IDs[i]].push_back(Bursts[i]->GetLine());
  }

  /* Send, in order, the size of each cluster and the lines themselves */
  for (size_t Cluster = 0; Cluster < LastPartition.NumberOfClusters(); Cluster++)
  {
    long CurrentClusterSize = LocalLinesPerCluster[Cluster].size();
    long CurrentClusterLines[CurrentClusterSize];
    ostringstream Messages;

    for (size_t Line = 0; Line < CurrentClusterSize; Line++)
    {
      CurrentClusterLines[Line] = (long) LocalLinesPerCluster[Cluster][Line];
    }

    /* Send the size */
    MPI_Send(&CurrentClusterSize, 1, MPI_LONG, 0, CLUSTER_SIZE_EXCHG_TAG, MPI_COMM_WORLD);
    /* Send the lines */
    MPI_Send(CurrentClusterLines, CurrentClusterSize, MPI_LONG, 0, CLUSTER_LINES_EXCHG_TAG, MPI_COMM_WORLD);

    Messages.str("");
    Messages << "Cluster " << Cluster << " Size = " << CurrentClusterSize << endl;
    system_messages::information(Messages.str().c_str());
  }

  MPI_Barrier(MPI_COMM_WORLD);
  
  return true;
#else
  return false;
#endif
}

/**
 * Reconstruct the LastPartition in the Master task using the structure created
 * in the 
 *
 * \result True if the operation finished correctly, false otherwise
 */
bool libTraceClusteringImplementation::ReconstructMasterPartition(vector<vector<vector<long> > >& GlobalLinesPerCluster)
{
  vector<CPUBurst*>&        CompleteBursts = Data->GetCompleteBursts();
  vector<cluster_id_t>      UnifiedIDs;
  map<line_t, cluster_id_t> LineToID;
  size_t                    TotalBursts = 0;

  ostringstream Messages;

  for (size_t Cluster = 0; Cluster < GlobalLinesPerCluster.size(); Cluster++)
  {
    for (size_t Task = 0; Task < GlobalLinesPerCluster[Cluster].size(); Task++)
    {
      /* DEBUG
      Messages.str("");
      Messages << "Cluster " << Cluster << " Task " << Task << " Size = " << GlobalLinesPerCluster[Cluster][Task].size() << endl;
      system_messages::information(Messages.str().c_str());
      */
      
      for (size_t Line = 0; Line < GlobalLinesPerCluster[Cluster][Task].size(); Line++)
      {
        LineToID[(line_t) GlobalLinesPerCluster[Cluster][Task][Line]] = (cluster_id_t) Cluster;
        Messages.str("");
        Messages << "Cluster " << Cluster << " Task " << Task << " Lines " << GlobalLinesPerCluster[Cluster][Task][Line] << endl;
        system_messages::information(Messages.str().c_str());
      }
    }
  }

  /* DEBUG */
  Messages.str("");
  Messages << "CompleteBursts size = " << CompleteBursts.size() << " ";
  Messages << "LineToID size = " << LineToID.size() << endl;
  system_messages::information(Messages.str().c_str());

  if (CompleteBursts.size() != LineToID.size())
  {
    SetError(true);
    SetErrorMessage("Data gathered from slave tasks doesn't match the number of bursts");
    return false;
  }
  
  for (size_t i = 0; i < CompleteBursts.size(); i++)
  {
    /*
    Messages.str("");
    Messages << "Burst #" << AllBursts[i]->GetInstance() << " Type = " << AllBursts[i]->GetBurstType() << endl;
    system_messages::information(Messages.str().c_str());
    */
    
    UnifiedIDs.push_back(LineToID[CompleteBursts[i]->GetLine()]);
  }

  /* Set the new assignment vector! */
  LastPartition.SetAssignmentVector (UnifiedIDs);
  
  /*
  Messages.str("");
  Messages << "UnifiedIDs size = " << UnifiedIDs.size();
  Messages << " TotalBursts = " << TotalBursts << endl;
  system_messages::information(Messages.str().c_str());
  */

  return true;
}
