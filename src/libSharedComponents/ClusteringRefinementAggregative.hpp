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
#ifndef _CLUSTERINGREFINEMENTAGGREGATIVE_HPP_
#define _CLUSTERINGREFINEMENTAGGREGATIVE_HPP_

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

class ClusteringRefinementAggregative: public Error
{
  private:
    INT32          MinPoints;
    vector<double> EpsilonPerLevel;

    size_t         Steps;
    size_t         LastStep;

    bool           PrintStepsInformation;
    string         OutputFilePrefix;

    libClustering*                         ClusteringCore;

    map<instance_t, size_t>                Instance2Burst;
    map<instance_t, vector<cluster_id_t> > IDPerLevel;

    vector<ClusteringStatistics>           StatisticsHistory;
    vector<vector<ClusterInformation*> >   NodesPerLevel;

    cluster_id_t                           MaxIDAssigned;

    vector<percentage_t>                   GlobalScoresPerLevel;

  public:

    ClusteringRefinementAggregative(INT32          MinPoints,
                                    vector<double> EpsilonPerLevel);

    bool Run(const vector<CPUBurst*>& Bursts,
             vector<Partition>&       IntermediatePartitions,
             Partition&               LastPartition,
             bool                     PrintStepsInformation,
             string                   OutputFilePrefix = "");

  private:

    bool RunFirstAnalysis(const vector<CPUBurst*>& Bursts,
                          Partition&               FirstPartition);

    bool RunStep(size_t                   Step,
                 const vector<CPUBurst*>& Bursts,
                 Partition&               PreviousPartition,
                 Partition&               NewPartition,
                 bool&                    Stop);

    bool RunDBSCAN(const vector<const Point*>& CurrentData,
                   double                      Epsilon,
                   Partition&                  CurrentPartition);

    bool GenerateCandidatesAndBurstSubset(const vector<CPUBurst*>&     Bursts,
                                          vector<ClusterInformation*>& ParentNodes,
                                          vector<CPUBurst*>&           BurstsSubset,
                                          vector<ClusterInformation*>& NodesSubset);

    bool StopCondition(void);

    bool GenerateNodes(size_t                       Step,
                       const vector<CPUBurst*>&     Bursts,
                       Partition&                   CurrentPartition,
                       vector<ClusterInformation*>& Nodes,
                       bool                         LastPartition = false);

    void LinkNodes(const vector<CPUBurst*>&     BurstsSubset,
                   vector<ClusterInformation*>& Parent,
                   vector<ClusterInformation*>& Children,
                   Partition&                   PreviousPartition,
                   Partition&                   NewPartition);

    void GeneratePartition(Partition& NewPartition);

    bool ComputeScores(size_t                       Step,
                       const vector<CPUBurst*>&     Bursts,
                       vector<ClusterInformation*>& NewNodes,
                       Partition&                   CurrentPartition,
                       bool                         LastPartition);

    vector<pair<instance_t, cluster_id_t> > GetAssignment(ClusterInformation* Node);

    bool GenerateLastPartition(const vector<CPUBurst*>& Bursts,
                               size_t                   LastStep,
                               Partition&               PreviousPartition,
                               Partition&               LastPartition);

    void LinkLastNodes(cluster_id_t        MainClusterID,
                       set<cluster_id_t>&  Merges,
                       ClusterInformation* NewNode);

    ClusterInformation* LocateNode(cluster_id_t ClusterID);

    set<set<cluster_id_t> > getSubsets(set<cluster_id_t>&          in_set,
                                       set<cluster_id_t>::iterator index);

    bool PrintPlots(const vector<CPUBurst*>& Bursts,
                    Partition&               CurrentPartition,
                    size_t                   Step);

    bool PrintTrees(size_t Step,
                    bool   LastTree = false);

    bool PrintTreeNodes(ofstream& str);

    bool PrintTreeLinks(ofstream& str);



};

struct setSizeCmp
{
  bool operator()(set<cluster_id_t> s1, set<cluster_id_t> s2) const
  {
    if (s1.size() == s2.size())
      return (s1 < s2);
    else
      return s1.size() > s2.size();
  }
};

#endif // _CLUSTERINGREFINEMENTAGGREGATIVE_HPP_
