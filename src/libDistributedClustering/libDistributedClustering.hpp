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

#ifndef _LIB_DISTRIBUTED_CLUSTERING_HPP_
#define _LIB_DISTRIBUTED_CLUSTERING_HPP_

#include <trace_clustering_types.h>
#include "ConvexHullModel.hpp"

#define SILENT   0
#define VERBOSE  1
#define VVERBOSE 2

#include <vector>
using std::vector;
#include <set>
using std::set;



class libDistributedClusteringImplementation;

class libDistributedClustering
{
  private:
    libDistributedClusteringImplementation* Implementation;

    bool   Error,        Warning;
    string ErrorMessage, WarningMessage;

    bool   Root;
    
    /*
    bool                  ClusteringExecuted;
    bool                  HWCGroupSet;
    vector<INT32>         CurrentHWCGroup;
    bool                  StoreClassificationPoints;
    */

  public:
    libDistributedClustering(int verbose);

    bool InitClustering(string ClusteringDefinitionXML,
                        double Epsilon,
                        int    MinPoints,
                        bool   Root,
                        int    MyRank,
                        int    TotalRanks);

    bool ExtractData(string InputFileName, set<int>& TasksToRead);

    bool ExtractData(string InputFileName);

    bool ClusterAnalysis(vector<ConvexHullModel>& ClusterModels);
    
    bool ClassifyData(vector<ConvexHullModel>& ClusterModels);

    bool ReconstructInputTrace(string OutputTraceName);

    bool PrintModels(vector<ConvexHullModel>& ClusterModels,
                     string ModelsFileName,
                     string ScriptsFileNamePrefix = "");
    
    bool PrintPlotScripts(string DataFileName,
                          string ScriptsFileNamePrefix = "",
                          bool   LocalPartition = false);

    string GetErrorMessage(void);

    string GetWarningMessage(void);

protected:

private:

};

#endif // _LIB_DISTRIBUTED_CLUSTERING_HPP_
