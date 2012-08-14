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
#include <Point.hpp>
#include "HullModel.hpp"

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
    libDistributedClustering(int         verbose,
                             const char *msg_prefix);

    /* This methods are intended to be used in the MRNet BE nodes, to perform
     * a local cluster analysis */

    bool InitClustering(string ClusteringDefinitionXML,
                        double Epsilon,
                        int    MinPoints,
                        bool   Root,
                        int    MyRank,
                        int    TotalRanks);

    bool InitClustering(string ClusteringDefinitionXML,
                        bool   Root,
                        int    MyRank,
                        int    TotalRanks);

    double    GetEpsilon(void);

    int       GetMinPoints(void);

    bool ExtractData(string InputFileName, set<int>& TasksToRead);

    bool ExtractData(string InputFileName);

    size_t GetNumberOfPoints(void);

    bool ClusterAnalysis(vector<HullModel*>& ClusterModels);

    bool ClassifyData(vector<HullModel*>& ClusterModels);

    bool FlushClustersInformation(string OutputClustersInfoFileName);

    bool ReconstructInputTrace(string OutputTraceName);


    /* This second interface is focused to implement the cluster of local noise
     * points */

    bool InitClustering(double Epsilon,
                        int    MinPoints);

    bool ClusterAnalysis(const vector<const Point*> &Points,
                         vector<HullModel*>         &ClusterModels);

    bool GetNoisePoints(vector<const Point*>& NoisePoints);

    /* A method to retrieve all information to perform the cross-process
     * analysis */

    bool GetFullBurstsInformation(vector<Point*>       &Points,
                                  vector<task_id_t>    &TaskIDs,
                                  vector<thread_id_t>  &ThreadIDs,
                                  vector<cluster_id_t> &ClusterIDs);

    bool GetClusterIDs(vector<cluster_id_t> &ClusterIDs);


    /* Methods to print the models and the scatter plots of data. To be used
     * in BE nodes */
    bool PrintModels(vector<HullModel*> &ClusterModels,
                     string              ModelsFileName,
                     string              ScriptsFileNamePrefix = "",
                     string              PlotTitle             = "");

    bool PrintPlotScripts(string DataFileName,
                          string ScriptsFileNamePrefix = "",
                          bool   LocalPartition = false);


    /* Error/Warning retrieveng methods */
    string GetErrorMessage(void);

    string GetWarningMessage(void);

protected:

private:

};

#endif // _LIB_DISTRIBUTED_CLUSTERING_HPP_
