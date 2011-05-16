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

  $URL:: https://svn.bsc.es/repos/ClusteringSuite/trunk/src/libClustering/#$:

  $Rev:: 5                          $:  Revision of last commit
  $Author:: jgonzale                $:  Author of last commit
  $Date:: 2011-02-04 12:39:47 +0100#$:  Date of last commit

\* -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=- */
#ifndef _CLUSTERINGREFINEMENTDIVISIVE_HPP_
#define _CLUSTERINGREFINEMENTDIVISIVE_HPP_

#include <Error.hpp>
using cepba_tools::Error;

#include <libClustering.hpp>
#include "TraceData.hpp"
#include "Partition.hpp"
#include "ClusteringStatistics.hpp"
#include "ClusterInformation.hpp"

#include <list>
using std::list;

#include <set>
using std::set;

#include <fstream>
using std::ofstream;

class ClusteringRefinementDivisive: public Error
{
  private:
    INT32  MinPoints;
    
    double MinEpsilon;
    double MaxEpsilon;

    size_t Steps;
    size_t LastStep;

    string OutputFilePrefix;
    bool   PrintStepsInformation;
    
    libClustering*               ClusteringCore;

    map<instance_t, size_t> Instance2Burst;
    
    vector<ClusteringStatistics>         StatisticsHistory;
    vector<vector<ClusterInformation*> > NodesPerLevel;

  public:
    
    ClusteringRefinementDivisive(INT32      MinPoints,
                                 double     MaxEpsilon,
                                 double     MinEpsilon,
                                 size_t     Steps);

    bool Run(const vector<CPUBurst*>& Bursts,
             vector<Partition>&       IntermediatePartitions,
             Partition&               LastPartition,
             string                   OutputFilePrefix = "");

  private:
    
    bool IsSplitOK(ClusterInformation* Parent);
    
    vector<CPUBurst*> GenerateBurstsSubset(const vector<CPUBurst*>& Bursts,
                                           ClusterInformation*      Node);

    bool RunFirstStep(const vector<CPUBurst*>& Bursts,
                      double                   Epsilon,
                      Partition&               FirstPartition);

    bool RunStep(size_t                   Step,
                 const vector<CPUBurst*>& Bursts,
                 double                   Epsilon,
                 Partition&               PreviousPartition,
                 Partition&               NewPartition,
                 bool&                    Stop);
    
    bool RunDBSCAN(const vector<const Point*>& CurrentData,
                   double                      Epsilon,
                   Partition&                  CurrentPartition);

    bool GenerateNodes(const vector<CPUBurst*>&     Bursts,
                       Partition&                   CurrentPartition,
                       vector<ClusterInformation*>& Nodes);

    void GeneratePartition(Partition& NewPartition);

    vector<pair<instance_t, cluster_id_t> > GetAssignment(ClusterInformation* Node);

    size_t ColapseNonDividedSubtrees(ClusterInformation* Node);
    void   ReclassifyNoise(const vector<CPUBurst*>& Bursts, 
                           ClusterInformation*      Node,
                           size_t                   Level);

    bool IsIDInSet(cluster_id_t       ID,
                   set<cluster_id_t>& IDsSet);

    bool PrintPlots(const vector<CPUBurst*>& Bursts,
                    Partition&               CurrentPartition,
                    size_t                   Step);

    bool PrintTrees(size_t Level, bool LastTree = false);

    bool PrintTreeNodes(ofstream& str, ClusterInformation* Node);
    bool PrintTreeLinks(ofstream& str, ClusterInformation* Node);

};

#endif // _CLUSTERINGREFINEMENTDIVISIVE_HPP_
