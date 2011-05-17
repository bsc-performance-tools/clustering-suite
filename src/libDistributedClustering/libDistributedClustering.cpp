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
libDistributedClustering::libDistributedClustering(int verbose)
{
  Implementation  = new libDistributedClusteringImplementation(verbose);
  Error = Warning = false;
}

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


bool libDistributedClustering::ExtractData(string    InputFileName,
                                           set<int>& TasksToRead)
{
  if (!Implementation->ExtractData(InputFileName, TasksToRead))
  {
    Error        = true;
    ErrorMessage = Implementation->GetLastError();
    return true;
  }
  
  return true;
}

bool libDistributedClustering::ExtractData(string InputFileName)
{
  if (!Implementation->ExtractData(InputFileName))
  {
    Error        = true;
    ErrorMessage = Implementation->GetLastError();
    return true;
  }
  
  return true;
}

bool libDistributedClustering::ClusterAnalysis(vector<ConvexHullModel>& ClusterModels)
{
  if (!Implementation->ClusterAnalysis(ClusterModels))
  {
    Error        = true;
    ErrorMessage = Implementation->GetLastError();
    return false;
  }
  
  return true;
}

bool libDistributedClustering::PrintModels(vector<ConvexHullModel>& ClusterModels, 
                                           string                   ModelsFileName,
                                           string                   ScriptsFileNamePrefix)
{
  
  if (!Implementation->PrintModels(ClusterModels,
                                   ModelsFileName,
                                   ScriptsFileNamePrefix))
  {
    Error        = true;
    ErrorMessage = Implementation->GetLastError();
    return false;
  }
  return true;
}

bool libDistributedClustering::ClassifyData(vector<ConvexHullModel>& ClusterModels)
{
  if (!Implementation->ClassifyData(ClusterModels))
  {
    Error = true;
    ErrorMessage = Implementation->GetLastError();
    return false;
  }
  
  return true;
}

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

bool libDistributedClustering::PrintPlotScripts(string DataFileName,
                                                string ScriptsFileNamePrefix,
                                                bool   LocalPartition)
{
  if (!Root)
  {
    return true;
  }
  
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
 * Returns the string containing the last error message
 * \return Last error message
 */
string libDistributedClustering::GetErrorMessage(void)
{
  return ErrorMessage;
}

/**
 * Returns the string containing the last warning message
 * \return Last warning message
 */
string libDistributedClustering::GetWarningMessage(void)
{
  return WarningMessage;
}
