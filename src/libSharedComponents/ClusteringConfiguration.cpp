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

#include "ClusteringConfiguration.hpp"
#include "XMLParser.hpp"

#include <errno.h>
#include <string.h>

#include <iostream>
using std::cout;
using std::endl;

/******************************************************************************
 * Singleton pointer
 *****************************************************************************/

ClusteringConfiguration* ClusteringConfiguration::_ClusteringConfiguration = NULL;

/******************************************************************************
 * GetInstance static methods
 *****************************************************************************/

/**
 * Parametrized 'GetInstance', used only first time invocation
 * \param XMLFileName The name of the XML where the clustering is defined
 * \param ApplyCPIStack Boolean which sets if CPI stack parameters should be used
 * \return The singleton instance of the clustering configuration container

ClusteringConfiguration*
ClusteringConfiguration::GetInstance(string XMLFileName,
                                     bool   ApplyCPIStack)
{
  if (ClusteringConfiguration::_ClusteringConfiguration == NULL)
  {
    ClusteringConfiguration::_ClusteringConfiguration =
      new ClusteringConfiguration(XMLFileName,
                                  ApplyCPIStack);
  }

  return ClusteringConfiguration::_ClusteringConfiguration;
}
*/

/**
 * General 'GetInstance' method, used along all application
 * \return The singleton instance of the clustering configuration container, null it has not been initialized
 */
ClusteringConfiguration* ClusteringConfiguration::GetInstance(void)
{
  if (ClusteringConfiguration::_ClusteringConfiguration == NULL)
  {
    _ClusteringConfiguration = new ClusteringConfiguration();
  }
  
  return ClusteringConfiguration::_ClusteringConfiguration;
}

/******************************************************************************
 * Initialization.
 *****************************************************************************/

/**
 * Empty constructor. Sets initialization to false
 *
 */
ClusteringConfiguration::ClusteringConfiguration()
{
  Initialized = false;
}

/**
 * Initialization of the class. Starts the XML parser to set up all attributes
 *
 * \param XMLFileName Name of the XML clustering configuration file
 * \param ApplyCPIStack  Boolean which sets if CPI stack extrapolation metrics should be used
 *
 * \return True if XML parsing worked properly, false otherwise
 */
bool ClusteringConfiguration::Initialize(string XMLFileName)
{
  FILE*     XMLFile;
  XMLParser Parser;

  Distributed     = false;
  DurationFilter  = 0.0;
  FilterThreshold = 0.0;
  NormalizeData   = true;

  Sampling           = NoSampling;
  SamplingPercentage = 0.0;

  if ( (XMLFile = fopen(XMLFileName.c_str(), "r")) == NULL)
  {
    cout << "XML FILE = " << XMLFileName << endl;
    SetErrorMessage("unable to open clustering definition file",
                    strerror(errno));
    SetError(true);
    return false;
  }
  fclose(XMLFile);

  if (!Parser.ParseXML(XMLFileName,
                       this))
  {
    SetErrorMessage("XML parsing error", Parser.GetLastError());
    SetError(true);
    return false;
  }

  Initialized = true;

  /* DEBUG
  cout << "Duration Filter = " << DurationFilter << endl;
  */
  
  return true;
}

/**
 * Returns if the configuration has been initialized
 *
 * \return True if configuration has been initialized, false otherwise
 */
bool ClusteringConfiguration::IsInitialized(void) const
{
  return Initialized;
}

/******************************************************************************
 * Setters and Getters of general attributes
 *****************************************************************************/
/**
 * Sets if the clustering is running over MPI
 */
void ClusteringConfiguration::SetDistributed(bool _Distributed)
{
  Distributed = _Distributed;
}

/**
 * Returns is clustering is running over MPI
 */
bool ClusteringConfiguration::GetDistributed(void)
{
  return Distributed;
}

/**
 * Set the rank of current instance of the library in a distributed environment
 */
void ClusteringConfiguration::SetMyRank(INT32 _MyRank)
{
  MyRank = _MyRank;
}

/**
 * Returns the rank of current instance of the library in a distributed environment
 */
INT32 ClusteringConfiguration::GetMyRank(void)
{
  return MyRank;
}

/**
 * Set the total number of ranks available on current instance of the library in a distributed environment
 */
void ClusteringConfiguration::SetTotalRanks(INT32 _TotalRanks)
{
  TotalRanks = _TotalRanks;

}

/**
 * Returns the total number of ranks available on current instance of the library in a distributed environment
 */
INT32 ClusteringConfiguration::GetTotalRanks(void)
{
  return TotalRanks;
}

/**
 * Sets the DurationFilter attribute
 */
void
ClusteringConfiguration::SetDurationFilter(duration_t DurationFilter)
{
  this->DurationFilter = DurationFilter;
}

/**
 * Returns the DurationFilter attribute
 * \return The value of the duration filter for short bursts
 */
duration_t
ClusteringConfiguration::GetDurationFilter(void)
{
  return DurationFilter;
}

/**
 * Sets the FilterThreshold attribute
 */
void
ClusteringConfiguration::SetFilterThreshold(double FilterThreshold)
{
  this->FilterThreshold = FilterThreshold;
}

/**
 * Returns the FilterThreshold attribute
 * \return The filter threshold (% of total bursts time) for small clusters
 */
double
ClusteringConfiguration::GetFilterThreshold(void)
{
  this->FilterThreshold = FilterThreshold;
}

/**
 * Sets the NormalizeData attribute
 */
void
ClusteringConfiguration::SetNormalizeData(bool NormalizeData)
{
  this->NormalizeData = NormalizeData;
}

/**
 * Returns the NormalizeData attribute
 * \return True if a data range normalization is needed
 */
bool
ClusteringConfiguration::GetNormalizeData(void)
{
  return NormalizeData;
}

/******************************************************************************
 * Clustering algorithm getters and setters
 *****************************************************************************/

/**
 * Sets the clustering algorithm parameters
 * \param ClusteringAlgorithm String containing the name of the algorithm
 * \param ClusteringAlgorithmParameters Parameters to tune the clustering algorithm, not the dimensions
 */
void
ClusteringConfiguration::SetClusteringAlgorithm(string                ClusteringAlgorithmName,
                                                map<string, string>   ClusteringAlgorithmParameters)
{
  this->ClusteringAlgorithmName       = ClusteringAlgorithmName;
  this->ClusteringAlgorithmParameters = ClusteringAlgorithmParameters;
}

/**
 * Returns the clustering algorithm
 * \return The clustering algorithm selected in the XML
 */
string
ClusteringConfiguration::GetClusteringAlgorithmName(void)
{
  return ClusteringAlgorithmName;
}

/**
 * Returns the clustering algorithm parameters
 * \return A map of paramter/value of all clustering algorithms
 */
map<string, string>&
ClusteringConfiguration::GetClusteringAlgorithmParameters(void)
{
  return ClusteringAlgorithmParameters;
}

/**
 * Sets the parsing error in the clustering algorithm
 * \param ClusteringAlgorithmError Boolean seting the possible parsing error
 */
void
ClusteringConfiguration::SetClusteringAlgorithmError(bool ClusteringAlgorithmError)
{
  this->ClusteringAlgorithmError = ClusteringAlgorithmError;
}

/**
 * Returns the error state of clustering algorithm parsing
 * \return Boolean seting the possible clustering algorithm parsing error
 */
bool
ClusteringConfiguration::GetClusteringAlgorithmError(void)
{
  return ClusteringAlgorithmError;
}

/**
 * Sets the error description of a possible parsing error in the clustering algorithm
 * \param ClusteringAlgorithmErrorMessage String describing the possible error in clustering algorithm parsing
 */
void
ClusteringConfiguration::SetClusteringAlgorithmErrorMessage(string ClusteringAlgorithmErrorMessage)
{
  this->ClusteringAlgorithmErrorMessage = ClusteringAlgorithmErrorMessage;
}

/**
 * Returns the error description of a possible parsing error in the clustering algorithm
 * \return ClusteringAlgorithmErrorMessage String describing the possible error in clustering algorithm parsing
 */
string
ClusteringConfiguration::GetClusteringAlgorithmErrorMessage(void)
{
  return ClusteringAlgorithmErrorMessage;
}

/******************************************************************************
 * Clustering Parameters getters and setters
 *****************************************************************************/

/**
 * Sets the metrics that will be used by the clustering algorithm
 * \param ParametersNames vector of strings containing the parameter names
 * \param ParametersDefinition vector of parameters definition containers
 */
void
ClusteringConfiguration::SetClusteringParameters(vector<string>&             ParametersNames,
                                                 vector<ParameterContainer>& ParametersDefinition)
{
  this->ClusteringParametersNames       = ParametersNames;
  this->ClusteringParametersDefinitions = ParametersDefinition;
}

/**
 * Returns the clustering metrics names
 * \return Vector containing names of each clustering metric
 */
vector<string>&
ClusteringConfiguration::GetClusteringParametersNames(void)
{
  return this->ClusteringParametersNames;
}

/**
 * Returns the clustering metrics definitions
 * \return Vector containing the definition of each clustering metric
 */
vector<ParameterContainer>&
ClusteringConfiguration::GetClusteringParametersDefinitions(void)
{
  return this->ClusteringParametersDefinitions;
}

/**
 * Sets the parsing error in the clustering metrics
 * \param ClusteringParametersError Boolean seting the possible clustering metrics parsing error 
 */
void
ClusteringConfiguration::SetClusteringParametersError(bool ClusteringParametersError)
{
  this->ClusteringParametersError = ClusteringParametersError;
}

/**
 * Returns the error state of clustering metrics parsing
 * \return Boolean seting the possible clustering metrics parsing error
 */
bool
ClusteringConfiguration::GetClusteringParametersError(void)
{
  return ClusteringParametersError;
}

/**
 * Sets the error description of a possible parsing error in the clustering metrics
 * \param ClusteringParametersErrorMessage String describing the possible error in clustering metrics parsing
 */
void
ClusteringConfiguration::SetClusteringParametersErrorMessage(string ClusteringParametersErrorMessage)
{
  this->ClusteringParametersErrorMessage = ClusteringParametersErrorMessage;
}

/**
 * Returns the error description of a possible parsing error in the clustering metrics
 * \return ClusteringParametersErrorMessage String describing the possible error in clustering metrics parsing
 */
string
ClusteringConfiguration::GetClusteringParametersErrorMessage(void)
{
  return ClusteringParametersErrorMessage;
}

/******************************************************************************
 * Clustering Parameters getters and setters
 *****************************************************************************/

/**
 * Sets the metrics that will be extrapolated
 * \param ParametersNames vector of strings containing the parameter names
 * \param ParametersDefinition vector of parameters definition containers
 */
void 
ClusteringConfiguration::SetExtrapolationParameters(vector<string>&            ParametersNames,
                                                   vector<ParameterContainer>& ParametersDefinition)
{
  this->ExtrapolationParametersNames       = ParametersNames;
  this->ExtrapolationParametersDefinitions = ParametersDefinition;
}

/**
 * Returns the extrapolation metrics names
 * \return Vector containing names of each extrapolation metric
 */
vector<string>&
ClusteringConfiguration::GetExtrapolationParametersNames(void)
{
  return this->ExtrapolationParametersNames;
}

/**
 * Returns the extrapolation metrics definitions
 * \return Vector containing the extrapolation metrics definition
 */
vector<ParameterContainer>&
ClusteringConfiguration::GetExtrapolationParametersDefinitions(void)
{
  return this->ExtrapolationParametersDefinitions;
}

/**
 * Sets the parsing error in the extrapolation metrics
 * \param ExtrapolationParametersError Boolean seting the possible extrapolation metrics parsing error 
 */
void
ClusteringConfiguration::SetExtrapolationParametersError(bool ExtrapolationParametersError)
{
  this->ExtrapolationParametersError = ExtrapolationParametersError;
}

/**
 * Returns the error state of extrapolation metrics parsing
 * \return Boolean seting the possible extrapolation metrics parsing error
 */
bool
ClusteringConfiguration::GetExtrapolationParametersError(void)
{
  return ExtrapolationParametersError;
}

/**
 * Sets the error description of a possible parsing error in the extrapolation metrics
 * \param ExtrapolationParametersErrorMessage String describing the possible error in extrapolation metrics parsing
 */
void
ClusteringConfiguration::SetExtrapolationParametersErrorMessage(string ExtrapolationParametersErrorMessage)
{
  this->ExtrapolationParametersErrorMessage = ExtrapolationParametersErrorMessage;
}

/**
 * Returns the error description of a possible parsing error in the extrapolation metrics
 * \return String describing the possible error in extrapolation metrics parsing
 */
string
ClusteringConfiguration::GetExtrapolationParametersErrorMessage(void)
{
  return ExtrapolationParametersErrorMessage;
}

/******************************************************************************
 * Output Plots definitions getters and setters
 *****************************************************************************/

/**
 * Sets if all possible combination of plots must be written
 *
 * \param Boolean containing if the plots must be written
 */
void
ClusteringConfiguration::SetAllPlots(bool AllPlots)
{
  this->AllPlots = AllPlots;
}

/**
 * Returns if all plots must be written
 *
 * \return True if all possible plots must be written, false otherwise
 */
bool
ClusteringConfiguration::GetAllPlots(void)
{
  return this->AllPlots;
}


/**
 * Sets the definitions of different output plots
 * \param PlotsDefinitions Vector containing the definition of the output plots
 */
void
ClusteringConfiguration::SetPlotsDefinitions(vector<PlotDefinition*>& PlotsDefinitions)
{
  this->PlotsDefinitions = PlotsDefinitions;
}

/**
 * Returns the definitions of different output plots
 * \return Vector containing the definition of the output plots
 */
vector<PlotDefinition*>&
ClusteringConfiguration::GetPlotsDefinitions(void)
{
  return this->PlotsDefinitions;
}

/**
 * Sets the parsing error in the plot scripts definitions
 * \param PlotsDefinitionsError Boolean seting the possible plot scripts definitions parsing error 
 */
void
ClusteringConfiguration::SetPlotsDefinitionsError(bool PlotsDefinitionsError)
{
  this->PlotsDefinitionsError = PlotsDefinitionsError;
}

/**
 * Returns the error state of plot scripts definitions parsing
 * \return Boolean seting the possible plot scripts definitions parsing error
 */
bool
ClusteringConfiguration::GetPlotsDefinitionsError(void)
{
  return PlotsDefinitionsError;
}

/**
 * Sets the error description of a possible parsing error in the plot scripts definitions
 * \param PlotsDefinitionsErrorMessage String describing the possible error in plot scripts definitions parsing
 */
void
ClusteringConfiguration::SetPlotsDefinitionsErrorMessage(string PlotsDefinitionsMessage)
{
  this->PlotsDefinitionsErrorMessage = PlotsDefinitionsErrorMessage;
}

/**
 * Returns the error description of a possible parsing error in the plot scripts definitions
 * \return String describing the possible error in plot scripts definitions parsing
 */
string
ClusteringConfiguration::GetPlotsDefinitionsErrorMessage(void)
{
  return PlotsDefinitionsErrorMessage;
}
