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


#include <SystemMessages.hpp>
using cepba_tools::system_messages;

#include "ClusteringRefinementAggregative.hpp"
#include "SequenceScore.hpp"

#include "PlottingManager.hpp"

#include <Utilities.hpp>
#include <ParaverColors.h>
#include <DBSCAN.hpp>

#include <sstream>
#include <iomanip>
using std::ostringstream;
using std::setfill;
using std::setw;
using std::setprecision;

#include <algorithm>
using std::sort;

/******************************************************************************
 * CLASS 'ClusteringRefinementAggregative'
 *****************************************************************************/

/**
 * Parametrized constructor.
 *
 * \param MinPoints        MinPoints value to be used in all DBSCAN runs
 * \param EpsilonsPerLevel Different values of epsilons to be used
 *
 */
ClusteringRefinementAggregative::ClusteringRefinementAggregative(INT32          MinPoints,
                                                                 vector<double> EpsilonPerLevel)
{
  this->MinPoints       = MinPoints;
  this->EpsilonPerLevel = EpsilonPerLevel;
  this->Steps           = EpsilonPerLevel.size();
  MaxIDAssigned         = NOISE_CLUSTERID;
}


/**
 * Executes the refinement analysis
 *
 * \param Bursts                 Vector of bursts that must be analyzed
 * \param IntermediatePartitions Vector of partitions corresponding to each
 *                               refinement step
 * \param LastPartition          Definitive partition obtained after the
 *                               refinement analysis
 * \param OutputFilePrefix       If different of empty string, it indicates
 *                               that every step should be printed
 *
 * \return True if the refinement worked properly, false otherwise
 *
 */
bool ClusteringRefinementAggregative::Run(const vector<CPUBurst*>& Bursts,
                                          vector<Partition>&       IntermediatePartitions,
                                          Partition&               LastPartition,
                                          bool                     PrintStepsInformation,
                                          string                   OutputFilePrefix)
{
  bool   Stop;
  ostringstream  Messages;

  this->OutputFilePrefix = OutputFilePrefix;

  this->PrintStepsInformation = PrintStepsInformation;

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

  ClusteringCore = new libClustering();

  /* Generate Instances to Bursts translation table */
  for (size_t i = 0; i < Bursts.size(); i++)
  {
    Instance2Burst[Bursts[i]->GetInstance()] = i;
  }

  Messages << "****** STEP 1 (Eps = " << EpsilonPerLevel[0] << ") ******" << endl;
  system_messages::information(Messages.str());

  IntermediatePartitions.clear();
  IntermediatePartitions.push_back(Partition());

  NodesPerLevel.clear();
  NodesPerLevel.push_back(vector<ClusterInformation*>(0));

  /* Generate top level leaves :D */
  if (!RunFirstAnalysis(Bursts,
                        IntermediatePartitions[0]))
  {
    return false;
  }

  /* Iterate through the epsilon values, and generate the clusters hierarchy */
  Stop = false;
  for (size_t CurrentStep = 1;
              CurrentStep < Steps && !Stop;
              CurrentStep++)
  {
    IntermediatePartitions.push_back(Partition());
    NodesPerLevel.push_back(vector<ClusterInformation*>(0));

    Messages.str("");
    Messages << "****** STEP " << (CurrentStep+1) << " (Eps = " << EpsilonPerLevel[CurrentStep];;
    Messages << ") ******" << endl;
    system_messages::information(Messages.str());

    if (!RunStep(CurrentStep,
                 Bursts,
                 IntermediatePartitions[CurrentStep-1],
                 IntermediatePartitions[CurrentStep],
                 Stop))
    {
      SetError(true);
      return false;
    }

    if (Stop)
    {
      /* Last partition is empty! */
      NodesPerLevel.pop_back();
      IntermediatePartitions.pop_back();
      Messages.str("");
      Messages << "****** CONVERGENCE ******" << endl;
      system_messages::information(Messages.str());
    }
    else
    {
      Messages.str("");
      Messages << "****** STEP " << CurrentStep+1 << " FINISHED ******" << endl;
      system_messages::information(Messages.str());
    }
  }

  LastPartition = IntermediatePartitions.back();

  if (!GenerateLastPartition(Bursts,
                             IntermediatePartitions.size(),
                             IntermediatePartitions.back(),
                             LastPartition))
  {
    return false;
  }

  /* DEBUG
  cout << "Last Partition Number of Clusters = " << LastPartition.NumberOfClusters() << endl;
  cout << "Last Partition Number of IDs = " << LastPartition.GetAssignmentVector().size() << endl;
  */

  return true;
}

bool ClusteringRefinementAggregative::RunFirstAnalysis(const vector<CPUBurst*>& Bursts,
                                                       Partition&               FirstPartition)
{
  ostringstream        Messages;
  ClusteringStatistics Statistics;
  double               Epsilon = EpsilonPerLevel[0];

  Messages.str("");
  Messages << "|---> Running DBSCAN (" << Bursts.size() << " bursts)" << endl;
  system_messages::information(Messages.str());

  if (!RunDBSCAN ((const vector<const Point*>&) Bursts,
                                                Epsilon,
                                                FirstPartition))
  {
    return false;
  }

  if (!GenerateNodes(0,
                     Bursts,
                     FirstPartition,
                     NodesPerLevel[0]))
  {
    return false;
  }

  ComputeScores(0, Bursts, NodesPerLevel[0], FirstPartition, false);

  vector<cluster_id_t>& AssignmentVector = FirstPartition.GetAssignmentVector();
  for (size_t i = 0; i < Bursts.size(); i++)
  {
    IDPerLevel[Bursts[i]->GetInstance()].push_back(AssignmentVector[i]);
  }

  MaxIDAssigned = FirstPartition.GetMaxID();

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
 * \param PreviousPartition Results of the previous step
 * \param NewPartition      Results of the current step
 * \param Stop              I/O parameter to indicate that no more clusters
 *                          were generated
 *
 * \return True if the step was executed correctly, false otherwise
 *
 */
bool ClusteringRefinementAggregative::RunStep(size_t                   Step,
                                              const vector<CPUBurst*>& Bursts,
                                              Partition&               PreviousPartition,
                                              Partition&               NewPartition,
                                              bool&                    Stop)
{
  ostringstream  Messages;
  double         Epsilon = EpsilonPerLevel[Step];

  vector<CPUBurst*>            BurstsSubset;
  vector<ClusterInformation*>& ParentNodes   = NodesPerLevel[Step-1];
  vector<ClusterInformation*>  CandidateNodes;

  Partition                    ChildrenPartition;
  vector<ClusterInformation*>  ChildrenNodes;

  /*
  Messages.str("");
  Messages << "|-> Generating candidates" << endl;
  system_messages::information(Messages.str());
  */

  if (!GenerateCandidatesAndBurstSubset(Bursts, ParentNodes, BurstsSubset, CandidateNodes))
  {
    Stop = true;
    return true;
  }

  /* DEBUG
  cout << "Candidate Bursts" << endl;
  for (size_t i = 0; i < BurstsSubset.size(); i++)
  {
    cout << "Instance = " << BurstsSubset[i]->GetInstance() << endl;
  }
  */
  Messages.str("");
  Messages << "|---> Candidate IDs = ";
  for (size_t i = 0; i < CandidateNodes.size(); i++)
  {
    if (CandidateNodes[i]->GetID() == NOISE_CLUSTERID)
    {
      Messages << "NOISE ";
    }
    else
    {
      Messages << CandidateNodes[i]->GetID() << " ";
    }
  }
  Messages << endl;
  system_messages::information(Messages.str());

  /* Run DBSCAN in the subset of bursts */
  Messages.str("");
  Messages << "|---> Running DBSCAN (" << BurstsSubset.size() << " bursts)" << endl;
  system_messages::information(Messages.str());

  if (!RunDBSCAN ((vector<const Point*>&) BurstsSubset,
                                          Epsilon,
                                          ChildrenPartition))
  {
    return false;
  }

  /* Create children nodes */
  Messages.str("");
  Messages << "|---> Generating current nodes" << endl;
  system_messages::information(Messages.str());

  if (!GenerateNodes(Step,
                     BurstsSubset,
                     ChildrenPartition,
                     ChildrenNodes))
  {
    return false;
  }

  Messages.str("");
  Messages << "|---> " << ChildrenNodes.size() << " nodes" << endl;
  system_messages::information(Messages.str());

  /* Link this level with the previous one */
  LinkNodes(BurstsSubset,
            CandidateNodes,
            ChildrenNodes,
            PreviousPartition,
            ChildrenPartition);

  /* Rename the children and put them in the current level vector */
  for (size_t i = 0; i < CandidateNodes.size(); i++)
  {
    CandidateNodes[i]->Visited(false);
  }

  for (size_t i = 0; i < ChildrenNodes.size(); i++)
  {
    ChildrenNodes[i]->RenameNode(MaxIDAssigned);
    NodesPerLevel[Step].push_back(ChildrenNodes[i]);
  }

  /* Join results for current level */
  Messages.str("");
  Messages << "|-> Joining results" << endl;
  system_messages::information(Messages.str());

  GeneratePartition(NewPartition);

  ComputeScores(Step, Bursts, ChildrenNodes, NewPartition, false);

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
 * Executes the DBSCAN algorithm
 *
 * \param CurrentData Points to be analyzed
 * \param Epsilon Value of epsilon to be used in the algorithm
 * \param CurrentPartition Partition object that helds the results
 *
 * \return True if the step was executed correctly, false otherwise
 *
 */
bool ClusteringRefinementAggregative::RunDBSCAN(const vector<const Point*>& CurrentData,
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
 * Generates the candidates nodes vector and candidates bursts vector
 *
 * \param Bursts       Vector of all bursts
 * \param ParentNodes  Nodes of previous level
 * \param BurstsSubset Output vector of those nodes that will take part in the
 *                     further analysis
 * \param NodesSubset  Output vector of nodes that will be analyzed in the
 *                     further analysis
 *
 * \return True if there are candidates available for this level, false if
 *         no more candidates are generated (convergence)
 */
bool ClusteringRefinementAggregative::GenerateCandidatesAndBurstSubset(
  const vector<CPUBurst*>&     Bursts,
  vector<ClusterInformation*>& ParentNodes,
  vector<CPUBurst*>&           BurstsSubset,
  vector<ClusterInformation*>& NodesSubset
)
{
  BurstsSubset.clear();

  vector<ClusterInformation*> Leaves;

  double SumOfScores   = 0.0;
  size_t NumberOfNodes = 0;

  bool MoreThan90 = false;
  bool LessThan10 = false;

  if (StopCondition())
  {
    return false;
  }

  for (size_t i = 0; i < ParentNodes.size(); i++)
  {
    SumOfScores += ParentNodes[i]->GetScore();
    NumberOfNodes++;
    if (!ParentNodes[i]->IsDiscarded())
    {
      if (ParentNodes[i]->GetScore() < 1) // Check here the cut value!
      {
        if(ParentNodes[i]->GetScore() > 0.9)
        {
          MoreThan90 = true;
        }
        else if (ParentNodes[i]->GetScore() < 0.1)
        {
          LessThan10 = true;
        }

        NodesSubset.push_back(ParentNodes[i]);
      }
    }
  }



  // Prune to stop aggregating when all clusters have more than 90% score and
  // there are no small ones (less than 10% score)
  /*
  if (MoreThan90 && !LessThan10)
  {
    NodesSubset.clear();
  }
  */

  if (NodesSubset.size() == 0)
  {
    return false;
  }

  for (size_t i = 0; i < NodesSubset.size(); i++)
  {
    vector<instance_t>& NodeInstances = NodesSubset[i]->GetInstances();

    for (size_t j = 0; j < NodeInstances.size(); j++)
    {
      CPUBurst* CurrentBurst = Bursts[Instance2Burst[NodeInstances[j]]];
      BurstsSubset.push_back(CurrentBurst);
    }
  }

  if (BurstsSubset.size() == 0)
  {
    return false;
  }

  sort(BurstsSubset.begin(), BurstsSubset.end(), InstanceNumCompare());

  return true;
}

/**
 * Checks the current state of the tree and decides if it is correct enough to
 * stop the aggregation
 *
 * \return True if the stop condition has reached, false otherwise
 *
 */
bool ClusteringRefinementAggregative::StopCondition(void)
{
  ostringstream               Messages;
  vector<ClusterInformation*> Leaves;
  double                      SumOfScores   = 0.0;

  for (size_t i = 0; i < NodesPerLevel.size(); i++)
  {
    for (size_t j = 0; j < NodesPerLevel[i].size(); j++)
    {
      if (NodesPerLevel[i][j]->IsLeaf() && !NodesPerLevel[i][j]->IsDiscarded())
      {
        Leaves.push_back(NodesPerLevel[i][j]);
      }
    }
  }

  for (size_t i = 0; i < Leaves.size(); i++)
  {
    SumOfScores += Leaves[i]->GetScore();
  }

  Messages.str("");
  Messages << "|---> Sum. of Scores = " << SumOfScores << " | Number of Nodes = ";
  Messages << Leaves.size() << " | Avg. Score per Node = " << SumOfScores/Leaves.size() << endl;
  system_messages::information(Messages.str());

  /*
  double Remainder = SumOfScores - floor(SumOfScores);

  if (Remainder > 0.95 || Remainder < 0.05)
  {
    Messages.str("");
    Messages << "|---> Remainder of Sum. of Scores = " << Remainder << endl;
    Messages << "|---> ** It could be a good point to STOP **" << endl;
    system_messages::information(Messages.str());
  }

  if ((SumOfScores/Leaves.size()) > 0.9)
  {
    Messages.str("");
    Messages << "|---> Average Score per node > 90% STOP" << endl;
    system_messages::information(Messages.str());
    return true;
  }
  */

  return false;
}

/**
 * Generate the nodes that describe a given partition. It generates the cluster
 * statistics, rename the clusters depending on their duration, and also compute
 * the sequence score of each cluster
 *
 * \param Step          The number of step to execute
 * \param Bursts        Set of bursts used in the cluster analysis
 * \param Partition     Partition obtained using the clustering algorithm
 * \param Node          The set of nodes that describe the partition
 * \param LastPartition Indicates if the nodes to generate come from the a
 *                      merge, of the last partition
 *
 * \return True if the the node were generated correctly, false otherwise
 *
 */
bool ClusteringRefinementAggregative::GenerateNodes(size_t                       Step,
                                                    const vector<CPUBurst*>&     Bursts,
                                                    Partition&                   CurrentPartition,
                                                    vector<ClusterInformation*>& Nodes,
                                                    bool                         LastPartition)
{
  SequenceScore              Scoring;
  vector<SequenceScoreValue> CurrentClustersScores;
  double                     GlobalScore;

  map<cluster_id_t, percentage_t>          PercentageDurations;
  map<cluster_id_t, vector<instance_t> >   BurstsPerNode;
  map<cluster_id_t, vector<instance_t> >::iterator BurstPerNodeIt;

  set<cluster_id_t>           DifferentIDs;
  set<cluster_id_t>::iterator IDsIterator;

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
  Statistics.InitStatistics(CurrentPartition.GetIDs());

  //  Messages.str("");
  // Messages << "|---> Computing statistics" << endl;
  // system_messages::information(Messages.str());

  if (!Statistics.ComputeStatistics(Bursts,
                                    CurrentPartition.GetAssignmentVector()))
  {
    SetErrorMessage(Statistics.GetLastError());
    return false;
  }

  if (!LastPartition)
  { // Last partition needs to have the same numbering because it doesn't get
    // renamed
    Statistics.TranslatedIDs(CurrentPartition.GetAssignmentVector());
  }

  PercentageDurations = Statistics.GetPercentageDurations();

  /*
  Messages.str("");
  Messages << "|-----> Computing score" << endl;
  system_messages::information(Messages.str());

  Scoring.ComputeScore(Bursts,
                       CurrentPartition.GetAssignmentVector(),
                       PercentageDurations,
                       CurrentClustersScores,
                       GlobalScore,
                       CurrentSequenceFileNamePrefix.str(),
                       true);
  */

  /* Generate current level hierarchy */
  map<cluster_id_t, double> CurrentClustersDurations   = Statistics.GetDurationSums();
  map<cluster_id_t, size_t> CurrentClustersIndividuals = Statistics.GetIndividuals();

  Nodes.clear();
  DifferentIDs = CurrentPartition.GetIDs();

  for (IDsIterator  = DifferentIDs.begin();
       IDsIterator != DifferentIDs.end();
       ++IDsIterator)
  {
    cluster_id_t CurrentID = (*IDsIterator);

    ClusterInformation* NewNode =
      new ClusterInformation(CurrentID,
                             CurrentClustersDurations[CurrentID],
                             CurrentClustersIndividuals[CurrentID]);

    Nodes.push_back(NewNode);
    BurstsPerNode[CurrentID] = vector<instance_t> (0);

    /* DEBUG
    cout << "Subcluster ID = " << CurrentID << " Score = " << CurrentClustersScores[i].GetClusterScore();
    cout << " Individuals = " << CurrentClustersIndividuals[CurrentID] << endl; */
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
    Nodes[i]->SetInstances(BurstsPerNode[Nodes[i]->GetID()]);
  }

  return true;
}

/**
 * Link parent and children nodes from two consecutive steps
 *
 * \param BurstsSubset      Vector of the burst used in the cluster analysis of
 *                          the last step
 * \param Parent            Vector of parent (candidate) nodes
 * \param Children          Vector of children nodes
 * \param PreviousPartition Partition obtained in the previous step
 * \param NewPartition      Partition obtained in the current step
 *
 */

void ClusteringRefinementAggregative::LinkNodes(const vector<CPUBurst*>&     BurstsSubset,
                                                vector<ClusterInformation*>& Parent,
                                                vector<ClusterInformation*>& Children,
                                                Partition&                   PreviousPartition,
                                                Partition&                   NewPartition)
{
  vector<cluster_id_t>& ChildrenAssignment = NewPartition.GetAssignmentVector();

  map<cluster_id_t, set<cluster_id_t> > Links;

  for (size_t i = 0; i < BurstsSubset.size(); i++)
  {
    cluster_id_t PreviousID;
    cluster_id_t CurrentID;

    PreviousID = IDPerLevel[BurstsSubset[i]->GetInstance()].back();
    CurrentID  = ChildrenAssignment[i];

    Links[PreviousID].insert(CurrentID);
  }

  /* DEBUG
  map<cluster_id_t, set<cluster_id_t> >::iterator LinksIt;
  cout << "LINKS!" << endl;
  for (LinksIt  = Links.begin();
       LinksIt != Links.end();
       ++LinksIt)
  {
    set<cluster_id_t>::iterator SetIterator;
    for (SetIterator  = LinksIt->second.begin();
         SetIterator != LinksIt->second.end();
         ++SetIterator)
    {
      cout << LinksIt->first << " --> " <<  (*SetIterator) << endl;
    }
  }
  */

  for (size_t i = 0; i < Parent.size(); i++)
  {
    ClusterInformation* ParentNode = Parent[i];
    cluster_id_t        ParentID;
    set<cluster_id_t>   ChildIDs;

    ParentID = ParentNode->GetID();
    ChildIDs  = Links[ParentID];

    for (size_t j = 0; j < Children.size(); j++)
    {
      if (ChildIDs.count(Children[j]->GetID()) > 0)
      {
        ParentNode->AddChild(Children[j]);
        Children[j]->AddParent(ParentNode);
      }
    }
  }

  /* Prune condition -> if a children reduces the Score of a parent, discard it
  for (size_t i = 0; i < Children.size(); i++)
  {
    if (Children[i]->GetID() != NOISE_CLUSTERID)
    {
      vector<ClusterInformation*>& Parents = Children[i]->GetParents();
      ClusterInformation*          MaxScoreParent;
      double                       MaxParentScore = 0;

      for (size_t j = 0; j < Parents.size(); j++)
      {
        if (Parents[j]->GetID() != NOISE_CLUSTERID &&
            Parents[j]->GetScore() > MaxParentScore)
        {
          MaxParentScore = Parents[j]->GetScore();
          MaxScoreParent = Parents[j];
        }
      }

      if (MaxParentScore > Children[i]->GetScore() && MaxScoreParent->GetChildren().size() == 1)
      {
        Children[i]->Discard();
      }
    }
  }
  */

  return;
}

/**
 * Generate a partition with the current state of the tree
 *
 * \param NewPartition Object to store the partition obtained
 *
 */
void ClusteringRefinementAggregative::GeneratePartition(Partition& NewPartition)
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

  return;
}

/**
 * Computes the score of a given partition, taking into account the cluster
 * from all possible levels
 *
 * \param Bursts           A vector of all bursts
 * \param NewNodes         Nodes from last level that must whose score needs to be updated
 * \param CurrentPartition The last partition obtained
 *
 * \return True if the score was correctly computed, false otherwise
 */
bool ClusteringRefinementAggregative::ComputeScores(size_t                       Step,
                                                    const vector<CPUBurst*>&     Bursts,
                                                    vector<ClusterInformation*>& NewNodes,
                                                    Partition&                   CurrentPartition,
                                                    bool                         LastPartition)
{
  ostringstream        CurrentSequenceFileNamePrefix, Messages;

  SequenceScore              Scoring;
  vector<SequenceScoreValue> CurrentClustersScores;
  double                     GlobalScore;

  ClusteringStatistics            Statistics;
  map<cluster_id_t, percentage_t> PercentageDurations;

  if (LastPartition)
  {
    CurrentSequenceFileNamePrefix << OutputFilePrefix;
  }
  else
  {
    CurrentSequenceFileNamePrefix << OutputFilePrefix << ".STEP" << (Step+1);
  }

  /* Update Statistics */
  Statistics.InitStatistics(CurrentPartition.GetIDs());

  if (!Statistics.ComputeStatistics(Bursts,
                                    CurrentPartition.GetAssignmentVector()))
  {
    SetErrorMessage(Statistics.GetLastError());
    return false;
  }

  PercentageDurations = Statistics.GetPercentageDurations();

  /* Compute Score */
  Messages.str("");
  Messages << "|-----> Computing score" << endl;
  system_messages::information(Messages.str());

  bool verbose_state = system_messages::verbose;
  system_messages::verbose = false;

  if (!Scoring.ComputeScore(Bursts,
                            CurrentPartition.GetAssignmentVector(),
                            PercentageDurations,
                            CurrentClustersScores,
                            GlobalScore,
                            PrintStepsInformation || LastPartition,
                            CurrentSequenceFileNamePrefix.str(),
                            true))
  {
    ostringstream ErrorMessage;
    ErrorMessage << "unable to compute sequence score:" << Scoring.GetLastError();

    SetError(true);
    SetErrorMessage(ErrorMessage.str());

    return false;
  }

  system_messages::verbose = verbose_state;

  GlobalScoresPerLevel.push_back(GlobalScore);

  /* Assign Score to each node */
  for (size_t i = 0; i < NewNodes.size(); i++)
  {
    ClusterInformation* CurrentNode = NewNodes[i];
    cluster_id_t CurrentID          = CurrentNode->GetID();

    for (size_t j = 0; j < CurrentClustersScores.size(); j++)
    {
      if (CurrentClustersScores[j].GetID() == CurrentID)
      {
        CurrentNode->SetScore(CurrentClustersScores[j].GetClusterScore());
        CurrentNode->SetOccurrences(CurrentClustersScores[j].GetOccurrences());
      }
    }
  }

  return true;
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
vector<pair<instance_t, cluster_id_t> > ClusteringRefinementAggregative::GetAssignment(ClusterInformation* Node)
{
  vector<pair<instance_t, cluster_id_t> > Result =
    vector<pair<instance_t, cluster_id_t> > (0);

  vector<ClusterInformation*>& Children = Node->GetChildren();

  if (Children.size() == 0)
  {
    vector<instance_t>& Instances = Node->GetInstances();

    for (size_t i = 0; i < Instances.size(); i++)
    {
      if (!Node->IsDiscarded())
      {
        Result.push_back(make_pair(Instances[i], Node->GetID()));

        /* Update the history of id's */
        IDPerLevel[Instances[i]].push_back(Node->GetID());
      }
      else
      {
        cluster_id_t LastID = IDPerLevel[Instances[i]].back();
        Result.push_back(make_pair(Instances[i], LastID));
        IDPerLevel[Instances[i]].push_back(LastID);
      }
    }
  }
  else
  {
    for (size_t i = 0; i < Children.size(); i++)
    {
      vector<pair<instance_t, cluster_id_t> > PartialResult;

      // if (!Children[i]->IsDiscarded())
      // {
        PartialResult = GetAssignment(Children[i]);

        for (size_t j = 0; j < PartialResult.size(); j++)
        {
          Result.push_back(PartialResult[j]);
        }
      // }
    }

    /* DEBUG
    cout << "NODE with ID = " << Node->GetID() << " has ";
    cout << Result.size() << " instances in its assignment" << endl; */

    return Result;
  }

  return Result;
}

/**
 * Generates the Last Partition, that joins those clusters that doesn't get
 * a perfect allignment, but appear simultaneously in the last alignment
 *
 * \param Burst         Vector containing the set of bursts
 *
 * \param LastStep      Value of the last step executed in the refinement
 *
 * \param PreviousPartition Partition obtained in the last step of refinement
 *
 * \param LastPartition I/O partition container. As input, it contains the
 *                      last assignment obtained in the aggregation process.
 *                      As output, it contains an assignment that merges those
 *                      clusters that apperar simultaneosly in the sequences
 *
 * \return              True if the last partition was correctly generated,
 *                      false otherwise
 */
bool ClusteringRefinementAggregative::GenerateLastPartition(const vector<CPUBurst*>& Bursts,
                                                            size_t                   LastStep,
                                                            Partition&               PreviousPartition,
                                                            Partition&               LastPartition)
{
  ostringstream                   Messages;
  ClusteringStatistics            Statistics;
  map<cluster_id_t, percentage_t> PercentageDurations;
  SequenceScore                   Scoring;
  vector<SequenceScoreValue>      CurrentClustersScores;
  double                          GlobalScore;
  ostringstream                   CurrentSequenceFileNamePrefix;

  size_t NumberOfSequences, SequencesLength;

  vector<set<cluster_id_t> >                 OccurrencesSequence;
  map<set<cluster_id_t>, size_t, setSizeCmp> SetAppearances;
  map<cluster_id_t, size_t>                  SingleAppearances;
  // map<pair<cluster_id_t, cluster_id_t>, size_t> PairApparences;
  vector<set<cluster_id_t> > Merges;


  /* Update Statistics */
  Statistics.InitStatistics(LastPartition.GetIDs());

  if (!Statistics.ComputeStatistics(Bursts,
                                    LastPartition.GetAssignmentVector()))
  {
    SetErrorMessage(Statistics.GetLastError());
    return false;
  }

  Scoring.ComputeScore(Bursts,
                       LastPartition.GetAssignmentVector(),
                       PercentageDurations,
                       CurrentClustersScores,
                       GlobalScore,
                       false, /* Print scores of last partitions! */
                       CurrentSequenceFileNamePrefix.str(),
                       true);


  vector<vector<cluster_id_t> >& Sequences = Scoring.GetSequencesMatrix();

  NumberOfSequences = Sequences.size();
  if (NumberOfSequences == 0)
  {
    SetErrorMessage("empty sequences when computing last partition");
    SetError(true);
    return false;
  }

  /* Check each position in the sequences to detect the simultaneous clusters */
  SequencesLength = Sequences[0].size();

  OccurrencesSequence = vector<set<cluster_id_t> > (SequencesLength);

  for (size_t i = 0; i < SequencesLength; i++)
  { /* i = sequence position */

    set<pair<cluster_id_t, cluster_id_t> > CurrentPairings;
    set<cluster_id_t>                      CurrentAppearances;
    set<cluster_id_t>::iterator            AppIt;
    map<cluster_id_t, double>::iterator    OccurrencesIt;

    for (size_t j = 0; j < NumberOfSequences; j++)
    { /* j = number of sequence */
      if (Sequences[j][i] != NOISE_CLUSTERID)
      {
        OccurrencesSequence[i].insert(Sequences[j][i]);
      }
    }
  }

  for (size_t i = 0; i < SequencesLength; i++)
  {
    // set<cluster_id_t>::iterator OccurrencesIt, OccurrencesInnIt;
    set<set<cluster_id_t> >           AllSubsets;
    set<set<cluster_id_t> >::iterator AllSubsetsIt;

    // cluster_id_t FirstClusterID, OtherClusterID;

    AllSubsets = getSubsets(OccurrencesSequence[i],
                            OccurrencesSequence[i].begin());

    for (AllSubsetsIt  = AllSubsets.begin();
         AllSubsetsIt != AllSubsets.end();
         ++AllSubsetsIt)
    {
      const set<cluster_id_t>& CurrentSet = (*AllSubsetsIt);

      if (CurrentSet.size() == 0)
      {
        continue;
      }
      else if (CurrentSet.size() == 1)
      { /* Single element sets */
        ++SingleAppearances[(*CurrentSet.begin())];
      }
      else
      {
        ++SetAppearances[CurrentSet];
      }
    }

    /*
    cout << "Sequence[" << i << "] = ";

    for (OccurrencesIt  = OccurrencesSequence[i].begin();
         OccurrencesIt != OccurrencesSequence[i].end();
         ++OccurrencesIt)
    {
      FirstClusterID = (*OccurrencesIt);

      cout << " " << FirstClusterID;

      if (SingleAppearances.count(FirstClusterID) == 0)
      {
        SingleAppearances[FirstClusterID] = 1;
      }
      else
      {
        SingleAppearances[FirstClusterID]++;
      }



      for (OccurrencesInnIt   = OccurrencesIt;
           OccurrencesInnIt  != OccurrencesSequence[i].end();
           ++OccurrencesInnIt)
      {
        if (OccurrencesInnIt != OccurrencesIt)
        {
          OtherClusterID = (*OccurrencesInnIt);

          if (FirstClusterID != NOISE_CLUSTERID)
          {
            if (PairApparences.count(make_pair(FirstClusterID, OtherClusterID)) == 0)
            {
              PairApparences[make_pair(FirstClusterID, OtherClusterID)] = 1;
            }
            else
            {
              PairApparences[make_pair(FirstClusterID, OtherClusterID)]++;
            }
          }
        }
      }
    }
    cout << endl;
    */
  }


  /*
  for (map<cluster_id_t, size_t>::iterator It = SingleAppearances.begin();
       It != SingleAppearances.end();
       ++It)
  {
    cout << "Cluster " << (*It).first << " = " << (*It).second << endl;
  }
  */

  /* Generate all merge sets */
  map<set<cluster_id_t>, size_t>::iterator SetsIt;
  for (SetsIt  = SetAppearances.begin();
       SetsIt != SetAppearances.end();
       ++SetsIt)
  {
    const set<cluster_id_t>& CurrentSet         = (*SetsIt).first;
    size_t                   CurrentAppearances = (*SetsIt).second;
    bool                     MergeCurrentSet    = true;

    if (CurrentSet.size() == 1)
    { /* No more sets to merge */
      break;
    }

    /* First check: all clusters in the set have the same number of occurrences
       isolately */
    // cout << "Set = {";
    for (set<cluster_id_t>::iterator i  = CurrentSet.begin();
                                     i != CurrentSet.end();
                                   ++i)
    {
      // cout << " " << (*i);
      if ( SingleAppearances[(*i)] != CurrentAppearances )
      {
        MergeCurrentSet = false;
      }
    }
    // cout << " } Occ = " << CurrentAppearances;

    /* Second check: there are no previous merge set that includes the current
       one */
    if (MergeCurrentSet)
    {
      for (size_t i = 0; i < Merges.size(); i++)
      {
        set<cluster_id_t> Intersection;

        set_intersection(Merges[i].begin(),  Merges[i].end(),
                         CurrentSet.begin(), CurrentSet.end(),
                         inserter(Intersection, Intersection.begin()));

        if (Intersection.size() == CurrentSet.size())
        {
          MergeCurrentSet = false;
        }
      }
    }

    if (MergeCurrentSet)
    {
      // cout << " MERGE!" << endl;
      Merges.push_back(CurrentSet);
    }
    else
    {
      // cout << " NO MERGE!" << endl;
    }
  }

  // cout << "Total merges = " << Merges.size() << endl;

  Messages << "|-> Merging clusters " << endl;
  system_messages::information(Messages.str());
  for (size_t i = 0; i < Merges.size(); i++)
  {
    cluster_id_t      MainClusterID = (*(Merges[i]).begin());
    set<cluster_id_t> MergeSet      = set<cluster_id_t>(++(Merges[i].begin()), Merges[i].end());

    Messages.str("");
    Messages << "|--->";

    for (set<cluster_id_t>::iterator MergeIt  = Merges[i].begin();
                                     MergeIt != Merges[i].end();
                                   ++MergeIt)
    {
      Messages << " " << (*MergeIt);
    }
    Messages << endl;
    system_messages::information(Messages.str());

    LastPartition.MergeIDs(MergeSet, MainClusterID);
  }

  vector<ClusterInformation*> NewNodes;

  GenerateNodes(0, Bursts, LastPartition, NewNodes, true); // true => indicate last partition nodes

  Messages.str("");
  Messages << "|-> Merging Tree Nodes " << endl;
  system_messages::information(Messages.str());

  vector<ClusterInformation*> LastLevelNodes;

  for (size_t i = 0; i < Merges.size(); i++)
  {
    cluster_id_t        MainClusterID = (*Merges[i].begin());
    set<cluster_id_t>   MergeSet = set<cluster_id_t>(++(Merges[i].begin()), Merges[i].end());
    ClusterInformation* MergeNode = NULL;

    for (size_t i = 0; i < NewNodes.size(); i++)
    {
      if (NewNodes[i]->GetID() == MainClusterID)
      {
        MergeNode = NewNodes[i];
        break;
      }
    }

    LinkLastNodes(MainClusterID, MergeSet, MergeNode);
    LastLevelNodes.push_back(MergeNode);
  }


  NodesPerLevel.push_back(LastLevelNodes);

  LastPartition.clear();
  GeneratePartition(LastPartition);

  if (!ComputeScores(0, Bursts, LastLevelNodes, LastPartition, true))
  {
    return false;
  }

  Messages.str("");
  Messages << "|-> Printing last trees" << endl;
  system_messages::information(Messages.str());

  if (!PrintTrees(LastStep, true)) /* Last tree */
  {
    return false;
  }

  return true;
}


/**
 * Merges the nodes in the tree resulting from the sequence based merge
 *
 * \param MainClusterID Cluster ID of the parent with lower ID
 * \param Merges        Set of cluster ids that got merge
 * \param NewNode       Node that defines the merge
 */
void ClusteringRefinementAggregative::LinkLastNodes(cluster_id_t        MainClusterID,
                                                    set<cluster_id_t>&  Merges,
                                                    ClusterInformation* NewNode)
{
  vector<ClusterInformation*> Parents;
  set<cluster_id_t>::iterator MergesIt;

  ClusterInformation* MasterParent = LocateNode(MainClusterID);
  Parents.push_back(MasterParent);

  MasterParent->AddChild(NewNode);

  for (MergesIt  = Merges.begin();
       MergesIt != Merges.end();
       ++MergesIt)
  {
    ClusterInformation* Node = LocateNode((*MergesIt));

    Node->AddChild(NewNode);
  }

  return;
}

/**
 * Locate a node with a the given cluster ID in the deepest level
 *
 * \param  ClusterID The cluster ID to be located into the tree
 *
 * \return A pointer to the node with the given cluster ID in the deepest level
 *         of a tree, NULL if doesn't exist
 */
ClusterInformation* ClusteringRefinementAggregative::LocateNode(cluster_id_t ClusterID)
{

  for (size_t i = NodesPerLevel.size()-1; i >= 0; i--)
  {
    for (size_t j = 0; j < NodesPerLevel[i].size(); j++)
    {
      ClusterInformation* Node = NodesPerLevel[i][j];

      if (Node->GetID() == ClusterID)
      {
        return Node;
      }
    }
  }
  return NULL;
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
bool ClusteringRefinementAggregative::PrintPlots(const vector<CPUBurst*>& Bursts,
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
                         CurrentPartition.GetIDs()))
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
 * \param Step    Current step to be printed
 * \param Epsilon Epsilon used in the current
 *
 * \return True if the tree was written correctly, false otherwise
 *
 */
bool ClusteringRefinementAggregative::PrintTrees(size_t Step,
                                                 bool   LastTree)
{
  ostringstream TreeFileName;
  ofstream     *Output;

  vector<ClusterInformation*>& TopLevelNodes = NodesPerLevel[0];
  vector<string>               LevelNames;

  if (LastTree)
  {
    TreeFileName << OutputFilePrefix << ".TREE.dot";
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
    if (PrintStepsInformation)
    {

      TreeFileName << OutputFilePrefix << ".STEP" << (Step+1) << ".TREE.dot";
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
  }

  (*Output) << "digraph Tree {" << endl;

  /* Level Information */
  (*Output) << endl << "{" << endl;
  (*Output) << "node [shape=plaintext]" << endl;

  for (size_t i = 0; i <= Step; i++)
  {
    ostringstream LevelName;

    if (LastTree && i == Steps)
    {
      (*Output) << "\"SEQUENCE BASED MERGE | Global Score =";
      (*Output) << GlobalScoresPerLevel[i]*100 << "% \"" << endl;
    }
    else
    {
      (*Output) << "\"STEP " << i+1 << " Eps = " << EpsilonPerLevel[i];
      (*Output) << " | Global Score = " << GlobalScoresPerLevel[i]*100 << "% \"";
      // LevelNames.push_back(LevelName.str());

      (*Output) << LevelName.str();
    }

    if (i != Step)
    {
      (*Output) << "->";
    }
  }
  (*Output) << ";" << endl;
  (*Output) << "}" << endl;

  /* Print the tree itself */
  (*Output) << "{" << endl;

  /*
  for (size_t i = 0; i < NodesPerLevel.size(); i++)
  {
    vector<ClusterInformation*>& CurrentLevelNodes = NodesPerLevel[i];

    (*Output) << "subgraph cluster" << i+1 << " {" << endl;
    for (size_t j = 0; j < CurrentLevelNodes.size(); j++)
    {
      ClusterInformation* Node = CurrentLevelNodes[j];

      if (Node->GetID() != NOISE_CLUSTERID || Node->GetIndividuals() != 0)
      {
        (*Output) << Node->NodeName() << " " << Node->NodeLabel() << ";" << endl;
      }
    }
    (*Output) << "label = \"Step " << i+1 << " Epsilon = ";
    (*Output) << EpsilonPerLevel[i] << "\"" << endl;
    (*Output) << "}" << endl;
  }
  */

  /*
  for (size_t i = 0; i < TopLevelNodes.size(); i++)
  {
  */
    PrintTreeNodes ((*Output));
    (*Output) << endl;
    PrintTreeLinks ((*Output));
    (*Output) << endl;
  // }
  (*Output) << "}" << endl;

  /* Rank nodes per level
  (*Output) << "{" << endl;

  for (size_t i = 0; i <= Step; i++)
  {
    vector<ClusterInformation*>& LevelNodes = NodesPerLevel[i];

    (*Output) << "{ rank = same; " << LevelNames[i];
    for (size_t j = 0; j < LevelNodes.size(); j++)
    {
      ClusterInformation* Node = LevelNodes[j];
      if (Node->GetID() != NOISE_CLUSTERID || Node->GetIndividuals() != 0)
      {
        (*Output) << "; " << Node->GetNodeID();
      }
    }
    (*Output) << " }" << endl;
  }

  (*Output) << "}" << endl;
  */

  (*Output) << "}" << endl << endl;

  /* DEBUG
  cout << "TOP LEVEL HAS " << ClustersHierarchy[0].size() << " NODES" << endl; */

  Output->close();

  return true;
}

bool ClusteringRefinementAggregative::PrintTreeNodes(ofstream& str)
{


  for (size_t i = 0; i < NodesPerLevel.size(); i++)
  {
    str << "// Level " << i << endl;

    str << "{rank=same; ";
    for (size_t j = 0; j < NodesPerLevel[i].size(); j++)
    {
      ClusterInformation* Node = NodesPerLevel[i][j];

      if (Node->GetID() != NOISE_CLUSTERID || Node->GetIndividuals() != 0)
      {
        str << Node->NodeName() << ";";
      }
    }
    str << "}" << endl;

    for (size_t j = 0; j < NodesPerLevel[i].size(); j++)
    {
      ClusterInformation* Node = NodesPerLevel[i][j];

      if (Node->GetID() != NOISE_CLUSTERID || Node->GetIndividuals() != 0)
      {
        str << Node->NodeName() << " " << Node->NodeLabel() << ";" << endl;
      }
    }
  }

  /*
  vector<ClusterInformation*>& Children = Node->GetChildren();

  if (Node->GetID() != NOISE_CLUSTERID || Node->GetIndividuals() != 0)
  {
    str << Node->NodeName() << " " << Node->NodeLabel() << ";" << endl;

    for (size_t i = 0; i < Children.size(); i++)
    {
      PrintTreeNodes(str, Children[i]);
    }
  }
  */

  return true;
}

bool ClusteringRefinementAggregative::PrintTreeLinks(ofstream& str)
{
  for (size_t i = 0; i < NodesPerLevel.size(); i++)
  {
    for (size_t j = 0; j < NodesPerLevel[i].size(); j++)
    {
      ClusterInformation* Node = NodesPerLevel[i][j];

      if (Node->GetID() != NOISE_CLUSTERID || Node->GetIndividuals() != 0)
      {
        vector<ClusterInformation*>& Children = Node->GetChildren();

        for (size_t k = 0; k < Children.size(); k++)
        {
          str << "\"" << Node->NodeName() << "\" ->  \"" << Children[k]->NodeName() << "\"";
          str << ";" << endl;
        }
      }
    }
  }
  /*
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
  */

  return true;
}

set<set<cluster_id_t> > ClusteringRefinementAggregative::getSubsets(set<cluster_id_t>&       in_set,
                                                                 set<cluster_id_t>::iterator index)

{
  set <set <cluster_id_t> > allsubsets;

  if(index != in_set.end())
  {
    cluster_id_t item = (*index);

    allsubsets = getSubsets(in_set, ++index);   //get subsets for set[index+1:n]

    set<set<cluster_id_t> >           moresubsets;
    set<set<cluster_id_t> >::iterator i = allsubsets.begin();

    do
    {
      set<cluster_id_t> newsubset;
      for(set<cluster_id_t>::iterator j  = (*i).begin();
                                      j != (*i).end();
                                    ++j)
      {
        //make a copy of each of the subsets obtained for set[index+1:n]
        newsubset.insert(*j);
      }

      newsubset.insert(item);       //add set[index] to each one of them
      moresubsets.insert(newsubset);
      ++i;

    } while (i != allsubsets.end());

    for( set<set<cluster_id_t> >::iterator  i  = moresubsets.begin();
                                            i != moresubsets.end();
                                          ++i)
    {
      //now this is the new set of subsets!!
      allsubsets.insert(*i);
    }

    return allsubsets;
  }

  std::set<cluster_id_t> empty;
  allsubsets.insert(empty);         //empty value acts as NULL for subsets
  return allsubsets;
}

