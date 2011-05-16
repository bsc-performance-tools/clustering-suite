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


#include <SystemMessages.hpp>
using cepba_tools::system_messages;

#include "ClusteringRefinementDivisive.hpp"
#include "SequenceScore.hpp"
#include "PlottingManager.hpp"
#include <DBSCAN.hpp>

#include <sstream>
using std::ostringstream;
using std::setfill;
using std::setw;
using std::setprecision;


/******************************************************************************
 * CLASS 'ClusteringRefinementDivisive'
 *****************************************************************************/

/**
 * Parametrized constructor.
 *
 * \param MinPoints MinPoints value to be used in all DBSCAN runs
 * \param MaxEpsilon Maximum value of Epsilon 
 * \param MinEpsilon Minimum value of Epislon
 * \param Steps Number of refinement steps
 *
 */ 
ClusteringRefinementDivisive::ClusteringRefinementDivisive(INT32      MinPoints,
                                           double     MaxEpsilon,
                                           double     MinEpsilon,
                                           size_t     Steps)
{
  this->MinPoints  = MinPoints;
  this->MinEpsilon = MinEpsilon;
  this->MaxEpsilon = MaxEpsilon;
  this->Steps      = Steps;
}


/**
 * Executes the refinement analysis
 *
 * \param OutputFilePrefix If different of empty string, it indicates that
 *        every step should be printed
 *
 * \return True if the refinement worked properly, false otherwise
 *
 */ 
bool ClusteringRefinementDivisive::Run(const vector<CPUBurst*>& Bursts,
                                       vector<Partition>&       IntermediatePartitions,
                                       Partition&               LastPartition,
                                       string                   OutputFilePrefix)
{
  bool           Stop = false;
  double         StepSize    = (MaxEpsilon - MinEpsilon)/(Steps-1);
  vector<double> Epsilons;

  ostringstream  Messages;

  this->OutputFilePrefix = OutputFilePrefix;
  
  if (OutputFilePrefix.compare("") == 0)
  {
    PrintStepsInformation = false;
  }
  else
  {
    PrintStepsInformation = true;
  }

  if (Bursts.size() == 0)
  {
    SetErrorMessage("no bursts no analyze");
    return false;
  }
  
  if (Steps <= 1)
  {
    SetErrorMessage("number of steps in a refinement should be higher than 1");
    return false;
  }

  for (size_t i = 0; i < Bursts.size(); i++)
  {
    Instance2Burst[Bursts[i]->GetInstance()] = i;
  }

  IntermediatePartitions.clear();
  IntermediatePartitions.push_back(Partition());

  StatisticsHistory.clear();
  StatisticsHistory.push_back(ClusteringStatistics());
  
  NodesPerLevel.clear();
  NodesPerLevel.push_back(vector<ClusterInformation*>(0));
  
  Epsilons.push_back(MaxEpsilon);
  for (size_t i = 1; i < Steps; i++)
  {
    Epsilons.push_back(Epsilons[i-1] - StepSize);
  }

  ClusteringCore = new libClustering();

  Messages << "*** TOTAL NUMBER OF BURSTS = " << Bursts.size() << " ***" << endl;
  Messages << "*** STEP 1 (Eps = " << Epsilons[0] << ") ***" << endl;
  system_messages::information(Messages.str());  
  if (!RunFirstStep(Bursts, Epsilons[0], IntermediatePartitions[0]))
  {
    return false;
  }

  /* On step 0, all new nodes are part of the hierarchy */
  map<cluster_id_t, ClusterInformation*>::iterator NewNodesIt;

  LastStep = 0;

  /* Iterate through the epsilon values, and generate the clusters hierarchy */
  for (size_t CurrentStep = 1; 
       CurrentStep < Steps && !Stop;
       CurrentStep++)
  {
    double CurrentEpsilon = Epsilons[CurrentStep];

    Messages.str("");
    
    Messages << "****** STEP " << (CurrentStep+1) << " (Eps = " << CurrentEpsilon;
    Messages << ") ******" << endl;
    system_messages::information(Messages.str());

    IntermediatePartitions.push_back(Partition());
    StatisticsHistory.push_back(ClusteringStatistics());
    NodesPerLevel.push_back(vector<ClusterInformation*>(0));
    
    Stop = true;
    if (!RunStep(CurrentStep,
                 Bursts,
                 CurrentEpsilon,
                 IntermediatePartitions[CurrentStep-1],
                 IntermediatePartitions[CurrentStep],
                 Stop))
    {
      return false;
    }

    if (!Stop)
    {
      LastStep++;
    }
    else
    {
      Messages.str("");
      Messages << "****** CONVERGENCE ******" << endl;
      system_messages::information(Messages.str());
    }
  }

  /* Prune non-expanded nodes and reclassify noise points at the leaves */
  Messages.str("");
  Messages << "****** PRUNING TREE ******" << endl;
  system_messages::information(Messages.str());
  
  for (size_t i = 0; i < NodesPerLevel[0].size(); i++)
  {
    if (NodesPerLevel[0][i]->GetID() != NOISE_CLUSTERID)
    {
      ColapseNonDividedSubtrees(NodesPerLevel[0][i]);
      ReclassifyNoise (Bursts, NodesPerLevel[0][i], 0);
    }
  }

  /* Generate last partition */
  GeneratePartition(LastPartition);

  /* DEBUG 
  cout << "Last Partition has " << LastPartition.NumberOfClusters() << " clusters" << endl; */
  
  if (PrintStepsInformation)
  {
    Messages.str("");
    Messages << "|-> Printing trees" << endl;
    system_messages::information(Messages.str());

    if (!PrintTrees(0, true)) /* Last tree */
    {
      return false;
    }
  }
  
  // PrintTrees ();
  
  return true;
}

/**
 * Executes the first analysis of the hirearchy
 *
 * \param Epsilon Value of epsilon to be used
 *
 * \return True if the analysis was executed correctly, false otherwise
 *
 */
bool ClusteringRefinementDivisive::RunFirstStep(const vector<CPUBurst*>& Bursts,
                                                double                   Epsilon,
                                                Partition&               FirstPartition)
{
  ostringstream        Messages;
  ClusteringStatistics Statistics;

  Messages.str("");
  Messages << "-> Running DBSCAN for top level" << endl;
  system_messages::information(Messages.str());
  
  if (!RunDBSCAN ((vector<const Point*>&) Bursts,
                                          Epsilon,
                                          FirstPartition))
  {
    return false;
  }

  if (!GenerateNodes(Bursts,
                     FirstPartition,
                     NodesPerLevel[0]))
  {
    return false;
  }

  /* Print plots */
  if (PrintStepsInformation)
  {
    Messages.str("");
    Messages << "|-> Printing trees" << endl;
    system_messages::information(Messages.str());

    if (!PrintTrees(0))
    {
      return false;
    }

    Messages.str("");
    Messages << "|-> Printing plots" << endl;
    system_messages::information(Messages.str());
    if (!PrintPlots(Bursts, FirstPartition, 0))
    {
      return false;
    }
  }
  
  return true;
}

/**
 * Executes the i-th step in the refinement analysis
 *
 * \param Step              The number of step to execute
 * \param Bursts            Vector of bursts to analyze
 * \param Epsilon           Value of epsilon to be used in this step
 * \param PreviousPartition Results of the previous step
 * \param NewPartition      Results of the current step
 * \param Stop              I/O parameter to indicate that no more clusters 
 *                          were generated
 *
 * \return True if the step was executed correctly, false otherwise
 *
 */
bool ClusteringRefinementDivisive::RunStep(size_t                   Step,
                                           const vector<CPUBurst*>& Bursts,
                                           double                   Epsilon,
                                           Partition&               PreviousPartition,
                                           Partition&               NewPartition,
                                           bool&                    Stop)
{
  ostringstream  Messages;
  size_t         TotalParentOccurrences;
  timestamp_t    TotalParentDurations;
  size_t         NodesAnalyzed = 0;
  
  vector<ClusterInformation*>& ParentNodes   = NodesPerLevel[Step-1];
  vector<ClusterInformation*>& ChildrenNodes = NodesPerLevel[Step];

  /*
  Messages.str("");
  Messages << "|-> Generating candidates" << endl;
  system_messages::information(Messages.str());
  */
 
  // GenerateCandidates(Step,);

  /* Compute ParentNodes totalizations */
  TotalParentOccurrences = 0;
  TotalParentDurations   = 0;
  for (size_t i = 0; i < ParentNodes.size(); i++)
  {
    if (ParentNodes[i]->GetID() != NOISE_CLUSTERID)
    {
      TotalParentOccurrences += ParentNodes[i]->GetOccurrences();
    }
    TotalParentDurations += ParentNodes[i]->GetTotalDuration();
  }

  /* Run different DBSCANs */
  Stop = true;
  for (size_t i = 0; i < ParentNodes.size(); i++)
  {
    if (ParentNodes[i]->IsCandidate(TotalParentOccurrences, TotalParentDurations))
    {
      vector<CPUBurst*>           BurstsSubset;
      Partition                   ChildrenPartition;
      vector<ClusterInformation*> CurrentChildrenNodes;

      NodesAnalyzed++;
      
      /* Run DBSCAN in the subset of bursts */
      BurstsSubset = GenerateBurstsSubset(Bursts, ParentNodes[i]);
      
      Messages.str("");
      Messages << "|---> Running DBSCAN for ID = " << ParentNodes[i]->GetID();
      Messages << " (" << BurstsSubset.size() << " bursts)" << endl;
      system_messages::information(Messages.str());

      if (!RunDBSCAN ((vector<const Point*>&) BurstsSubset,
                                              Epsilon,
                                              ChildrenPartition))
      {
        return false;
      }

      /* Create current node children */
      Messages.str("");
      Messages << "|---> Generating current nodes" << endl;
      system_messages::information(Messages.str());


      if (!GenerateNodes(BurstsSubset,
                         ChildrenPartition,
                         CurrentChildrenNodes))
      {
        return false;
      }

      Messages.str("");
      Messages << "|---> " << CurrentChildrenNodes.size() << " nodes" << endl;
      system_messages::information(Messages.str());

      /* Add children to current node and current level nodes*/
      for (size_t j = 0; j < CurrentChildrenNodes.size(); j++)
      {
        ParentNodes[i]->AddChild(CurrentChildrenNodes[j]);
        ChildrenNodes.push_back(CurrentChildrenNodes[j]);
      }

      /* Evaluate split */
      if (IsSplitOK(ParentNodes[i]))
      {
        Stop = false;
      }
    }
  }

  if (NodesAnalyzed == 0)
  {
    Messages.str("");
    Messages << "|-> NO NODES ANALYZED" << endl;
    system_messages::information(Messages.str());
    return true;
  }
  
  /* Rename the children */
  /* THAT'S BUGGY! TAKE INTO ACCOUNT ALL LEVEL NODES */
  Messages.str("");
  Messages << "|-> Renaming nodes" << endl;
  system_messages::information(Messages.str());
  
  cluster_id_t MaxIDAssigned = NOISE_CLUSTERID;
  for (size_t i = 0; i < Step; i++)
  {
    for (size_t j = 0; j < NodesPerLevel[i].size(); j++)
    {
      if (!NodesPerLevel[i][j]->IsDiscarded() &&
          NodesPerLevel[i][j]->GetID() > MaxIDAssigned)
      {
        MaxIDAssigned = NodesPerLevel[i][j]->GetID();
      }
    }
  }
  /*
  for (size_t i = 0; i < ParentNodes.size(); i++)
  {
    if (ParentNodes[i]->GetID() > MaxIDAssigned)
    {
      MaxIDAssigned = ParentNodes[i]->GetID();
    }
  }
  */
  MaxIDAssigned++;

  for (size_t i = 0; i < ParentNodes.size(); i++)
  {
    ParentNodes[i]->RenameChildren(MaxIDAssigned);
  }
  
  /* Join results for current level */
  Messages.str("");
  Messages << "|-> Joining results" << endl;
  system_messages::information(Messages.str());

  GeneratePartition(NewPartition);
  

  /* DEBUG
  cout << "Current Assignment Vector Has " << CurrentAssignmentVector.size();
  cout << " IDs" << endl; */

  /* Print plots */
  if (PrintStepsInformation)
  {
    Messages.str("");
    Messages << "|-> Printing trees" << endl;
    system_messages::information(Messages.str());

    if (!PrintTrees(Step))
    {
      return false;
    }
    
    Messages.str("");
    Messages << "|-> Printing plots" << endl;
    system_messages::information(Messages.str());
    
    if (!PrintPlots(Bursts, NewPartition, Step))
    {
      return false;
    }
  }
  
  return true;
}

/**
 * Creates a vector which is a subset of those bursts from the input vector
 * that belong to the given node
 *
 * \param Bursts The input bursts vector
 * \param Node   The node that define the bursts
 *
 * \return A vector of bursts that have the selected ID in the partition
 *
 */
vector<CPUBurst*> ClusteringRefinementDivisive::GenerateBurstsSubset(const vector<CPUBurst*>& Bursts,
                                                             ClusterInformation*       Node)
{
  vector<CPUBurst*>   Result;
  vector<instance_t>& Instances = Node->GetInstances();

  for (size_t i = 0; i < Instances.size(); i++)
  {
    Result.push_back(Bursts[Instance2Burst[Instances[i]]]);
  }

  return Result;
}

/**
 * Executes the DBSCAN algorithm
 *
 * \param CurrentData Points to be analyzed
 * \param Epsilon Value of epsilon to be used in the algorithm
 * \param CurrentPartition Partition object that helds the results
 *
 * \return True if the step was executed correctly, false otherwise
 *
 */ 
bool ClusteringRefinementDivisive::RunDBSCAN(const vector<const Point*>& CurrentData,
                                     double                      Epsilon,
                                     Partition&                  CurrentPartition)
{
  string              ClusteringAlgorithmName;
  map<string, string> ClusteringAlgorithmParameters;
  ostringstream       Converter;
  string              EpsilonStr, MinPointsStr;

  ClusteringAlgorithmName   = DBSCAN::NAME;

  Converter << Epsilon;
  ClusteringAlgorithmParameters.insert(std::make_pair(DBSCAN::EPSILON_STRING, string(Converter.str())));
  Converter.str("");
  Converter << MinPoints;
  ClusteringAlgorithmParameters.insert(std::make_pair(DBSCAN::MIN_POINTS_STRING, string(Converter.str())));

  if (!ClusteringCore->InitClustering (ClusteringAlgorithmName,
                                       ClusteringAlgorithmParameters))
  {
    SetError(true);
    SetErrorMessage(ClusteringCore->GetErrorMessage());
    return false;
  }

  bool verbose_state = system_messages::verbose;
  system_messages::verbose = false;
  
  if (!ClusteringCore->ExecuteClustering(CurrentData,
                                         CurrentPartition))
  {
    system_messages::verbose = verbose_state;
    SetError(true);
    SetErrorMessage(ClusteringCore->GetErrorMessage());
    return false;
  }
  system_messages::verbose = verbose_state;
  
  return true;
}

/**
 * Generate the nodes that describe a given partition. It generates the cluster
 * statistics, rename the clusters depending on their duration, and also compute
 * the sequence score of each cluster
 *
 * \Bursts    Set of bursts used in the cluster analysis
 * \Partition Partition obtained using the clustering algorithm
 * \Node      The set of nodes that describe the partition
 *
 * \return True if the the node were generated correctly, false otherwise
 *
 */ 
bool ClusteringRefinementDivisive::GenerateNodes(const vector<CPUBurst*>&     Bursts,
                                                 Partition&                   CurrentPartition,
                                                 vector<ClusterInformation*>& Nodes)
{
  SequenceScore                            Scoring;
  vector<SequenceScoreValue>               CurrentClustersScores;
  double                                   GlobalScore;
  vector<percentage_t>                     PercentageDurations;
  map<cluster_id_t, vector<instance_t> >   BurstsPerNode;
  map<cluster_id_t, vector<instance_t> >::iterator BurstPerNodeIt;

  bool NoNoise = false;
  
  ostringstream  Messages;

  ClusteringStatistics Statistics;

  vector<cluster_id_t>& AssignmentVector = CurrentPartition.GetAssignmentVector();
  if (Bursts.size() != AssignmentVector.size())
  {
    ostringstream ErrorMessage;

    ErrorMessage << "number of points (" << Bursts.size();
    ErrorMessage << ") different from number of IDs (" << AssignmentVector.size() << ")";
    ErrorMessage << " when generating partition points";
    
    SetErrorMessage(ErrorMessage.str());
    SetError(true);
    return false;
  }
  
  /* Update Statistics */
  Statistics.InitStatistics(CurrentPartition.GetIDs(),
                            CurrentPartition.HasNoise());

  //  Messages.str("");
  // Messages << "|---> Computing statistics" << endl;
  // system_messages::information(Messages.str());

  if (!Statistics.ComputeStatistics(Bursts,
                                    CurrentPartition.GetAssignmentVector()))
  {
    SetErrorMessage(Statistics.GetLastError());
    return false;
  }
  
  Statistics.TranslatedIDs(CurrentPartition.GetAssignmentVector());
  
  PercentageDurations = Statistics.GetPercentageDurations(); 

  Messages.str("");
  Messages << "|-----> Computing score" << endl;
  system_messages::information(Messages.str());
  
  Scoring.ComputeScore(Bursts,
                       CurrentPartition.GetAssignmentVector(),
                       PercentageDurations,
                       CurrentPartition.HasNoise(),
                       CurrentClustersScores,
                       GlobalScore,
                       "TEST",
                       true);

  /* Generate current level hierarchy */
  vector<double> CurrentClustersDurations   = Statistics.GetDurationSums();
  vector<size_t> CurrentClustersIndividuals = Statistics.GetIndividuals();
  
  Nodes.clear();
  
  for (size_t i = 0; i < CurrentPartition.NumberOfClusters(); i++)
  {
    /* DEBUG 
    cout << "Subcluster ID = " << i << " Score = " << CurrentClustersScores[i].GetClusterScore();
    cout << " Individuals = " << CurrentClustersIndividuals[i] << endl; */

    ClusterInformation* NewNode = 
      new ClusterInformation((cluster_id_t) i,
                              CurrentClustersScores[i].GetClusterScore(),
                              CurrentClustersScores[i].GetOccurrences(),
                              CurrentClustersDurations[i],
                              CurrentClustersIndividuals[i]);

    Nodes.push_back(NewNode);


    BurstsPerNode[i] = vector<instance_t> (0);
  }

  /* Fill each node with the instances */
  for (size_t i = 0; i < Bursts.size(); i++)
  {
    BurstsPerNode[AssignmentVector[i]].push_back(Bursts[i]->GetInstance());
    /* DEBUG 
    cout << "NODE ID = " << AssignmentVector[i] << " INSTANCE = " << Bursts[i]->GetInstance() << endl; */
  }

  for (size_t i = 0; i < Nodes.size(); i++)
  {
    /* DEBUG 
    cout << "NODE with ID = " << Nodes[i]->GetID() << " has " << BurstsPerNode[i].size() << " instances" << endl; */
    Nodes[i]->SetInstances(BurstsPerNode[i]);
  }
  
  return true;
}

/**
 * Generate a partition with the current state of the tree
 *
 * \param NewPartition Object to store the partition obtained
 *
 */
void ClusteringRefinementDivisive::GeneratePartition(Partition& NewPartition)
{
  map<instance_t, cluster_id_t>           Assignment;
  map<instance_t, cluster_id_t>::iterator AssignmentIt;
  vector<ClusterInformation*>             TopLevelNodes = NodesPerLevel[0];

  for (size_t i = 0; i < TopLevelNodes.size(); i++)
  {
    vector<pair<instance_t, cluster_id_t> > PartialAssignment;

    PartialAssignment = GetAssignment(TopLevelNodes[i]);

    /* DEBUG
    cout << "PartialAssignment from ID = " << TopLevelNodes[i]->GetID();
    cout << " sizes " << PartialAssignment.size() << endl; */

    for (size_t j = 0; j < PartialAssignment.size(); j++)
    {
      Assignment[PartialAssignment[j].first] = PartialAssignment[j].second;
    }
  }

  vector<cluster_id_t>& CurrentAssignmentVector =
    NewPartition.GetAssignmentVector();

  set<cluster_id_t> DifferentIDs;

  for (AssignmentIt  = Assignment.begin();
       AssignmentIt != Assignment.end();
       ++AssignmentIt)
  {
    CurrentAssignmentVector.push_back(AssignmentIt->second);
    DifferentIDs.insert(AssignmentIt->second);
  }

  NewPartition.SetIDs(DifferentIDs);
  NewPartition.NumberOfClusters(DifferentIDs.size());
  NewPartition.HasNoise(true);

  return;
}

/**
 * Returns an assignment vector based on instances for the tree which root is 
 * the given node
 *
 * \param Node The root of a tree where we want to extract the assignment
 *
 * \return A vector of pairs burst instance/ID correspondig to the refinement
 *         happened in the current tree
 */
vector<pair<instance_t, cluster_id_t> > ClusteringRefinementDivisive::GetAssignment(ClusterInformation* Node)
{
  vector<pair<instance_t, cluster_id_t> > Result =
    vector<pair<instance_t, cluster_id_t> > (0);

  bool LeafNode;
  
  /* Node not taken into account */
  if (Node->IsDiscarded())
  {
    return Result;
  }

  vector<ClusterInformation*>& Children = Node->GetChildren();

  if (Children.size() == 0)
  {
    LeafNode = true;
  }
  else
  {
    LeafNode = true;
    
    for (size_t i = 0; i < Children.size(); i++)
    {
      if (!Children[i]->IsDiscarded())
      {
        LeafNode = false;
      }
    }
  }

  if (LeafNode)
  { /* Leaf node, generate the assignemnt vector */
    vector<instance_t>& Instances = Node->GetInstances();

    for (size_t i = 0; i < Instances.size(); i++)
    {
      Result.push_back(make_pair(Instances[i], Node->GetID()));
    }

    return Result;
  }
  else
  {
    for (size_t i = 0; i < Children.size(); i++)
    {
      vector<pair<instance_t, cluster_id_t> > PartialResult;

      if (!Children[i]->IsDiscarded())
      {
        PartialResult = GetAssignment(Children[i]);

        for (size_t j = 0; j < PartialResult.size(); j++)
        {
          Result.push_back(PartialResult[j]);
        }
      }
    }

    /* DEBUG 
    cout << "NODE with ID = " << Node->GetID() << " has ";
    cout << Result.size() << " instances in its assignment" << endl; */
    
    return Result;
  }
}


/**
 * Heuristic to Decide if it is better to split the current cluster (represented 
 * as a top node) of keep it as a single one
 *
 * \param Parent Node that contains the information of the hierarchy
 *
 * \return True if the split is OK, false otherwise
 *
 */ 
bool ClusteringRefinementDivisive::IsSplitOK(ClusterInformation* Parent)
{
  timestamp_t          ParentDuration = 0, NoiseDuration = 0;
  timestamp_t          TotalChildrenDuration = 0;
  size_t               TotalChildrenOccurrences = 0;
  percentage_t         ChildrenWeightedScore = 0, ParentWeightedScore = 0;

  bool Result;

  vector<ClusterInformation*>& Children = Parent->GetChildren();
  
  if (Children.size() == 0)
  { /* No children */
    /* DEBUG 
    cout << "No children!" << endl; */
    return false;
  }

  if (Children.size() == 1 && Children[0]->GetID() == NOISE_CLUSTERID)
  { /* Everything turned into noise */
    /* DEBUG 
    cout << "Everything turned into noise!" << endl; */
    Children[0]->Discard();
    return false;
  }

  ParentDuration = Parent->GetTotalDuration();

  for (size_t i = 0; i < Children.size(); i++)
  {
    if (Children[i]->GetID() == NOISE_CLUSTERID)
    {
      NoiseDuration = Children[i]->GetTotalDuration();
      break;
    }
  }

  /* Noise >= 60% of parent duration. CHECK PERCENTAGE TO REFINE */
  if (NoiseDuration >= 0.6*ParentDuration)
  {
    for (size_t i = 0; i < Children.size(); i++)
    {
      Children[i]->Discard();
    }

    /* DEBUG 
    cout << "Noise duration (" << NoiseDuration << ")";
    cout << " bigger than 80% of parent duration (" << ParentDuration << ")" << endl; */
    
    return false;
  }

  for (size_t i = 0; i < Children.size(); i++)
  {
    if (Children[i]->GetID() != NOISE_CLUSTERID)
    {
      TotalChildrenDuration += Children[i]->GetTotalDuration();
      ChildrenWeightedScore += (Children[i]->GetScore()*Children[i]->GetTotalDuration());
    }
  }
  ChildrenWeightedScore /= TotalChildrenDuration;


  /* Weighted children score < 80% of parent score */
  if (ChildrenWeightedScore < (0.8*Parent->GetScore()))
  {
    for (size_t i = 0; i < Children.size(); i++)
    {
      Children[i]->Discard();
    }

    /* DEBUG 
    cout << "ChildrenWeightedScore (" << ChildrenWeightedScore << ") ";
    cout << " less than 80% of Parent Score (" << Parent->GetScore() << ")" << endl; */
    return false;
  }

  return true;
}

/**
 * Collapses the current node if the subtree where it is root has no divisions.
 * It also add the non-colapsed children to the ClustersHierarchy vectors
 *
 * \param Node ClusterInformation node to be treated
 *
 * \return The total number of divisions in the subtree
 *
 */  
size_t ClusteringRefinementDivisive::ColapseNonDividedSubtrees(ClusterInformation* Node)
{
  size_t NumberOfDivisions = 0;
  
  vector<ClusterInformation*>& Children = Node->GetChildren();
  
  if (Children.size() == 0) /* Base Case */
  { 
      /* DEBUG 
    cout << "ID " << Node->GetID() << " on Level " << Level+1 << " is a LEAF" << endl; */

    return 1;
  }

  for (size_t i = 0; i < Children.size(); i++)
  {
    if (Children[i]->GetID() != NOISE_CLUSTERID &&
        !Children[i]->IsDiscarded())
    {
      NumberOfDivisions += ColapseNonDividedSubtrees(Children[i]);
    }
  }

  if (NumberOfDivisions == 1 || NumberOfDivisions == 0)
  { /* Delete the subtree! */

    /* DEBUG 
    cout << "** Collapsing node " << Node->GetID() << " on level " << Level+1;
    cout << " (" << Children[1] << ") **" << endl; */

    /* EXPERIMENTAL, do not delete nodes, just mark them as discarded */
    Children[0]->Discard();
    if (NumberOfDivisions == 1)
    {
      Children[1]->Discard();
    }
    // delete Children[0];
    // delete Children[1];
    // Children.clear();
  }

  if (NumberOfDivisions == 0)
  {
    NumberOfDivisions++;
  }
  
  /* DEBUG
  cout << "ID " << Node->GetID() << " on Level " << Level+1 << " ";
  cout << "has " << NumberOfDivisions << " divisions" << endl; */
  
  return NumberOfDivisions;
}

/**
 *
 */

void ClusteringRefinementDivisive::ReclassifyNoise(const vector<CPUBurst*>& Bursts,
                                                   ClusterInformation*      Node,
                                                   size_t                   Level)
{
  vector<ClusterInformation*>& Children = Node->GetChildren();
  bool LeafChildren = true;

  if (Children.size() == 0)
  {
    return;
  }

  for (size_t i = 0; i < Children.size(); i++)
  {
    if (!Children[i]->IsLeaf())
    {
      LeafChildren = false;
    }
  }
  
  if (!LeafChildren)
  { /* Descend to leaves */
    for (size_t i = 0; i < Children.size(); i++)
    {
      if (Children[i]->GetID() != NOISE_CLUSTERID)
      {
        ReclassifyNoise(Bursts, Children[i], Level+1);
      }
    }
  }
  else
  { /* Apply the classification */
    vector<instance_t>                     NoiseInstances;
    map<instance_t, cluster_id_t>          ClusterInstances;
    vector<cluster_id_t>                   NoiseClassification;

    ClusterInformation*                    NoiseNode;
    map<cluster_id_t, ClusterInformation*> ClusterNodes;

    /* DEBUG
    cout << "Reclassifying Noise of Node ID = " << Node->GetID();
    cout << " on Level " << Level+1 << endl; */
    
    /* Separate noise and clusters instances */
    for (size_t i = 0; i < Children.size(); i++)
    {
      if (Children[i]->GetID() == NOISE_CLUSTERID)
      {
        NoiseInstances = Children[i]->GetInstances();
        Children[i]->Discard();
        Children[i]->SetReclassified();
        NoiseNode = Children[i];

        if (NoiseInstances.size() == 0)
        {
          return;
        }
      }
      else
      {
        vector<instance_t>& ChildrenInstances = Children[i]->GetInstances();
        ClusterNodes[Children[i]->GetID()] = Children[i];

        for (size_t j = 0; j < ChildrenInstances.size(); j++)
        {
          ClusterInstances[ChildrenInstances[j]] = Children[i]->GetID();
        }
      }
    }

    /* Reclassify points */
    for (size_t i = 0; i < NoiseInstances.size(); i++)
    {
      map<instance_t, cluster_id_t>::iterator ClusterInstancesIt;

      double       MinDistance = MAX_DOUBLE;
      cluster_id_t CandidateID = NOISE_CLUSTERID;
      CPUBurst*    NoisePoint  = Bursts[Instance2Burst[NoiseInstances[i]]];

      for (ClusterInstancesIt  = ClusterInstances.begin();
           ClusterInstancesIt != ClusterInstances.end();
           ++ClusterInstancesIt)
      {
        CPUBurst* ClusteredPoint = Bursts[Instance2Burst[ClusterInstancesIt->first]];
        double Distance = (*NoisePoint).EuclideanDistance((*ClusteredPoint));

        if (Distance < MinDistance)
        {
          MinDistance = Distance;
          CandidateID = ClusterInstancesIt->second;
        }
      }

      NoiseClassification.push_back(CandidateID);
    }

    /* Adjust the nodes */
    vector<instance_t> Clear = vector<instance_t> (0);
    NoiseNode->SetInstances(Clear);

    for (size_t i = 0; i < NoiseInstances.size(); i++)
    {
      cluster_id_t ClassificationID = NoiseClassification[i];

      /* DEBUG
      cout << "Child Noise of Node = " << Node->GetID() << " on Level " << Level+1;
      cout << " reclassified as " << ClassificationID << endl; */
      
      ClusterNodes[ClassificationID]->AddInstance(NoiseInstances[i]);
    }
  }

  return;
}

/**
 * Checks if an ID is inside a set of IDs
 *
 * \param ID Value to check
 * \param IDsSet Set of IDs
 *
 * \return True ID is inside IDsSet, false otherwise
 *
 */ 
bool ClusteringRefinementDivisive::IsIDInSet(cluster_id_t       ID,
                                             set<cluster_id_t>& IDsSet)
{
  set<cluster_id_t>::iterator SetIterator;

  if (IDsSet.count(ID) > 0)
    return true;
  else
    return false;
}

/**
 * Generates the DATA files and the GNUPlot scripts for a given cluster results
 *
 * \param Bursts Vector containing the bursts of the current step
 * \param CurrentPartition Partition object of the burst vector
 * \param Step Depth of the current step
 *
 * \return True if plots were printed correctly, false otherwise
 *
 */ 
bool ClusteringRefinementDivisive::PrintPlots(const vector<CPUBurst*>& Bursts,
                                              Partition&               CurrentPartition,
                                              size_t                   Step)
{
  ostringstream         CurrentDataFileName;
  ostringstream         CurrentPlotFileNamePrefix;
  ostringstream         PlotTitle;
  ofstream              CurrentDataStream;
  vector<cluster_id_t>& IDs = CurrentPartition.GetAssignmentVector();

  CurrentPlotFileNamePrefix << OutputFilePrefix << ".STEP" << (Step+1);
  CurrentDataFileName << OutputFilePrefix << ".STEP" << (Step+1) << ".DATA.csv";

  if (Bursts.size() != IDs.size())
  {
    ostringstream ErrorMessage;

    ErrorMessage << "number of points (" << Bursts.size();
    ErrorMessage << ") different from number of IDs (" << IDs.size() << ")";
    ErrorMessage << " when printing plots";
    
    SetErrorMessage(ErrorMessage.str());
    SetError(true);
    return false;
  }
  
  CurrentDataStream.open(CurrentDataFileName.str().c_str(), ios_base::trunc);

  if (CurrentDataStream.fail())
  {
    ostringstream ErrorMessage;

    ErrorMessage << "unable to open data output file for step " << Step;
    SetError(true);
    SetErrorMessage(ErrorMessage.str().c_str(), strerror(errno));
    
    return false;

  }

  /* Flush points */
  ParametersManager *Parameters = ParametersManager::GetInstance();
      
  vector<string> ClusteringParametersNames;
  vector<string> ExtrapolationParametersNames;

  vector<bool>   ClusteringParametersPrecision;
  vector<bool>   ExtrapolationParametersPrecision;

  ClusteringParametersNames        = Parameters->GetClusteringParametersNames();
  ExtrapolationParametersNames     = Parameters->GetExtrapolationParametersNames();

  ClusteringParametersPrecision    = Parameters->GetClusteringParametersPrecision();
  ExtrapolationParametersPrecision = Parameters->GetExtrapolationParametersPrecision();
  

  CurrentDataStream << "# Instance,TaskId,ThreadId,Begin_Time,End_Time,Duration, Line";
  for (size_t i = 0; i < ClusteringParametersNames.size(); i++)
  {
    CurrentDataStream << "," << ClusteringParametersNames[i];
  }

  for (size_t i = 0; i < ClusteringParametersNames.size(); i++)
  {
    CurrentDataStream << "," << ClusteringParametersNames[i] << "_Norm";
  }

  for (size_t i = 0; i < ExtrapolationParametersNames.size(); i++)
  {
    CurrentDataStream << "," << ExtrapolationParametersNames[i];
  }

  CurrentDataStream << ",ClusterID" << endl;

  for (size_t i = 0; i < Bursts.size(); i++)
  {
    Bursts[i]->Print(CurrentDataStream,
                     ClusteringParametersPrecision,
                     ExtrapolationParametersPrecision,
                     IDs[i]+PARAVER_OFFSET);
  }

  /* Create plots */
  PlottingManager *Plots;
  
  Plots = PlottingManager::GetInstance();  // No Data Extraction

  /* DEBUG 
  cout << __FUNCTION__ << "Algorithm name = " << ClusteringCore->GetClusteringAlgorithmName() << endl;
  cout << "Current partition has " << CurrentPartition.NumberOfClusters() << " clusters" << endl; */
  
  PlotTitle << "REFINEMENT STEP " << Step+1 << " - ";
  PlotTitle << ClusteringCore->GetClusteringAlgorithmName();

  bool verbose_state = system_messages::verbose;
  system_messages::verbose = false;
  
  if (!Plots->PrintPlots(CurrentDataFileName.str(),
                         CurrentPlotFileNamePrefix.str(),
                         PlotTitle.str(),
                         CurrentPartition.NumberOfClusters(),
                         CurrentPartition.HasNoise()))
  {
    system_messages::verbose = verbose_state;
    SetError(true);
    SetErrorMessage(Plots->GetLastError());
    return false;
  }
  
  system_messages::verbose = verbose_state;
  
  
  return true;
}

/**
 * Generates a representation of the resulting partitioning tree to a file
 *
 * \return True if the tree was written correctly, false otherwise
 *
 */  
bool ClusteringRefinementDivisive::PrintTrees(size_t Step, bool LastTree)
{
  ofstream* Output;

  vector<ClusterInformation*>& TopLevelNodes = NodesPerLevel[0];
  
  if (PrintStepsInformation)
  {
    ostringstream TreeFileName;

    if (!LastTree)
    {
      TreeFileName << OutputFilePrefix << ".STEP" << (Step+1) << ".TREE.dot";
    }
    else
    {
      TreeFileName << OutputFilePrefix << ".TREE.dot";
    }

    Output = new ofstream(TreeFileName.str().c_str(), ios_base::trunc);

    if (Output->fail())
    {
      SetError(true);
      SetErrorMessage ("unable to open tree file", strerror(errno));
      return false;
    }
    
  }
  else
  {
    Output = (ofstream*) &cout;
  }

  (*Output) << "digraph Tree {" << endl;

  
  for (size_t i = 0; i < TopLevelNodes.size(); i++)
  {
    
    PrintTreeNodes ((*Output), TopLevelNodes[i]);

    (*Output) << endl;

    PrintTreeLinks ((*Output), TopLevelNodes[i]);
    
    (*Output) << endl;
  }

  (*Output) << "}" << endl << endl;
  
  /* DEBUG 
  cout << "TOP LEVEL HAS " << ClustersHierarchy[0].size() << " NODES" << endl; */

  Output->close();
  
  return true;
}

bool ClusteringRefinementDivisive::PrintTreeNodes(ofstream&           str,
                                          ClusterInformation* Node)
{
  vector<ClusterInformation*>& Children = Node->GetChildren();

  if (Node->GetID() != NOISE_CLUSTERID || Node->GetIndividuals() != 0)
  {
    str << Node->NodeName() << " " << Node->NodeLabel() << ";" << endl;
    
    for (size_t i = 0; i < Children.size(); i++)
    {
      PrintTreeNodes(str, Children[i]);
    }
  }

  return true;
}

bool ClusteringRefinementDivisive::PrintTreeLinks(ofstream&           str,
                                          ClusterInformation* Node)
{
  vector<ClusterInformation*>& Children = Node->GetChildren();
  
  for (size_t i = 0; i < Children.size(); i++)
  {
    if (Children[i]->GetID() != NOISE_CLUSTERID || Children[i]->GetIndividuals() != 0)
    {
      str << "\"" << Node->NodeName() << "\" ->  \"" << Children[i]->NodeName() << "\"";
      str << ";" << endl;
    }
  }

  for (size_t i = 0; i < Children.size(); i++)
  {
    if (Children[i]->GetID() != NOISE_CLUSTERID || Children[i]->GetIndividuals() != 0)
    {
      PrintTreeLinks(str, Children[i]);
    }
  }
  
  return true;
}

