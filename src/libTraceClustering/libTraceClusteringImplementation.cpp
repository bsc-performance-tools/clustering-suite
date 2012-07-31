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

#include "libTraceClusteringImplementation.hpp"
#include "libTraceClustering.hpp"

#include <DBSCAN.hpp>

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

#include "SequenceScore.hpp"
#include "ClusteringRefinementDivisive.hpp"
#include "ClusteringRefinementAggregative.hpp"

#include <cerrno>
#include <cstring>

#include <fstream>
using std::ofstream;
using std::ios_base;

#include <sstream>
using std::ostringstream;

#include <algorithm>
using std::sort;

#ifdef HAVE_MPI
#include <mpi.h>

#define CLUSTER_SIZE_EXCHG_TAG  1
#define CLUSTER_LINES_EXCHG_TAG 2

#endif

const string libTraceClusteringImplementation::DataFilePostFix        = ("DATA");
const string libTraceClusteringImplementation::SampledDataFilePostFix = ("SAMPLE_DATA");

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
 * \param SampleData    True if data read from trace must be sampled
 * \param MaxSamples    Maximum number of burst to be samples
 * \param EventsToDealWith Set of parameters in case we want to do an event parsing
 *                         of a Paraver trace
 *
 * \return True if the data extraction work properly. False otherwise
 */
bool
libTraceClusteringImplementation::ExtractData(string            InputFileName,
                                              bool              SampleData,
                                              unsigned int      MaxSamples,
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

  if (SampleData)
  {
    this->SampleData = true;
    Data->Sampling(MaxSamples);
  }

  if (Extractor->GetFileType() == ClusteringCSV)
  {
    if (Extractor->GetPartition(LastPartition))
    {
      ClusteringExecuted = true;

      Statistics.InitStatistics(LastPartition.GetIDs());

      if (!Statistics.ComputeStatistics(Data->GetCompleteBursts(),
                                        LastPartition.GetAssignmentVector()))
      {
        SetErrorMessage(Statistics.GetLastError());
        return false;
      }
    }
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
bool libTraceClusteringImplementation::FlushData(string OutputCSVFileNamePrefix)
{
  string OutputFileName = OutputCSVFileNamePrefix + "." +
                          libTraceClusteringImplementation::DataFilePostFix +
                          ".csv";

  ofstream     OutputStream (OutputFileName.c_str(), ios_base::trunc);

  Partition   &PartitionUsed = (SampleData ? ClassificationPartition : LastPartition);
  DataPrintSet WhatToPrint;

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

  if (USE_CLUSTERING(UseFlags) || USE_CLUSTERING_REFINEMENT(UseFlags))
  {
    WhatToPrint = PrintCompleteBursts;
  }
  else
  {
    WhatToPrint = PrintAllBursts;
  }

  if (USE_CLUSTERING(UseFlags) || USE_CLUSTERING_REFINEMENT(UseFlags))
  {
    if (!Data->FlushPoints(OutputStream,
                           PartitionUsed.GetAssignmentVector(),
                           WhatToPrint))
    {
      SetError(true);
      SetErrorMessage(Data->GetLastError());
      return false;
    }
  }
  else
  {
    if (!Data->FlushPoints(OutputStream,
                           vector<cluster_id_t> (0),
                           WhatToPrint))
    {
      SetError(true);
      SetErrorMessage(Data->GetLastError());
      return false;
    }
  }

  OutputStream.close();

  /* When sampling data, also the sample subset is printed */
  if (SampleData)
  {

    string SampledOutputFileName = OutputCSVFileNamePrefix + "."
                      + libTraceClusteringImplementation::SampledDataFilePostFix
                      + ".csv";

    ofstream SampledOutputStream (SampledOutputFileName.c_str(), ios_base::trunc);
    if (!SampledOutputStream)
    {
      SetError(true);
      SetErrorMessage("unable to open smapling output file", strerror(errno));
      return false;
    }

    if (!Data->FlushPoints(SampledOutputStream,
                           LastPartition.GetAssignmentVector(),
                           PrintClusteringBursts))
    {
      SetError(true);
      SetErrorMessage(Data->GetLastError());
      return false;
    }

    SampledOutputStream.close();
  }

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

  /* Statistics */
  Parameters = ParametersManager::GetInstance();

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

  if (SampleData)
  {
    ClusteringStatistics SamplingStatistics;

    SamplingStatistics.InitStatistics(LastPartition.GetIDs(),
                                      Parameters->GetClusteringParametersNames(),
                                      Parameters->GetClusteringParametersPrecision(),
                                      Parameters->GetExtrapolationParametersNames(),
                                      Parameters->GetExtrapolationParametersPrecision());

    if (!SamplingStatistics.ComputeStatistics(Data->GetClusteringBursts(),
                                              LastPartition.GetAssignmentVector()))
    {
      SetErrorMessage(SamplingStatistics.GetLastError());
      return false;
    }

    SamplingStatistics.TranslatedIDs(LastPartition.GetAssignmentVector());
  }

  /* If cluster analysis was executed using sampling data, we must apply
     a classification */
  if (SampleData)
  {
    vector<const Point*> &CompletePoints = Data->GetCompletePoints();

    if (!ClusteringCore->ClassifyData(CompletePoints, ClassificationPartition))
    {
      SetErrorMessage(ClusteringCore->GetErrorMessage());
      return false;
    }
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



  Partition& PartitionUsed = (SampleData ? ClassificationPartition : LastPartition);

  Statistics.InitStatistics(PartitionUsed.GetIDs(),
                            Parameters->GetClusteringParametersNames(),
                            Parameters->GetClusteringParametersPrecision(),
                            Parameters->GetExtrapolationParametersNames(),
                            Parameters->GetExtrapolationParametersPrecision());

  if (!Statistics.ComputeStatistics(Data->GetCompleteBursts(),
                                    PartitionUsed.GetAssignmentVector()))
  {
    SetErrorMessage(Statistics.GetLastError());
    return false;
  }

  Statistics.TranslatedIDs(PartitionUsed.GetAssignmentVector());

  return true;
}

/**
 * Performs a DBSCAN cluster analysis with auto refinement based on sequence
 * score. The exploration range is guessed automatically
 *
 * \param Divisive             True if the refinement will be top down, false if
 *                             it would be bottom up
 * \param OutputFileNamePrefix Prefix of the output files for each step data and plots
 *
 * \result True if the analysis finished correctly, false otherwise
 */
bool libTraceClusteringImplementation::ClusterRefinementAnalysis(bool   Divisive,
                                                                 bool   PrintStepsInformation,
                                                                 string OutputFileNamePrefix)
{
  map<string, string> ClusteringAlgorithmParameters;
  ostringstream       Converter;
  ostringstream       Messages;

  vector<double>      Distances;
  vector<double>      Epsilons;

  size_t StopPosition;

  size_t TraceObjects;
  int    MinPoints;

  if (Data == NULL)
  {
    SetErrorMessage("data not initialized");
    return false;
  }

  if ((TraceObjects = Data->GetTraceObjects()) == 0)
  {
    SetErrorMessage("number of objects not available. Automatic refinement not available on TRF traces");
    return false;
  }

  MinPoints = TraceObjects/4;

  if (MinPoints <= 1)
  {
    MinPoints = 2;
  }

  Messages.str("");
  Messages << "|-> Selected MinPoints = " << MinPoints << endl;
  system_messages::information(Messages.str());

  /* Compute the k-neighbours distance */

  Messages.str("");
  Messages << "|-> Computing K-Neighbour Data" << endl;
  system_messages::information(Messages.str());


  Converter << 0.0;
  ClusteringAlgorithmParameters.insert(std::make_pair(DBSCAN::EPSILON_STRING, string(Converter.str())));
  Converter.str("");
  Converter << MinPoints;
  ClusteringAlgorithmParameters.insert(std::make_pair(DBSCAN::MIN_POINTS_STRING, string(Converter.str())));

  DBSCAN DBSCANCore = DBSCAN(ClusteringAlgorithmParameters);

  bool verbose_state = system_messages::verbose;
  system_messages::verbose = false;
  DBSCANCore.ComputeNeighbourhood(Data->GetClusteringPoints(), MinPoints, Distances);
  system_messages::verbose = verbose_state;

  /* 10 different epsilons 1% to 10% "NOISE"
  for (size_t i = 0; i < Distances.size(); i++)
  {
    if (Distances[i] < 0.001)
    {
      StopPosition = i;
      break;
    }
  }
  */

  /*
  cout << "POSITIONS = ";
  for (size_t i = 0; i < 10; i++)
  {
    int Position = (StopPosition*(i+1))/100;
    /* DEBUG
    cout << Position << " ";
    Epsilons.push_back(Distances[Position]);
  }
  cout << endl;

  srand(time(NULL));

  size_t MinPos = (Distances.size()*1)/100;
  size_t MaxPos = (Distances.size()*10)/100;

  /*
  cout << "POSITIONS = ";
  for (size_t i = 0; i < 10; i++)
  {
    int Position = (rand()%(MaxPos-MinPos+1))+MinPos;
    /* DEBUG
    cout << Position << " ";
    Epsilons.push_back(Distances[Position]);
  }
  cout << endl; */

  /* New way to find Epsilons */
  size_t Elements   = Distances.size();
  size_t MinX       = static_cast<size_t>((Elements*5)/1000);
  size_t MaxX       = static_cast<size_t>((Elements*995)/1000);
  //size_t MinX = 0;
  //size_t MaxX = Elements - 1;

  double SumEps = 0;
  for (size_t i = 0; i < Distances.size(); i++)
  {
    SumEps += Distances[i];
  }

  double AvgEps     = SumEps/Distances.size();
  double MaxEpsilon = Distances[MinX];   // < 0.5%  data discarded
  double MinEpsilon = Distances[MaxX];   // > 99.5% data discarded

  /* DEBUG
  cout << "MaxEpsilon = " << MaxEpsilon << " ";
  cout << "MinEpsilon = " << MinEpsilon << " ";
  cout << "Avg.Epsilon = " << AvgEps << endl;
  */

  double Intercept = MaxEpsilon;
  double Slope     = -1.0*(MaxEpsilon/(MaxX/2));
  // double Slope     = -1.0*(MaxEpsilon/MaxX);

  size_t MaxIndex = 0;
  double MaxDelta = 0;
  double MinDelta = MAX_DOUBLE;
  double SumDeltas = 0;


  /*
  ofstream DISTANCES;
  DISTANCES.open("next_point_distance.csv");
  double MaxPairDistance = 0;
  double MinPairDistance = MAX_DOUBLE;
  double SumPairDistance = 0;

  // vector<double> RangePairDistances;
  vector<double> NewCandidates;

  cout << "New Candidates = [ ";
  for (size_t i = 0; i < Distances.size()-1; i++)
  {
    double PairDistance  = Distances[i] - Distances[i+1];
    SumPairDistance     += PairDistance;

    if (PairDistance > 0.0005 && PairDistance < 0.0015)
    {
      NewCandidates.push_back(Distances[i]);
      cout << Distances[i] << " ";
    }
    if (PairDistance > MaxPairDistance)
    {
      MaxPairDistance = PairDistance;
    }

    if (PairDistance < MinPairDistance)
    {
      MinPairDistance = PairDistance;
    }
    // DISTANCES << PairDistance << endl;
  }
  cout << "]" << endl;
  // DISTANCES.close();

  for (size_t i = 0; i < Distances.size(); i++)
  {
    double PairDistance  = Distances[i] - Distances[i+1];

    DISTANCES << ( PairDistance - MinPairDistance ) / ( MaxPairDistance - MinPairDistance) << endl;
  }

  DISTANCES.close();

  // exit(EXIT_SUCCESS);

  cout << "MaxPairDistance = " << MaxPairDistance << " ";
  cout << "MinPairDistance = " << MinPairDistance << " ";
  cout << "AvgPairDistance = " << SumPairDistance/ (Distances.size()-1) << endl;
  */


  /* DEBUG: Avg Delta
  ofstream DELTAS;

  DELTAS.open("deltas.csv");
  for (size_t i = MinX; i <= (MaxX/2); i++)
  {
    double Delta = ((Slope * i) + Intercept) - Distances[i];

    SumDeltas += Delta;

    /* DEBUG
    DELTAS << Delta << endl;;

    if (Delta > MaxDelta)
    {
      MaxIndex = i;
      MaxDelta = Delta;
    }
    else if (Delta < MinDelta)
    {
      MinDelta = Delta;
    }
  }
  DELTAS.close();

  cout << "Average Delta = " << SumDeltas/Elements << endl;
  cout << "Max Delta = " << MaxDelta << endl;
  cout << "Min Delta = " << MinDelta << endl;

  */

  /* */
  for (size_t i = MinX; i <= (MaxX/2); i++)
  {
    double Delta = ((Slope * i) + Intercept) - Distances[i];

    if (Delta > MaxDelta)
    {
      MaxIndex = i;
      MaxDelta = Delta;
    }
  }

  /* DEBUG: 2on Delta!
  double MaxDelta2 = 0;
  for (size_t i = MinX; i <= MaxDelta; i++)
  {
    double Delta = ((Slope * i) + Intercept) - Distances[i];

    if (Delta > MaxDelta2)
    {
      MaxIndex = i;
      MaxDelta2 = Delta;
    }
  }
  */

  // cout << "Candidate MinEpsilon: idx = " << MaxIndex << " Value = " << Distances[MaxIndex] << endl;

  MinEpsilon        = Distances[MaxIndex];
  size_t IndexRange = MaxIndex - MinX;

  if (IndexRange < 10)
  {
    for (size_t i = 0; i <= IndexRange; i++)
    {
      Epsilons.push_back(Distances[MinX+i]);
    }
  }
  else
  {
    // double StepSize   = (MaxEpsilon - MinEpsilon)/10;
    // cout << "Elements = " << Elements << endl;
    // cout << "MinX = " << MinX << " MaxIndex = " << MaxIndex << endl;
    // cout << "Positions = [ ";
    for (size_t i = 0; i < 10; i++)
    {
      // Epsilons.push_back(MinEpsilon + (StepSize*(i+1)));
      size_t CurrentIndex = MinX + ((10*(i+1)*IndexRange)/100);
      // cout << CurrentIndex << " ";
      Epsilons.push_back(Distances[CurrentIndex]);
    }
    // cout << "]" << endl;
  }

  /* DEBUG!
  Epsilons = NewCandidates;
  */

  if (Divisive)
  {
    sort(Epsilons.rbegin(), Epsilons.rend());
  }
  else
  { /* When using aggregative version, Epsilons should be sorted increasing */
    sort(Epsilons.begin(), Epsilons.end());
  }

  Messages.str("");
  Messages << "|-> Epsilon Values = [ ";

  /* DEBUG */
  for (size_t i = 0; i < Epsilons.size(); i++)
  {
    Messages << Epsilons[i] << " ";

  }
  Messages << "]" << endl;
  system_messages::information(Messages.str());

  // exit (EXIT_SUCCESS);

  return GenericRefinement(Divisive,
                           MinPoints,
                           Epsilons,
                           PrintStepsInformation,
                           OutputFileNamePrefix);

}

/**
 * Performs a DBSCAN cluster analysis with auto refinement based on sequence
 * score. The exploration range is provided by the user
 *
 * \param Divisive             True if the refinement will be top down, false if
 *                             it would be bottom up
 * \param MinPoints Fixed MinPoints value used in all runs of DBSCAN
 * \param MaxEps Maximum value of Epsilon
 * \param MinEps Minimum value of Epsilon
 * \param Steps  Number of iterarions of the algorithm
 * \param OutputFileNamePrefix Prefix of the output files for each step data and plots
 *
 * \return True if the analysis finished correctly, false otherwise
 */
bool libTraceClusteringImplementation::ClusterRefinementAnalysis(bool   Divisive,
                                                                 int    MinPoints,
                                                                 double MaxEps,
                                                                 double MinEps,
                                                                 int    Steps,
                                                                 bool   PrintStepsInformation,
                                                                 string OutputFileNamePrefix)
{
  vector<double>     EpsilonPerLevel;
  double             StepSize    = (MaxEps - MinEps)/(Steps-1);

  if (Data == NULL)
  {
    SetErrorMessage("data not initialized");
    return false;
  }

  if (Divisive)
  {
    EpsilonPerLevel.push_back(MaxEps);
    for (size_t i = 1; i < Steps; i++)
    {
      EpsilonPerLevel.push_back(EpsilonPerLevel[i-1] - StepSize);
    }
  }
  else
  {
    EpsilonPerLevel.push_back(MinEps);
    for (size_t i = 1; i < Steps; i++)
    {
      EpsilonPerLevel.push_back(EpsilonPerLevel[i-1] + StepSize);
    }
  }

  return GenericRefinement(Divisive,
                           MinPoints,
                           EpsilonPerLevel,
                           PrintStepsInformation,
                           OutputFileNamePrefix);
}

/**
 * Effectively executes the refinement analysis
 *
 * \param Divisive             True if the refinement will be top down, false if
 *                             it would be bottom up
 * \param MinPoints Fixed MinPoints value used in all runs of DBSCAN
 * \param EpsilonPerLevel      Vector with all values of epsilon to be used
 * \param OutputFileNamePrefix Prefix of the output files for each step data and plots
 *
 * \return True if the analysis finished correctly, false otherwise
 */
bool libTraceClusteringImplementation::GenericRefinement(bool           Divisive,
                                                         int            MinPoints,
                                                         vector<double> EpsilonPerLevel,
                                                         bool           PrintStepsInformation,
                                                         string         OutputFileNamePrefix)
{
  if (SampleData)
  {
    SetErrorMessage("Refinement analysis plus sampling not implemented yet");
    SetError(true);
    return false;
  }

  ParametersManager *Parameters;
  vector<Partition>  PartitionsHierarchy;

  if (Divisive)
  {
    ClusteringRefinementDivisive RefinementAnalyzer(MinPoints,
                                                    EpsilonPerLevel);

    if (!RefinementAnalyzer.Run(Data->GetClusteringBursts(),
                                PartitionsHierarchy,
                                LastPartition,
                                PrintStepsInformation,
                                OutputFileNamePrefix))
    {
      SetErrorMessage(RefinementAnalyzer.GetLastError());
      SetError(true);
      return false;
    }
  }
  else
  {
    ClusteringRefinementAggregative RefinementAnalyzer(MinPoints,
                                                       EpsilonPerLevel);

    if (!RefinementAnalyzer.Run(Data->GetClusteringBursts(),
                                PartitionsHierarchy,
                                LastPartition,
                                PrintStepsInformation,
                                OutputFileNamePrefix))
    {
      SetErrorMessage(RefinementAnalyzer.GetLastError());
      SetError(true);
      return false;
    }
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

  /* Statistics */
  Parameters = ParametersManager::GetInstance();

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

  // Statistics.TranslatedIDs(LastPartition.GetAssignmentVector());

  /* Generate score and all intermediate (event) traces */
  if (PrintStepsInformation)
  {
    /* Compute resulting score */
    if (!ComputeSequenceScore (OutputFileNamePrefix, true))
    {
      return false;
    }

    for (size_t i = 0; i < PartitionsHierarchy.size(); i++)
    {
      ClusteredTraceGenerator* TraceGenerator;
      ostringstream OutputTraceName, Messages;

      OutputTraceName << OutputFileNamePrefix << ".STEP" << i+1 << ".prv";

      ClusteringStatistics Stats;

      Messages << "****** Writing events trace of STEP " << i+1 << " ******" << endl;
      system_messages::information(Messages.str());

      /* Sort IDs
      Stats.InitStatistics(PartitionsHierarchy[i].GetIDs(),
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
      */

      // Stats.TranslatedIDs(PartitionsHierarchy[i].GetAssignmentVector());


      if (PRVEventsParsing)
      {
        TraceGenerator = new ClusteredEventsPRVGenerator (InputFileName,
                                                          OutputTraceName.str());

        TraceGenerator->SetEventsToDealWith(EventsToDealWith);
      }
      else
      {
        TraceGenerator = new ClusteredStatesPRVGenerator (InputFileName,
                                                          OutputTraceName.str());
      }

      /* DEBUG
      cout << "All Burst size = " << Data->GetAllBursts().size() << endl;
      cout << "IDs size = " << PartitionsHierarchy[i].GetAssignmentVector().size() << endl; */

      // bool verbose_state = system_messages::verbose;
      // system_messages::verbose = false;

      if (!TraceGenerator->Run(Data->GetAllBursts(),
                               PartitionsHierarchy[i].GetAssignmentVector(),
                               PartitionsHierarchy[i].GetIDs(),
                               true)) // Minimize information
      {
          // system_messages::verbose = verbose_state;
          SetErrorMessage(TraceGenerator->GetLastError());
          return false;
      }
      // system_messages::verbose = verbose_state;

      delete TraceGenerator;
    }
  }

  ClusteringExecuted = true;

  return true;
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
  map<cluster_id_t, percentage_t> PercentageDurations;
  SequenceScore                   Scoring;
  vector<SequenceScoreValue>      ScoresPerCluster;
  double                          GlobalScore;
  bool                            PrintScore;

  if (OutputFilePrefix.compare("") == 0)
  {
    PrintScore = false;
  }
  else
  {
    PrintScore = true;
  }


  if (Data == NULL)
  {
    SetErrorMessage("data not initialized");
    return false;
  }

  PercentageDurations = Statistics.GetPercentageDurations();

  if(!Scoring.ComputeScore(Data->GetClusteringBursts(),
                           LastPartition.GetAssignmentVector(),
                           PercentageDurations,
                           ScoresPerCluster,
                           GlobalScore,
                           PrintScore,
                           OutputFilePrefix,
                           FASTASequencesFile))
  {
    SetError(true);
    SetErrorMessage("unable to compute sequences score");
    return false;
  }

  return true;

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

  Partition& PartitionUsed = (SampleData ? ClassificationPartition : LastPartition);

  vector<cluster_id_t>& IDs = PartitionUsed.GetAssignmentVector();

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
                               PartitionUsed.GetAssignmentVector(),
                               PartitionUsed.GetIDs()))
  {
    SetErrorMessage(TraceReconstructor->GetLastError());
    return false;
  }

  return true;
}

/**
 * Print the plot scripts for GNUPlot defined in the XML
 *
 * \param DataFileNamePrefix Prefix of the file containg the data to plot
 * \param ScriptsFileNamePrefix Prefix of the output scripts
 *
 * \result True if the scripts where printed correctly, false otherwise
 */
bool libTraceClusteringImplementation::PrintPlotScripts(string DataFileNamePrefix,
                                                        string ScriptsFileNamePrefix)
{
  PlottingManager *Plots;
  string Prefix;
  string PlotTitle;

  Partition &PartitionUsed = (SampleData ? ClassificationPartition : LastPartition);

  Plots = PlottingManager::GetInstance();

  if (ScriptsFileNamePrefix.compare("") == 0)
  {
    Prefix = DataFileNamePrefix;
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

  if (!Plots->PrintPlots(DataFileNamePrefix + "." + libTraceClusteringImplementation::DataFilePostFix + ".csv",
                         Prefix,
                         PlotTitle,
                         PartitionUsed.GetIDs()))
  {
    SetError(true);
    SetErrorMessage(Plots->GetLastError());
    return false;
  }

  if(SampleData)
  {
    if (!Plots->PrintPlots(DataFileNamePrefix + "." + libTraceClusteringImplementation::SampledDataFilePostFix +".csv",
                           Prefix + "." + libTraceClusteringImplementation::SampledDataFilePostFix,
                           PlotTitle + " (Sampled Data)",
                           PartitionUsed.GetIDs()))
    {
      SetError(true);
      SetErrorMessage(Plots->GetLastError());
      return false;
    }
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

  /* DEBUG
  Messages.str("");
  Messages << "CompleteBursts size = " << CompleteBursts.size() << " ";
  Messages << "LineToID size = " << LineToID.size() << endl;
  system_messages::information(Messages.str().c_str()); */

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
