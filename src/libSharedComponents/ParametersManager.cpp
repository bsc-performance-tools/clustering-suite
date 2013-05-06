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


#include "ParametersManager.hpp"
#include "CPIStackModel.hpp"

#include <sstream>
using std::ostringstream;

#include <limits>
using std::numeric_limits;

/******************************************************************************
 * Singleton pointer
 *****************************************************************************/

/**
 * Singleton pointer
 */
ParametersManager* ParametersManager::_Parameters = NULL;

/******************************************************************************
 * GetInstance method
 *****************************************************************************/

/**
 * Returns the single instance of the parameter manager and creates it if doesn't
 * exists
 * \return The parameter manager singleton instance
 */
ParametersManager*
ParametersManager::GetInstance(void)
{
  if (ParametersManager::_Parameters == NULL)
  {
    ParametersManager::_Parameters = new ParametersManager();
  }

  return ParametersManager::_Parameters;
}

/******************************************************************************
 * Class private constructor
 *****************************************************************************/

/**
 * Creates the single instance of the class using the clustering configuration
 */
ParametersManager::ParametersManager(void)
{
  ClusteringConfiguration* Configuration;

  Configuration = ClusteringConfiguration::GetInstance();

  if (!Configuration->IsInitialized())
  {
    SetErrorMessage("clustering configuration not initilized");
    SetError(true);
    return;
  }

  if (Configuration->GetClusteringParametersError())
  {
    SetErrorMessage(Configuration->GetClusteringParametersErrorMessage());
    SetError(true);
    return;
  }

  if (!LoadParameters(Configuration))
  {
    SetError(true);
    return;
  }
}

/******************************************************************************
 * Information retrieval methods
 *****************************************************************************/

/**
 * Returns the number of clustering parameters
 * \return The number of clustering parameters
 */
size_t
ParametersManager::GetClusteringParametersSize(void)
{
  return ClusteringParameters.size();
}

/**
 * Returns the number of extrapolation parameters
 * \return The number of extrapolation parameters
 */
size_t
ParametersManager::GetExtrapolationParametersSize(void)
{
  return ExtrapolationParameters.size();
}

/**
 * Looks for a clustering parameter position based on the parameter name
 * \return Position of the clustering parameter if found. NOT_FOUND otherwise
 */
size_t
ParametersManager::GetClusteringParameterPosition(string ClusteringParameterName)
{
  map<string, INT32>::iterator Finder;

  Finder = ClusteringParametersIndex.find(ClusteringParameterName);

  if (Finder == ClusteringParametersIndex.end())
  {
    return NOT_FOUND;
  }
  else
  {
    return Finder->second;
  }
}

/**
 * Looks for a extrapolation parameter position based on the parameter name
 * \return Position of the clustering parameter if found. NOT_FOUND otherwise
 */
size_t
ParametersManager::GetExtrapolationParameterPosition(string ExtrapolationParameterName)
{
  map<string, INT32>::iterator Finder;

  Finder = ExtrapolationParametersIndex.find(ExtrapolationParameterName);

  if (Finder == ExtrapolationParametersIndex.end())
  {
    return NOT_FOUND;
  }
  else
  {
    return Finder->second;
  }
}

/**
 * Returns a vector with the names of all clustering parameterrs
 * \return A vector containig the clustering parameters names
 */
vector<string>
ParametersManager::GetClusteringParametersNames(void)
{
  vector<string> Result;

  for (INT32 i = 0; i < ClusteringParameters.size(); i++)
  {
    Result.push_back(ClusteringParameters[i]->GetParameterName());
  }

  return Result;
}

/**
 * Returns a vector with the names of all extrapolation paramters
 * \return A vector containig the extrapolation parameters names
 */
vector<string>
ParametersManager::GetExtrapolationParametersNames (void)
{
  vector<string> Result;

  for (size_t i = 0; i < ExtrapolationParameters.size(); i++)
  {
    Result.push_back(ExtrapolationParameters[i]->GetParameterName());
  }

  return Result;
}

/**
 * Returns the precision of the clustering parameters
 * \return A vector containing if the clustering parameters are high precision or not
 */
vector<bool>
ParametersManager::GetClusteringParametersPrecision (void)
{
  vector<bool> Result;

  for (size_t i = 0; i < ClusteringParameters.size(); i++)
  {
    Result.push_back(ClusteringParameters[i]->HighPrecision());
  }

  return Result;
}

/**
 * Returns the precision of the extrapolation parameters
 * \return A vector containing if the extrapolation parameters are high precision or not
 */
vector<bool>
ParametersManager::GetExtrapolationParametersPrecision (void)
{
  vector<bool> Result;

  for (size_t i = 0; i < ExtrapolationParameters.size(); i++)
  {
    Result.push_back(ExtrapolationParameters[i]->HighPrecision());
  }

  return Result;
}

/**
 * Returns the factors associated to each clustering parameter
 * \return A vector containing if the factors of the clustering parameters
 */
vector<double> ParametersManager::GetClusteringParametersFactors(void)
{
  vector<double> Result;

  for (size_t i = 0; i < ClusteringParameters.size(); i++)
  {
    Result.push_back(ClusteringParameters[i]->GetFactor());
  }

  return Result;
}

/******************************************************************************
 * Data Processing
 *****************************************************************************/

/**
 * Clear the contents of all parameters
 */
void
ParametersManager::Clear(void)
{
  for (INT32 i = 0; i < ClusteringParameters.size(); i++)
  {
    ClusteringParameters[i]->Clear();
  }

  for (INT32 i = 0; i < ExtrapolationParameters.size(); i++)
  {
    ExtrapolationParameters[i]->Clear();
  }
}

/**
 * Fills parameters with data from current burst
 * \param EventsData Map containing 'type/value' pairs read from a trace
 */
void
ParametersManager::NewData(map<event_type_t, event_value_t>& EventsData)
{
  map<event_type_t, event_value_t>::iterator DataIterator;

  for (DataIterator  = EventsData.begin();
       DataIterator != EventsData.end();
       DataIterator++)
  {
    event_type_t  EventType  = DataIterator->first;
    event_value_t EventValue = DataIterator->second;

    for (INT32 i = 0; i < ClusteringParameters.size(); i++)
    {
      ClusteringParameters[i]->NewData(EventType, EventValue);
    }

    for (INT32 i = 0; i < ExtrapolationParameters.size(); i++)
    {
      ExtrapolationParameters[i]->NewData(EventType, EventValue);
    }
  }
  return;
}

/**
 * Gathers all data collected and returns it in different containers.
 * \param RawClusteringData Vector of raw values of different clustering parameters
 * \param ProcessedClusteringData Vector of logarithmic Normalization of different clustering parameters
 * \param ExtrapolationData Map of extrapolation parameters captured
 * \return 'point_type_t' expressing if all data is present or if it has been filtered for any reason
 */

burst_type_t ParametersManager::GetData(vector<double>&      RawClusteringData,
                                        vector<double>&      ProcessedClusteringData,
                                        map<size_t, double>& ExtrapolationData)
{
  burst_type_t Result = CompleteBurst;

  for (size_t i = 0; i < ClusteringParameters.size(); i++)
  {
    if (ClusteringParameters[i]->IsReady())
    {
      RawClusteringData.push_back(ClusteringParameters[i]->GetRawMetric());
      ProcessedClusteringData.push_back(ClusteringParameters[i]->GetMetric());

      if (ClusteringParameters[i]->IsRangeFiltered())
      {
        Result = RangeFilteredBurst;
      }
    }
    else
    {
      RawClusteringData.push_back(numeric_limits<double>::min());
      Result = MissingDataBurst;
    }
  }

  for (size_t i = 0; i < ExtrapolationParameters.size(); i++)
  {
    if (ExtrapolationParameters[i]->IsReady())
    {
      ExtrapolationData[i] = ExtrapolationParameters[i]->GetRawMetric();
    }
  }

  return Result;
}

/******************************************************************************
 * Parameters loader
 *****************************************************************************/

/**
 * Loads all information read from XML and creates the parameters objects
 */
bool
ParametersManager::LoadParameters(ClusteringConfiguration* Configuration)
{
  SingleEvent* NewSingleEvent;
  MixedEvents* NewMixedEvent;

  vector<string>& ClusteringParametersNames =
    Configuration->GetClusteringParametersNames();

  vector<ParameterContainer>& ClusteringParametersDefinitions =
    Configuration->GetClusteringParametersDefinitions();

  vector<string>& ExtrapolationParametersNames =
    Configuration->GetExtrapolationParametersNames();

  vector<ParameterContainer>& ExtrapolationParametersDefinitions =
    Configuration->GetExtrapolationParametersDefinitions();

  if (Configuration->GetClusteringParametersError())
  {
    SetError(true);
    SetErrorMessage(Configuration->GetClusteringParametersErrorMessage());
    return false;
  }

  /* Clustering parameters creation */
  for (size_t i = 0; i < ClusteringParametersDefinitions.size(); i++)
  {
    if (GetClusteringParameterPosition(ClusteringParametersNames[i]) != NOT_FOUND)
    {
      string ErrorMessage;
      SetError(true);

      ErrorMessage += "clustering parameter \"";
      ErrorMessage += ClusteringParametersNames[i];
      ErrorMessage += "\" duplicated";

      SetErrorMessage(ErrorMessage);
      return false;
    }
    else
    {
      ClusteringParameter* NewParameter;

      NewParameter = CreateParameter(ClusteringParametersNames[i],
                                     ClusteringParametersDefinitions[i]);
      if (NewParameter == NULL)
      {
        return false;
      }
      else
      {
        /* Add parameter index */
        ClusteringParametersIndex[ClusteringParametersNames[i]] = ClusteringParameters.size();

        ClusteringParameters.push_back(NewParameter);
      }
    }
  }

  /* OUTDATED. Now CPI stack must be computed using prepared XMLs
  if (Configuration->GetApplyCPIStack())
  {
    /* Warning message
    if (ExtrapolationParametersDefinitions.size() != 0)
    {
      SetWarning(true);
      SetWarningMessage("using CPI stack parameters. XML extrapolation parameters will be discarded");
    }
    LoadCPIStackExtrapolationParameters();
    return true;
  }
  */

  if (!Configuration->GetExtrapolationParametersError())
  {
    /* Extrapolation parameters creation */
    for (size_t i = 0; i < ExtrapolationParametersDefinitions.size(); i++)
    {
      if (GetExtrapolationParameterPosition(ExtrapolationParametersNames[i]) != NOT_FOUND)
      {
        string WarningMessage;
        SetWarning(true);

        WarningMessage += "extrapolation parameter \"";
        WarningMessage += ExtrapolationParametersNames[i];
        WarningMessage += "\" duplicated";

        SetWarningMessage(WarningMessage);
      }
      else
      {
        ClusteringParameter* NewParameter;

        NewParameter = CreateParameter (ExtrapolationParametersNames[i],
                                        ExtrapolationParametersDefinitions[i]);
        if (NewParameter == NULL)
        {
          SetError(false);
          SetWarning(true);
          SetWarningMessage(GetLastError());
        }
        else
        {
          /* Add parameter index */
          ExtrapolationParametersIndex[ExtrapolationParametersNames[i]] = ExtrapolationParameters.size();
          ExtrapolationParameters.push_back(NewParameter);
        }
      }
    }
  }
  else
  {
    SetWarning(true);
    SetWarningMessage("extrapolation parameters from XML not used",
                      Configuration->GetExtrapolationParametersErrorMessage());
  }

  return true;
}

/**
 * Loads a single event parameter from a container
 * \param ParameterDefinition Container of the parameter definition read from the XML
 * \return Generic parameter pointer to the new single event parameter
 */
ClusteringParameter*
ParametersManager::CreateParameter(string              ParameterName,
                                   ParameterContainer& ParameterDefinition)
{
  SingleEvent* NewSingleEvent;
  MixedEvents* NewMixedEvent;

  switch(ParameterDefinition.ParameterType)
  {
    case SingleEventParameter:

      NewSingleEvent = new SingleEvent(ParameterName,
                                       ParameterDefinition.Factor,
                                       ParameterDefinition.ApplyLog,
                                       ParameterDefinition.EventType,
                                       ParameterDefinition.RangeMin,
                                       ParameterDefinition.RangeMax);
      return NewSingleEvent;

    case DerivedEventParameter:

      derived_event_op_t Operation;
      Operation = GetOperation(ParameterDefinition.Operation);

      if (Operation == UndefinedOperation)
      {
        ostringstream ErrorMessage;
        ErrorMessage << "wrong operation for parameter \"" << ParameterName;
        ErrorMessage << "\" defined on line " << ParameterDefinition.Line;
        SetErrorMessage(ErrorMessage.str());

        return (ClusteringParameter*) NULL;
      }
      else
      {
        NewMixedEvent = new MixedEvents(ParameterName,
                                        ParameterDefinition.Factor,
                                        ParameterDefinition.ApplyLog,
                                        ParameterDefinition.EventType,
                                        ParameterDefinition.EventType2,
                                        Operation,
                                        ParameterDefinition.RangeMin,
                                        ParameterDefinition.RangeMax);

        return NewMixedEvent;
      }
      break;
    default:
      ostringstream ErrorMessage;
      ErrorMessage << "wrong parameter type \"" << ParameterName;
      ErrorMessage << "\" defined on line " << ParameterDefinition.Line;
      SetErrorMessage(ErrorMessage.str());

      return (ClusteringParameter*) NULL;
  }
}

/**
 * Checks the operation type from the char defined on the XML to the inner representation
 * \param Operation The character contained on the XML
 * \return The inner representation of the operation, defined in 'ClusteringTypes.h'
 */
derived_event_op_t
ParametersManager::GetOperation(char Operation)
{
  switch (Operation)
  {
    case '+':
      return Add;
    case '-':
      return Substract;
    case '*':
      return Multiply;
    case '/':
      return Divide;
    default:
      return UndefinedOperation;
  }
}

/**
 * Loads the predifined extrapolation parameters set for the PPC970 processor
 */
void
ParametersManager::LoadCPIStackExtrapolationParameters(void)
{
  SingleEvent* CurrentEvent;

  CurrentEvent = new SingleEvent(PM_CYC_NAME, 1.0, false, PM_CYC_VAL);
  ExtrapolationParameters.push_back(CurrentEvent);

  CurrentEvent = new SingleEvent(PM_GRP_CMPL_NAME, 1.0, false, PM_GRP_CMPL_VAL);
  ExtrapolationParameters.push_back(CurrentEvent);

  CurrentEvent = new SingleEvent(PM_GCT_EMPTY_CYC_NAME, 1.0, false, PM_GCT_EMPTY_CYC_VAL);
  ExtrapolationParameters.push_back(CurrentEvent);

  CurrentEvent = new SingleEvent(PM_GCT_EMPTY_IC_MISS_NAME, 1.0, false, PM_GCT_EMPTY_IC_MISS_VAL);
  ExtrapolationParameters.push_back(CurrentEvent);

  CurrentEvent = new SingleEvent(PM_GCT_EMPTY_BR_MPRED_NAME, 1.0, false, PM_GCT_EMPTY_BR_MPRED_VAL);
  ExtrapolationParameters.push_back(CurrentEvent);

  CurrentEvent = new SingleEvent(PM_CMPLU_STALL_LSU_NAME, 1.0, false, PM_CMPLU_STALL_LSU_VAL);
  ExtrapolationParameters.push_back(CurrentEvent);

  CurrentEvent = new SingleEvent(PM_CMPLU_STALL_REJECT_NAME, 1.0, false, PM_CMPLU_STALL_REJECT_VAL);
  ExtrapolationParameters.push_back(CurrentEvent);

  CurrentEvent = new SingleEvent(PM_CMPLU_STALL_ERAT_MISS_NAME, 1.0, false, PM_CMPLU_STALL_ERAT_MISS_VAL);
  ExtrapolationParameters.push_back(CurrentEvent);

  CurrentEvent = new SingleEvent(PM_CMPLU_STALL_DCACHE_MISS_NAME, 1.0, false, PM_CMPLU_STALL_DCACHE_MISS_VAL);
  ExtrapolationParameters.push_back(CurrentEvent);

  CurrentEvent = new SingleEvent(PM_CMPLU_STALL_FXU_NAME, 1.0, false, PM_CMPLU_STALL_FXU_VAL);
  ExtrapolationParameters.push_back(CurrentEvent);

  CurrentEvent = new SingleEvent(PM_CMPLU_STALL_DIV_NAME, 1.0, false, PM_CMPLU_STALL_DIV_VAL);
  ExtrapolationParameters.push_back(CurrentEvent);

  CurrentEvent = new SingleEvent(PM_CMPLU_STALL_FPU_NAME, 1.0, false, PM_CMPLU_STALL_FPU_VAL);
  ExtrapolationParameters.push_back(CurrentEvent);

  CurrentEvent = new SingleEvent(PM_CMPLU_STALL_FDIV_NAME, 1.0, false, PM_CMPLU_STALL_FDIV_VAL);
  ExtrapolationParameters.push_back(CurrentEvent);

  CurrentEvent = new SingleEvent(PM_CMPLU_STALL_OTHER_NAME, 1.0, false, PM_CMPLU_STALL_OTHER_VAL);
  ExtrapolationParameters.push_back(CurrentEvent);

  CurrentEvent = new SingleEvent(PM_INST_CMPL_NAME, 1.0, false, PM_INST_CMPL_VAL);
  ExtrapolationParameters.push_back(CurrentEvent);

  CurrentEvent = new SingleEvent(PM_INST_DISP_NAME, 1.0, false, PM_INST_DISP_VAL);
  ExtrapolationParameters.push_back(CurrentEvent);

  CurrentEvent = new SingleEvent(PM_LD_REF_L1_NAME, 1.0, false, PM_LD_REF_L1_VAL);
  ExtrapolationParameters.push_back(CurrentEvent);

  CurrentEvent = new SingleEvent(PM_ST_REF_L1_NAME, 1.0, false, PM_ST_REF_L1_VAL);
  ExtrapolationParameters.push_back(CurrentEvent);

  CurrentEvent = new SingleEvent(PM_LD_MISS_L1_NAME, 1.0, false, PM_LD_MISS_L1_VAL);
  ExtrapolationParameters.push_back(CurrentEvent);

  CurrentEvent = new SingleEvent(PM_ST_MISS_L1_NAME, 1.0, false, PM_ST_MISS_L1_VAL);
  ExtrapolationParameters.push_back(CurrentEvent);

  CurrentEvent = new SingleEvent(PM_LD_MISS_L1_LSU0_NAME, 1.0, false, PM_LD_MISS_L1_LSU0_VAL);
  ExtrapolationParameters.push_back(CurrentEvent);

  CurrentEvent = new SingleEvent(PM_LD_MISS_L1_LSU1_NAME, 1.0, false, PM_LD_MISS_L1_LSU1_VAL);
  ExtrapolationParameters.push_back(CurrentEvent);

  CurrentEvent = new SingleEvent(PM_L1_WRITE_CYC_NAME, 1.0, false, PM_L1_WRITE_CYC_VAL);
  ExtrapolationParameters.push_back(CurrentEvent);

  CurrentEvent = new SingleEvent(PM_DATA_FROM_MEM_NAME, 1.0, false, PM_DATA_FROM_MEM_VAL);
  ExtrapolationParameters.push_back(CurrentEvent);

  CurrentEvent = new SingleEvent(PM_DATA_FROM_L2_NAME, 1.0, false, PM_DATA_FROM_L2_VAL);
  ExtrapolationParameters.push_back(CurrentEvent);

  CurrentEvent = new SingleEvent(PM_DTLB_MISS_NAME, 1.0, false, PM_DTLB_MISS_VAL);
  ExtrapolationParameters.push_back(CurrentEvent);

  CurrentEvent = new SingleEvent(PM_ITLB_MISS_NAME, 1.0, false, PM_ITLB_MISS_VAL);
  ExtrapolationParameters.push_back(CurrentEvent);

  CurrentEvent = new SingleEvent(PM_LSU0_BUSY_NAME, 1.0, false, PM_LSU0_BUSY_VAL);
  ExtrapolationParameters.push_back(CurrentEvent);

  CurrentEvent = new SingleEvent(PM_LSU1_BUSY_NAME, 1.0, false, PM_LSU1_BUSY_VAL);
  ExtrapolationParameters.push_back(CurrentEvent);

  CurrentEvent = new SingleEvent(PM_LSU_FLUSH_NAME, 1.0, false, PM_LSU_FLUSH_VAL);
  ExtrapolationParameters.push_back(CurrentEvent);

  CurrentEvent = new SingleEvent(PM_LSU_LDF_NAME, 1.0, false, PM_LSU_LDF_VAL);
  ExtrapolationParameters.push_back(CurrentEvent);

  CurrentEvent = new SingleEvent(PM_FPU_FIN_NAME, 1.0, false, PM_FPU_FIN_VAL);
  ExtrapolationParameters.push_back(CurrentEvent);

  CurrentEvent = new SingleEvent(PM_FPU_FSQRT_NAME, 1.0, false, PM_FPU_FSQRT_VAL);
  ExtrapolationParameters.push_back(CurrentEvent);

  CurrentEvent = new SingleEvent(PM_FPU_FDIV_NAME, 1.0, false, PM_FPU_FDIV_VAL);
  ExtrapolationParameters.push_back(CurrentEvent);

  CurrentEvent = new SingleEvent(PM_FPU_FMA_NAME, 1.0, false, PM_FPU_FMA_VAL);
  ExtrapolationParameters.push_back(CurrentEvent);

  CurrentEvent = new SingleEvent(PM_FPU0_FIN_NAME, 1.0, false, PM_FPU0_FIN_VAL);
  ExtrapolationParameters.push_back(CurrentEvent);

  CurrentEvent = new SingleEvent(PM_FPU1_FIN_NAME, 1.0, false, PM_FPU1_FIN_VAL);
  ExtrapolationParameters.push_back(CurrentEvent);

  CurrentEvent = new SingleEvent(PM_FPU_STF_NAME, 1.0, false, PM_FPU_STF_VAL);
  ExtrapolationParameters.push_back(CurrentEvent);

  CurrentEvent = new SingleEvent(PM_LSU_LMQ_SRQ_EMPTY_CYC_NAME, 1.0, false, PM_LSU_LMQ_SRQ_EMPTY_CYC_VAL);
  ExtrapolationParameters.push_back(CurrentEvent);

  CurrentEvent = new SingleEvent(PM_HV_CYC_NAME, 1.0, false, PM_HV_CYC_VAL);
  ExtrapolationParameters.push_back(CurrentEvent);

  CurrentEvent = new SingleEvent(PM_1PLUS_PPC_CMPL_NAME, 1.0, false, PM_1PLUS_PPC_CMPL_VAL);
  ExtrapolationParameters.push_back(CurrentEvent);

  CurrentEvent = new SingleEvent(PM_TB_BIT_TRANS_NAME, 1.0, false, PM_TB_BIT_TRANS_VAL);
  ExtrapolationParameters.push_back(CurrentEvent);

  CurrentEvent = new SingleEvent(PM_FLUSH_BR_MPRED_NAME, 1.0, false, PM_FLUSH_BR_MPRED_VAL);
  ExtrapolationParameters.push_back(CurrentEvent);

  CurrentEvent = new SingleEvent(PM_BR_MPRED_TA_NAME, 1.0, false, PM_BR_MPRED_TA_VAL);
  ExtrapolationParameters.push_back(CurrentEvent);

  CurrentEvent = new SingleEvent(PM_GCT_EMPTY_SRQ_FULL_NAME, 1.0, false, PM_GCT_EMPTY_SRQ_FULL_VAL);
  ExtrapolationParameters.push_back(CurrentEvent);

  CurrentEvent = new SingleEvent(PM_FXU_FIN_NAME, 1.0, false, PM_FXU_FIN_VAL);
  ExtrapolationParameters.push_back(CurrentEvent);

  CurrentEvent = new SingleEvent(PM_FXU_BUSY_NAME, 1.0, false, PM_FXU_BUSY_VAL);
  ExtrapolationParameters.push_back(CurrentEvent);

  CurrentEvent = new SingleEvent(PM_IOPS_CMPL_NAME, 1.0, false, PM_IOPS_CMPL_VAL);
  ExtrapolationParameters.push_back(CurrentEvent);

  CurrentEvent = new SingleEvent(PM_GCT_FULL_CYC_NAME, 1.0, false, PM_GCT_FULL_CYC_VAL);
  ExtrapolationParameters.push_back(CurrentEvent);

}
