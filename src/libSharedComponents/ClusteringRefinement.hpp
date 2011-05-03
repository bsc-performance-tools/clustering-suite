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
#ifndef _CLUSTERINGREFINEMENT_HPP_
#define _CLUSTERINGREFINEMENT_HPP_

#include <Error.hpp>
using cepba_tools::Error;

#include <libClustering.hpp>
#include "TraceData.hpp"
#include "Partition.hpp"
#include "ClusteringStatistics.hpp"

#include <list>
using std::list;

#include <set>
using std::set;

#include <fstream>
using std::ofstream;

class ClusterInformation: public Error
{
  private:
    cluster_id_t              ID;

    bool                      Discarded;

    percentage_t              Score;
    size_t                    Occurrences;

    timestamp_t               TotalDuration;
    size_t                    Individuals;
    
    vector<ClusterInformation*> Children;

  public:
    ClusterInformation(cluster_id_t ID,
                       percentage_t Score,
                       size_t       Occurrences,
                       timestamp_t  TotalDuration,
                       size_t       Individuals);

    cluster_id_t GetID(void) { return ID; };

    percentage_t GetScore(void)       { return Score; };
    size_t       GetOccurrences(void) { return Occurrences; };
    
    timestamp_t  GetTotalDuration(void) { return TotalDuration; };
    size_t       GetIndividuals(void)   { return Individuals; };

    bool IsCandidate(size_t      TotalOccurrences,
                     timestamp_t ClustersTotalDuration);
    
    bool AddChild(ClusterInformation*);

    vector<ClusterInformation*>& GetChildren(void) { return Children; };

    bool IsDiscarded(void) { return this->Discarded; };
    
    void Discard(void)     { 
      /* DEBUG */
      cout << "NODE = " << ID << " IS BEING DISCARDED" << endl;
      this->Discarded = true; };

};

class ClusteringRefinement: public Error
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
    vector<Partition>            PartitionsHistory;
    vector<ClusteringStatistics> StatisticsHistory;

    vector<vector<CPUBurst*> >        BurstsPerStep;
    map<instance_t, vector<size_t> >  InstancesTracking;

    vector<ClusterInformation*>            OngoingCandidates;
    map<cluster_id_t, ClusterInformation*> NewNodes;
    
    vector<set<ClusterInformation*> >   ClustersHierarchy;

  public:
    
    ClusteringRefinement(INT32      MinPoints,
                         double     MaxEpsilon,
                         double     MinEpsilon,
                         size_t     Steps);

    bool Run(const vector<CPUBurst*>& Bursts,
             vector<Partition>&       ResultingPartitions,
             string                   OutputFilePrefix = "");

  private:

    /*
    vector<CPUBurst*> GetBurstsSubset(vector<CPUBurst*>&   ClusteringBursts,
                                      Partition&           CurrentPartition,
                                      vector<cluster_id_t> Candidates);
    */

    bool RunStep(size_t Step,
                 double Epsilon,
                 bool&  NoMoreClusters);

    bool LinkToPreviuosStep(size_t Step,
                            bool&  Convergence);
    
    bool RunDBSCAN(const vector<const Point*>& CurrentData,
                   double                      Epsilon,
                   Partition&                  CurrentPartition);

    bool GenerateCandidates(size_t Step);
    
    bool GenerateCurrentLevelNodes(size_t Step);

    bool EvaluateSplit(ClusterInformation*          Parent,
                       vector<ClusterInformation*>& Children,
                       bool&                        SplitOK);

    void ReassignNoisePartition(cluster_id_t ID,
                                size_t Step);

    bool CreateDefinitivePartitions(vector<Partition>& ResultingPartitions);

    size_t ColapseNonDividedSubtrees(ClusterInformation* Node, size_t Level);

    bool IsIDInSet(cluster_id_t       ID,
                   set<cluster_id_t>& IDsSet);

    bool PrintPlots(vector<CPUBurst*>& Bursts,
                    Partition&         CurrentPartition,
                    size_t             Step);

    bool PrintTrees(void);

    bool PrintNode(ofstream& str, ClusterInformation* Node, size_t Level);

};

#endif // _CLUSTERINGREFINEMENT_HPP_
