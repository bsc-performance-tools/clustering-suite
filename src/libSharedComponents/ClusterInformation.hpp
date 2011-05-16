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

  $URL::                                          $:  File
  $Rev::                                          $:  Revision of last commit
  $Author::                                       $:  Author of last commit
  $Date::                                         $:  Date of last commit

\* -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=- */

#ifndef _CLUSTERINFORMATION_HPP_
#define _CLUSTERINFORMATION_HPP_

#include <Error.hpp>
using cepba_tools::Error;

#include <libClustering.hpp>
#include "trace_clustering_types.h"

#include <vector>
using std::vector;

typedef UINT32 node_id_t;

class ClusterInformation: public Error
{
  public:
    static node_id_t NodeIDNumber;
    
  private:
    UINT32                      NodeID;
    
    cluster_id_t                ID;

    bool                        Discarded;
    bool                        Reclassification;

    percentage_t                Score;
    size_t                      Occurrences;
    timestamp_t                 TotalDuration;
    size_t                      Individuals;

    vector<instance_t>          Instances;
    
    vector<ClusterInformation*> Children;

  public:
    ClusterInformation(cluster_id_t ID,
                       percentage_t Score,
                       size_t       Occurrences,
                       timestamp_t  TotalDuration,
                       size_t       Individuals);

    node_id_t    GetNodeID(void)        { return this->NodeID; };
    
    void         SetID(cluster_id_t ID) { this->ID = ID; };
    cluster_id_t GetID(void)            { return this->ID; };

    percentage_t GetScore(void)       { return Score; };
    size_t       GetOccurrences(void) { return Occurrences; };
    
    timestamp_t  GetTotalDuration(void) { return TotalDuration; };
    size_t       GetIndividuals(void)   { return Individuals; };

    bool IsCandidate(size_t      TotalOccurrences,
                     timestamp_t ClustersTotalDuration);

    void SetInstances(vector<instance_t>& Instances) { this->Instances = Instances; };
    vector<instance_t>& GetInstances(void)           { return this->Instances; };

    void AddInstance(instance_t Instance) { Reclassification = true;
                                            Individuals++;
                                            Instances.push_back(Instance); };

    bool AddChild(ClusterInformation*);

    vector<ClusterInformation*>& GetChildren(void) { return Children; };

    size_t TotalClusters(void);

    void RenameChildren(cluster_id_t& RestOfChildrenID);

    bool IsLeaf(void);
    
    bool IsDiscarded(void) { return this->Discarded; };
    
    void Discard(void) {
      /* DEBUG 
      cout << "NODE = " << ID << " IS BEING DISCARDED" << endl; */
      this->Discarded = true; };

    void SetReclassified(void)
    {
      Reclassification = true;
    }

    string NodeName(void);

    string NodeLabel(void);

    string Color(void);
};

#endif /* _CLUSTERINFORMATION_HPP_ */
