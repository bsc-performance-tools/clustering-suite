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

#include "libTraceClusteringImplementation.hpp"

#include <ClusteringConfiguration.hpp>
#include <DataExtractor.hpp>
#include <DataExtractorFactory.hpp>
#include <PlottingManager.hpp>
#include <FileNameManipulator.hpp>

#include <SystemMessages.hpp>

#include <cerrno>
#include <cstring>

#include <fstream>
using std::ofstream;
using std::ios_base;

/**
 * Empty constructor
 */
libTraceClusteringImplementation::libTraceClusteringImplementation(bool verbose)
{
  system_messages::verbose = verbose;
}

/**
 * Initialize the clustering library. Loads the XML and initializes the configuration
 * object, core of the application
 * \param ClusteringDefinitionXML Name of the XML file where the XML is defined
 * \param ApplyCPIStack Boolean indicating if PPC970MP CPI stack counters should be extrapolated
 * \return True if initialization has been done properly. False otherwise
 */
bool
libTraceClusteringImplementation::InitTraceClustering(string        ClusteringDefinitionXML,
                                                      bool          ApplyCPIStack,
                                                      unsigned char UseFlags)
{
  ClusteringConfiguration *ConfigurationManager;
  ParametersManager       *Parameters;
  PlottingManager         *Plots;

  ConfigurationManager = ClusteringConfiguration::GetInstance();

  if (!ConfigurationManager->Initialize(ClusteringDefinitionXML, ApplyCPIStack))
  {
    SetError(true);
    SetErrorMessage(ConfigurationManager->GetLastError());
    return false;
  }

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

  if (USE_CLUSTERING(UseFlags))
  { /* Check if clustering defined in the XML is correct */
    if (ConfigurationManager->GetClusteringAlgorithmError())
    {
      SetErrorMessage(ConfigurationManager->GetClusteringAlgorithmErrorMessage());
      return false;
    }
  }

  if (USE_PLOTS(UseFlags))
  { /* Check if GNUplots defined in the XML are correct */

    if (!USE_CLUSTERING(UseFlags))
    { /* Data Extraction mode */
      Plots = PlottingManager::GetInstance(true);
    }
    else
    {
      Plots = PlottingManager::GetInstance(false);
    }

    if (Plots->GetError())
    {
      SetError(true);
      SetErrorMessage(Plots->GetLastError());
      return false;
    }
  }
  return true;
}

/**
 * Loads data from an input file. It could be a Paraver trace, Dimemas trace or
 * a previously generated CSV file. This method doesn't generate an output file
 * and should be used to later run an analysis.
 * \param InputFileName The name of the input file where data is located
 * \return True if the data extraction work properly. False otherwise
 */
bool
libTraceClusteringImplementation::ExtractData(string InputFileName)
{
  DataExtractorFactory*    ExtractorFactory;
  DataExtractor*           Extractor;

  /* Get the container */
  Data = TraceData::GetInstance();

  /* Get input file extractor */
  ExtractorFactory = DataExtractorFactory::GetInstance();
  if (!ExtractorFactory->GetExtractor(InputFileName, Extractor))
  {
    SetError(true);
    SetErrorMessage(ExtractorFactory->GetLastError());
    return false;
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

  if (!Data->FlushPoints(OutputStream, LastPartition.GetAssignmentVector()))
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

  string              ClusteringAlgorithmName;
  map<string, string> ClusteringAlgorithmParameters;

  if (Data == NULL)
  {
    SetErrorMessage("data not initialized");
    return false;
  }
  
  ClusteringCore = new libClustering();

  ConfigurationManager = ClusteringConfiguration::GetInstance();
  if (!ConfigurationManager->IsInitialized())
  {
    SetErrorMessage("clustering not initialized");
    return false;
  }

  ClusteringAlgorithmName       = ConfigurationManager->GetClusteringAlgorithmName();
  ClusteringAlgorithmParameters = ConfigurationManager->GetClusteringAlgorithmParameters();

  if (!ClusteringCore->InitClustering (ClusteringAlgorithmName,
                                       ClusteringAlgorithmParameters))
  {
    SetErrorMessage(ClusteringCore->GetErrorMessage());
    return false;
  }

  vector<const Point*>& ClusteringPoints = Data->GetClusteringPoints();

  cout << "Clustering Points size = " << ClusteringPoints.size() << endl;
  
  if (!ClusteringCore->ExecuteClustering(ClusteringPoints, LastPartition))
  {
    SetErrorMessage(ClusteringCore->GetErrorMessage());
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
bool
libTraceClusteringImplementation::PrintPlotScripts(string DataFileName,
                                                   string ScriptsFileNamePrefix)
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

  if (!Plots->PrintPlots(DataFileName, Prefix, LastPartition.GetNumberOfClusters()))
  {
    SetError(true);
    SetErrorMessage(Plots->GetLastError());
    return false;
  }
  
  return true;
}
