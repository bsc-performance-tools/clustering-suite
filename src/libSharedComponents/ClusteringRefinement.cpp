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

#include "ClusteringRefinement.hpp"
#include "SequenceScore.hpp"

#include "PlottingManager.hpp"

#include <DBSCAN.hpp>

#include <sstream>
using std::ostringstream;
using std::setfill;
using std::setw;
using std::setprecision;

/* PARAMETERS TO TUNE CANDIDATE GENERATION */
#define CANDIDATES_GENERAL_SCORE 0.9

#define CANDIDATES_OCCURENCES_PERCENTAGE 0.25
#define CANDIDATES_OCCURENCES_SCORE 0.6

#define CANDIDATES_TIME_PERCENTAGE 0.25
#define CANDIDATES_TIME_SCORE      0.6

/******************************************************************************
 * CLASS 'ClusterInformation'
 *****************************************************************************/

ClusterInformation::ClusterInformation(cluster_id_t ID,
                                       percentage_t Score,
                                       size_t       Occurrences,
                                       timestamp_t  TotalDuration,
                                       size_t       Individuals)
{
  this->ID            = ID;
  this->Score         = Score;
  this->Occurrences   = Occurrences;
  this->TotalDuration = TotalDuration;
  this->Individuals   = Individuals;
  this->Discarded     = false;
}

bool ClusterInformation::IsCandidate(size_t      TotalOccurrences,
                                     timestamp_t ClustersTotalDuration)
{
  if (Discarded)
  {
    return false;
  }
  
  if (Occurrences > 1 && Score > CANDIDATES_GENERAL_SCORE)
  {
    return true;
  }

  if ( (TotalDuration/ClustersTotalDuration) > CANDIDATES_TIME_PERCENTAGE &&
       Score > CANDIDATES_TIME_SCORE)
  {
    return true;
  }
  
  if ( (Occurrences/TotalOccurrences) > CANDIDATES_OCCURENCES_PERCENTAGE &&
        Score > CANDIDATES_OCCURENCES_SCORE)
  {
    return true;
  }
    
  return false;
}

bool ClusterInformation::AddChild(ClusterInformation* NewChild)
{
  Children.push_back(NewChild);
  
  return true;
}

/******************************************************************************
 * CLASS 'ClusteringRefinement'
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
ClusteringRefinement::ClusteringRefinement(INT32      MinPoints,
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
bool ClusteringRefinement::Run(const vector<CPUBurst*>& Bursts,
                               vector<Partition>&       ResultingPartitions,
                               string                   OutputFilePrefix)
{
  bool           Convergence = false, NoMoreClusters = false;
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
  
  if (Steps <= 1)
  {
    SetErrorMessage("number of steps in a refinement should be higher than 1");
    return false;
  }

  PartitionsHistory = vector<Partition> (Steps);
  ClustersHierarchy = vector<set<ClusterInformation*> > (Steps);
  StatisticsHistory = vector<ClusteringStatistics> (Steps);
  BurstsPerStep     = vector<vector<CPUBurst*> > (Steps);
  
  Epsilons.push_back(MaxEpsilon);
  for (size_t i = 1; i < Steps; i++)
  {
    Epsilons.push_back(Epsilons[i-1] - StepSize);
  }

  ClusteringCore = new libClustering();

  /* Execute initial clustering */
  BurstsPerStep[0] = Bursts;
  
  Messages << "*** STEP 1 (Eps = " << Epsilons[0] << ") ***" << endl;
  system_messages::information(Messages.str());  
  if (!RunStep(0, Epsilons[0], NoMoreClusters))
  {
    return false;
  }

  /* On step 0, all new nodes are part of the hierarchy */
  map<cluster_id_t, ClusterInformation*>::iterator NewNodesIt;

  /* DEBUG 
  cout << "Inserting top level nodes "; */
  for (NewNodesIt  = NewNodes.begin();
       NewNodesIt != NewNodes.end();
       ++NewNodesIt)
  {
    /* DEBUG 
    cout << NewNodesIt->second->GetID() << " "; */
    ClustersHierarchy[0].insert((NewNodesIt->second));
  }
  // cout << endl;

  LastStep = 0;

  /* Iterate through the epsilon values, and generate the clusters hierarchy */
  for (size_t CurrentStep = 1; 
       CurrentStep < Steps && !Convergence && !NoMoreClusters;
       CurrentStep++)
  {
    double CurrentEpsilon = Epsilons[CurrentStep];

    Messages.str("");
    
    Messages << "****** STEP " << (CurrentStep+1) << " (Eps = " << CurrentEpsilon;
    Messages << ") ******" << endl;
    system_messages::information(Messages.str());
    
    if (!RunStep(CurrentStep,
                 CurrentEpsilon,
                 NoMoreClusters))
    {
      return false;
    }

    Messages.str("");
    
    Messages << "-> Linking to previous step" << endl;
    system_messages::information(Messages.str());

    Convergence = false;
    if (!LinkToPreviuosStep(CurrentStep, Convergence))
    {
      return false;
    }

    if (!Convergence)
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

  /* DEBUG 
  cout << "My Last Step was = " << LastStep << endl; */
  
  /* Create the partitions and statistics for all steps */
  if (!CreateDefinitivePartitions(ResultingPartitions))
  {
    return false;
  }
  
  PrintTrees ();
  
  return true;
}

/**
 * Executes the i-th step in the refinement analysis
 *
 * \param Step The number of step to execute
 * \param Epsilon Value of epsilon to be used in this step
 * \param NoMoreClusters I/O parameter to indicate that no more clusters were generated
 *
 * \return True if the step was executed correctly, false otherwise
 *
 */ 
bool ClusteringRefinement::RunStep(size_t Step,
                                   double Epsilon,
                                   bool&  NoMoreClusters)
{
  ostringstream  Messages;
  
  /* Prepare the Bursts */

  Messages.str("");
    
  Messages << "-> Generating candidates" << endl;
  system_messages::information(Messages.str());
  
  if (Step == 0)
  {
    for (size_t i = 0; i < BurstsPerStep[0].size(); i++)
    {
      InstancesTracking[BurstsPerStep[0][i]->GetInstance()].push_back(i);
    }
  }
  else
  {
    GenerateCandidates(Step);
  }

  if (BurstsPerStep[Step].size() == 0)
  {
    NoMoreClusters = true;
    return true;
  }
  
  
  /* Run DBSCAN */
  Messages.str("");
    
  Messages << "-> Running DBSCAN" << endl;
  system_messages::information(Messages.str());
  if (!RunDBSCAN ((vector<const Point*>&) BurstsPerStep[Step],
                                          Epsilon,
                                          PartitionsHistory[Step]))
  {
    return false;
  }

  /* Create current level nodes */
  Messages.str("");
    
  Messages << "-> Generating resulting nodes" << endl;
  system_messages::information(Messages.str());
  
  if (!GenerateCurrentLevelNodes(Step))
  {
    return false;
  }

  /* Print plots */
  if (PrintStepsInformation)
  {
      Messages.str("");
    
    Messages << "-> Printing plots" << endl;
    system_messages::information(Messages.str());
    
    if (!PrintPlots(BurstsPerStep[Step],
                    PartitionsHistory[Step],
                    Step))
      return false;
  }
  
  return true;
}

/**
 * Links the clusters of the i-the step with the clusters obtained in the previous
 * one.
 *
 * \param Step Number of step to be analyzed
 * \param Convergence I/O parameter to indicate that current step and the previous converge
 *
 * \return True if the step was executed correctly, false otherwise
 *
 */ 
bool ClusteringRefinement::LinkToPreviuosStep(size_t Step,
                                              bool&  Convergence)
{
  SequenceScore              Scoring;
  vector<SequenceScoreValue> CurrentClustersScores;
  double                     GlobalScore;
  vector<percentage_t>       PercentageDurations;
  vector<double>             CurrentClustersDurations;
  vector<size_t>             CurrentClustersIndividuals;

  vector<CPUBurst*>& CurrentBursts = BurstsPerStep[Step];
  Partition& CurrentPartition      = PartitionsHistory[Step];
  
  if (Step == 0)
  {
    SetErrorMessage("unable to link first level on the hierarchy");
    SetError(true);
    return false;
  }

  /* Relationships between levels */
  map<cluster_id_t, set<cluster_id_t> > Relationships;

  /* Noise accounting */
  map<cluster_id_t, timestamp_t> NoisesDuration;
  map<cluster_id_t, size_t>      NoisesIndividuals;

  for (size_t i = 0; i < OngoingCandidates.size(); i++)
  {
    NoisesDuration[OngoingCandidates[i]->GetID()]    = 0;
    NoisesIndividuals[OngoingCandidates[i]->GetID()] = 0;

    // Relationships[OngoingCandidates[i]->GetID()]
  }

  Partition& PreviousPartition = PartitionsHistory[Step-1];
    
  vector<cluster_id_t>& CurrentAssignment = 
    CurrentPartition.GetAssignmentVector();

  vector<cluster_id_t>& PreviousAssignment =
    PreviousPartition.GetAssignmentVector();

  /* Generate the relationships */
  for (size_t i = 0; i < CurrentBursts.size(); i++)
  {
    size_t CurrentBurstPosition  = 
      InstancesTracking[CurrentBursts[i]->GetInstance()][Step];

    cluster_id_t CurrentID = CurrentAssignment[CurrentBurstPosition];

    size_t PreviousBurstPosition = 
      InstancesTracking[CurrentBursts[i]->GetInstance()][Step-1];

    cluster_id_t PreviousID = PreviousAssignment[PreviousBurstPosition];
    
    if (CurrentID == NOISE_CLUSTERID)
    {
      NoisesDuration[PreviousID] += CurrentBursts[i]->GetDuration();
      NoisesIndividuals[PreviousID]++;
    }
    else
    {
      Relationships[PreviousID].insert(CurrentID);
    }
  }

  /* Update levels */
  Convergence = true;
  
  for (size_t i = 0; i < OngoingCandidates.size(); i++)
  {
    bool SplitOK;
    
    ClusterInformation* ParentNode = OngoingCandidates[i];
    cluster_id_t        ParentID   = ParentNode->GetID();

    set<cluster_id_t>&          ChildrenIDs = Relationships[ParentID];
    set<cluster_id_t>::iterator ChildrenIDsIterator;
    vector<ClusterInformation*> ChildrenNodes;

    /* Fill the children vector */
    /* First, noise cluster */
    ChildrenNodes.push_back(new ClusterInformation(NOISE_CLUSTERID,
                                                   0.0,
                                                   0,
                                                   NoisesDuration[ParentID],
                                                   NoisesIndividuals[ParentID]));

    /* The rest of children */
    for (ChildrenIDsIterator  = ChildrenIDs.begin();
         ChildrenIDsIterator != ChildrenIDs.end();
         ++ChildrenIDsIterator)
    {
      cluster_id_t CurrentChildID = (*ChildrenIDsIterator);
      ChildrenNodes.push_back(NewNodes[CurrentChildID]);
    }

    /* Check if the split was OK */
    if (!EvaluateSplit(ParentNode, ChildrenNodes, SplitOK))
    {
      return false;
    }
    
    if (SplitOK)
    { /* Add children to parent node and current level hierarchy */
      Convergence = false;

      /* DEBUG 
      cout << "ID " << ParentNode->GetID() << " Splits on ";
      for (ChildrenIDsIterator  = ChildrenIDs.begin();
         ChildrenIDsIterator != ChildrenIDs.end();
         ++ChildrenIDsIterator)
      {
        cluster_id_t CurrentChildID = (*ChildrenIDsIterator);
        // cout << CurrentChildID << " ";
      }
      cout << endl; */
      
      for (size_t j = 0; j < ChildrenNodes.size(); j++)
      {
        ParentNode->AddChild(ChildrenNodes[j]);

        if (ChildrenNodes[j]->GetID() != NOISE_CLUSTERID)
        {
          ClustersHierarchy[Step].insert(ChildrenNodes[j]);
        }
      }
    }
    else
    {
      /* Delete children nodes */
      /* EXPERIMENTAL inserted but discarded */
      for (size_t j = 0; j < ChildrenNodes.size(); j++)
      {
        // delete ChildrenNodes[j];
        ChildrenNodes[j]->Discard();

        /* DEBUG */
        cout << "Discarded ID " << ChildrenNodes[j]->GetID() << " on Level " << Step+1 << endl;
        
        ParentNode->AddChild(ChildrenNodes[j]);
      }

      /* Special case for those cluster that became all NOISE 
      if(ChildrenNodes.size() == 1)
      {
        ReassignNoisePartition(ParentNode->GetID(), Step);
      }
      */
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
bool ClusteringRefinement::RunDBSCAN(const vector<const Point*>& CurrentData,
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

  if (!ClusteringCore->ExecuteClustering(CurrentData,
                                         CurrentPartition))
  {
    SetError(true);
    SetErrorMessage(ClusteringCore->GetErrorMessage());
    return false;
  }
  
  return true;
}

/**
 * Generates the candidates of the current step, evaluating the clusters obtained
 * in the previous one.
 *
 * \param Step Number of step to be analyzed
 *
 * \return True if the step was executed correctly, false otherwise
 *
 */ 
bool ClusteringRefinement::GenerateCandidates(size_t Step)
{
  size_t TotalOccurrences           = 0;
  timestamp_t TotalClustersDuration = 0;
  
  set<ClusterInformation*>& PreviousStepClustersInfo = ClustersHierarchy[Step-1];
  set<ClusterInformation*>::iterator PreviousStepClustersInfoIt;
  
  vector<CPUBurst*>& PreviousStepBursts = BurstsPerStep[Step-1];
  vector<CPUBurst*>& CurrentStepBursts  = BurstsPerStep[Step];

  set<cluster_id_t> CandidateIDs;

  OngoingCandidates.clear();
  
  /* Chose the cluster candidates */
  // for (size_t i = 0; i < PreviousStepClustersInfo.size(); i++)
  for (PreviousStepClustersInfoIt  = PreviousStepClustersInfo.begin();
       PreviousStepClustersInfoIt != PreviousStepClustersInfo.end();
       ++PreviousStepClustersInfoIt)
  {
    // ClusterInformation* Node = PreviousStepClustersInfo[i];
    ClusterInformation* Node = (*PreviousStepClustersInfoIt);
    
    TotalOccurrences      += Node->GetTotalDuration();
    TotalClustersDuration += Node->GetOccurrences();
  }
  
  // for (size_t i = 0; i < PreviousStepClustersInfo.size(); i++)
  for (PreviousStepClustersInfoIt  = PreviousStepClustersInfo.begin();
       PreviousStepClustersInfoIt != PreviousStepClustersInfo.end();
       ++PreviousStepClustersInfoIt)
  {
    // ClusterInformation* Node = PreviousStepClustersInfo[i];
    ClusterInformation* Node = (*PreviousStepClustersInfoIt);
    if(Node->GetID() != NOISE_CLUSTERID)
    {
      if(Node->IsCandidate(TotalOccurrences,
                           TotalClustersDuration))
      {
        OngoingCandidates.push_back(Node);
        CandidateIDs.insert(Node->GetID());
      }
    }
  }

  /* Generate Candidate Bursts vector */
  vector<cluster_id_t>& PreviuousAssignment =
    PartitionsHistory[Step-1].GetAssignmentVector();
  
  for (size_t i = 0; i < PreviousStepBursts.size(); i++)
  {
    CPUBurst* Burst = PreviousStepBursts[i];
    
    cluster_id_t PreviousID = 
      PreviuousAssignment[InstancesTracking[Burst->GetInstance()][Step-1]];

    if(IsIDInSet(PreviousID, CandidateIDs))
    {
      CurrentStepBursts.push_back(Burst);

      size_t CurrentPosition = CurrentStepBursts.size()-1;

      /* Update instances tracking */
      InstancesTracking[Burst->GetInstance()].push_back(CurrentPosition);
    }
  }

  return true;
}

/**
 * Generate the nodes that describe the clusters obtained in the current step.
 * It generates the cluster statistics, renaming the clusters depending on
 * their duration, and also compute the scores of each cluster
 *
 * \param Step Number of step to be analyzed
 *
 * \return True if the step was executed correctly, false otherwise
 *
 */ 
bool ClusteringRefinement::GenerateCurrentLevelNodes(size_t Step)
{
  SequenceScore              Scoring;
  vector<SequenceScoreValue> CurrentClustersScores;
  double                     GlobalScore;
  vector<percentage_t>       PercentageDurations;

  ostringstream  Messages;

  Partition&            CurrentPartition  = PartitionsHistory[Step];
  ClusteringStatistics& CurrentStatistics = StatisticsHistory[Step];

  vector<CPUBurst*>& CurrentBursts = BurstsPerStep[Step];

  /* Update Statistics */
  CurrentStatistics.InitStatistics(CurrentPartition.GetIDs(),
                                   CurrentPartition.HasNoise());

  Messages.str("");
  Messages << "|---> Computing statistics" << endl;
  system_messages::information(Messages.str());
  if (!CurrentStatistics.ComputeStatistics(CurrentBursts,
                                           CurrentPartition.GetAssignmentVector()))
  {
    SetErrorMessage(CurrentStatistics.GetLastError());
    return false;
  }
  
  CurrentStatistics.TranslatedIDs(CurrentPartition.GetAssignmentVector());

  /* Compute Scores */
  ostringstream SequencesFileNamePrefix;
  
  if (PrintStepsInformation)
  {
    SequencesFileNamePrefix << OutputFilePrefix << ".STEP" << Step+1;
  }
  else
  {
    SequencesFileNamePrefix.str("");
  }
  
  PercentageDurations = StatisticsHistory[Step].GetPercentageDurations(); 

  Messages.str("");
  Messages << "|---> Computing score" << endl;
  system_messages::information(Messages.str());
  
  Scoring.ComputeScore((const vector<CPUBurst*>&) CurrentBursts,
                       CurrentPartition.GetAssignmentVector(),
                       PercentageDurations,
                       CurrentPartition.HasNoise(),
                       CurrentClustersScores,
                       GlobalScore,
                       SequencesFileNamePrefix.str(),
                       true);

  /* Generate current level hierarchy */
  vector<double> CurrentClustersDurations   = StatisticsHistory[Step].GetDurationSums();
  vector<size_t> CurrentClustersIndividuals = StatisticsHistory[Step].GetIndividuals();

  NewNodes.clear();
  
  for (size_t i = 0; i < CurrentPartition.NumberOfClusters(); i++)
  {
    if (i != NOISE_CLUSTERID || Step == 0)
    {
      ClusterInformation* NewNode = 
        new ClusterInformation((cluster_id_t) i,
                               CurrentClustersScores[i].GetClusterScore(),
                               CurrentClustersScores[i].GetOccurrences(),
                               CurrentClustersDurations[i],
                               CurrentClustersIndividuals[i]);

      NewNodes[i] = NewNode;
    }
  }
  
  return true;
}

/**
 * Compares the result of two steps, using ClusterInformation objects, to 
 * decide if it is better to split the cluster of keep it as a single one
 *
 * \param Parent Node that contains the information of the parent in the hierarchy
 * \param Children Nodes that contain the information of the different children
 *        obtained applying a more restricted value of epsilon
 * \param SplitOK I/O parameter to show if it is worth to split the parent cluster
 *        or not
 *
 * \return True if the step was executed correctly, false otherwise
 *
 */ 
bool ClusteringRefinement::EvaluateSplit(ClusterInformation*          Parent,
                                         vector<ClusterInformation*>& Children,
                                         bool&                        SplitOK)
{
  timestamp_t          ParentDuration = 0, NoiseDuration = 0;
  timestamp_t          TotalChildrenDuration = 0;
  size_t               TotalChildrenOccurrences = 0;
  percentage_t         ChildrenWeightedScore = 0, ParentWeightedScore = 0;

  if (Children.size() == 0)
  {
    return false;
  }

  if (Children.size() == 1)
  { /* Everything turn into noise */
    SplitOK = false;
    return true;
  }

  ParentDuration = Parent->GetTotalDuration();
  
  NoiseDuration = Children[0]->GetTotalDuration();

  /* Noise >= 60% of parent duration. CHECK PERCENTAGE TO REFINE */
  if (NoiseDuration >= 0.6*ParentDuration)
  {
    SplitOK = false;
    return true;
  }

  for (size_t i = 1; i < Children.size(); i++)
  {
    TotalChildrenDuration += Children[i]->GetTotalDuration();
    ChildrenWeightedScore += (Children[i]->GetScore()*Children[i]->GetTotalDuration());
  }

  ChildrenWeightedScore /= TotalChildrenDuration;

  /* Weighted children score < 80% of parent score */
  if (ChildrenWeightedScore < (0.8*Parent->GetScore()))
  {
    SplitOK = false;
    return true;
  }

  SplitOK = true;
  return true;
}

/**
 * Reassigns the IDs of the previous step, to those clusters that were split
 * into NOISE
 *
 * \param ID The value of the ID in the previous step
 * \param Step Step where the ID was transformed into NOISE
 *
 */
void ClusteringRefinement::ReassignNoisePartition(cluster_id_t ID,
                                                  size_t Step)
{
  Partition& PreviousPartition = PartitionsHistory[Step-1];
  Partition& CurrentPartition  = PartitionsHistory[Step];
  
  vector<cluster_id_t>& PreviousAssignement = PreviousPartition.GetAssignmentVector();
  vector<cluster_id_t>& CurrentAssignement  = CurrentPartition.GetAssignmentVector();

  set<cluster_id_t>& CurrentIDs = CurrentPartition.GetIDs();
  set<cluster_id_t>::iterator IDsIt;

  cluster_id_t MaxID = NOISE_CLUSTERID;

  for (IDsIt  = CurrentIDs.begin();
       IDsIt != CurrentIDs.end();
       ++IDsIt)
  {
    if ((*IDsIt) > MaxID)
    {
      MaxID = (*IDsIt);
    }
  }
  
  vector<CPUBurst*>& Bursts = BurstsPerStep[Step];

  for (size_t i = 0; i < Bursts.size(); i++)
  {
    size_t PreviousPosition = InstancesTracking[Bursts[i]->GetInstance()][Step-1];

    if (PreviousAssignement[PreviousPosition] == ID)
    {
      CurrentAssignement[i] = MaxID+1;
    }
  }

  /* Increase the number of clusters */
  size_t NumberOfClusters = CurrentPartition.NumberOfClusters();
  CurrentPartition.NumberOfClusters(NumberOfClusters+1);
  
  return;
}

/**
 * Generates the 'full partitions' for each step (level) traversing the clusters 
 * tree obtained after the different runs
 *
 * \return True if the methods was executed correctly, false otherwise
 *
 */  
bool ClusteringRefinement::CreateDefinitivePartitions(vector<Partition>& ResultingPartitions)
{
  ResultingPartitions    = vector<Partition> (LastStep+1);
  ResultingPartitions[0] = PartitionsHistory[0];
  
  set<ClusterInformation*>::iterator ClustersInfoIt;
  
  vector<CPUBurst*>& Bursts = BurstsPerStep[0];

  /* DEBUG 
  cout << "Last Step executed = " << LastStep << endl; */
  
  /* DEBUG 
  cout << "Bursts size = " << Bursts.size() << endl; */
  
  /* DEBUG 
  cout << __FUNCTION__ << "ResultingPartitions[0] NumberOfClusters = " << ResultingPartitions[0].NumberOfClusters() << endl;
  */
  
  for (ClustersInfoIt  = ClustersHierarchy[0].begin();
       ClustersInfoIt != ClustersHierarchy[0].end();
       ++ClustersInfoIt)
  {
    if ((*ClustersInfoIt)->GetID() != NOISE_CLUSTERID)
    {
      size_t TotalDivisions = ColapseNonDividedSubtrees((*ClustersInfoIt), 0);

      /* DEBUG 
      cout << "MASTER NODE " << (*ClustersInfoIt)->GetID() << " ";
      cout << "GENERATED " << TotalDivisions << " DIVISION(S)" << endl;
      */
    }
  }

  /*
  for (size_t i = 0; i < ClustersHierarchy[0].size(); i++)
  {
    if (ClustersHierarchy[0][i]->GetID() != NOISE_CLUSTERID)
    {
      ColapseNonDividedSubtrees(ClustersHierarchy[0][i], 0);
    }
  }
  */

  size_t PreviousNumberOfClusters = 0;
  
  for (size_t Step = 1; Step <= LastStep; Step++)
  {
    PreviousNumberOfClusters += ResultingPartitions[Step-1].NumberOfClusters();

    /* DEBUG
    cout << "Previous Number of Clusters = " << PreviousNumberOfClusters << endl;
    */
     
    vector<cluster_id_t>& PreviousIDs = ResultingPartitions[Step-1].GetAssignmentVector();
    vector<cluster_id_t>& ComputedIDs = PartitionsHistory[Step].GetAssignmentVector();
    vector<cluster_id_t>& OutputIDs   = ResultingPartitions[Step].GetAssignmentVector();
    
    set<cluster_id_t> OutputIDsSet;
    set<cluster_id_t>::iterator OutputIDsSetIt;

    /* DEBUG 
    cout << "PreviousIDs size = " << PreviousIDs.size() << endl; */

    OutputIDs.clear();
    OutputIDsSet.clear();

    set<cluster_id_t> StepAcceptedIDs;

    /* DEBUG
    cout << "Level " << Step+1 << " Accepted IDS = "; */
      
    for (ClustersInfoIt  = ClustersHierarchy[Step].begin();
         ClustersInfoIt != ClustersHierarchy[Step].end();
         ++ClustersInfoIt)
    {
      // cout << (*ClustersInfoIt)->GetID() << " ";
      StepAcceptedIDs.insert((*ClustersInfoIt)->GetID());
    }
    // cout << endl;

    /*
    for (size_t i = 0; i < ClustersHierarchy[Step].size(); i++)
    {
      StepAcceptedIDs.insert(ClustersHierarchy[Step][i]->GetID());
    }
    */

    OutputIDsSet.insert(NOISE_CLUSTERID);
    
    for (size_t i = 0; i < Bursts.size(); i++)
    {
      if(InstancesTracking[Bursts[i]->GetInstance()].size() <= Step)
      {
        /* Current burst was not used in this step */
                
        cluster_id_t PreviousID = PreviousIDs[i];
        OutputIDs.push_back(PreviousID);
        OutputIDsSet.insert(PreviousID);
      }
      else
      { 
        cluster_id_t CurrentID;
        size_t CurrentBurstPosition = InstancesTracking[Bursts[i]->GetInstance()][Step];

        CurrentID = ComputedIDs[CurrentBurstPosition];

        if (CurrentID == NOISE_CLUSTERID)
        {
          OutputIDs.push_back(NOISE_CLUSTERID);
        }
        else if (CurrentID == UNCLASSIFIED)
        {
          cluster_id_t PreviousID = PreviousIDs[i];

          OutputIDs.push_back(PreviousID);
          OutputIDsSet.insert(PreviousID);
        }
        else
        {
          if (IsIDInSet (CurrentID, StepAcceptedIDs))
          {
            /* DEBUG 
            cout << "Accepted ID -> " << CurrentID << endl; */
            OutputIDs.push_back(CurrentID+PreviousNumberOfClusters);
            OutputIDsSet.insert(CurrentID+PreviousNumberOfClusters);
          }
          else
          {
            cluster_id_t PreviousID = PreviousIDs[i];
            
            OutputIDs.push_back(PreviousID);
            OutputIDsSet.insert(PreviousID);
          }
        }
      }
    }

    /* DEBUG 
    cout << "Step " << Step+1 << " Assignment" << endl;
    for (size_t i = 0; i < Bursts.size(); i++)
    {
      cout << OutputIDs[i] << " ";
    }
    cout << endl;
    */

    ResultingPartitions[Step].HasNoise(true);
    ResultingPartitions[Step].NumberOfClusters(OutputIDsSet.size());
    ResultingPartitions[Step].SetIDs(OutputIDsSet);

    /* DEBUG 
    cout << "Different IDs in Step " << Step << " = ";
    for (OutputIDsSetIt  = OutputIDsSet.begin();
         OutputIDsSetIt != OutputIDsSet.end();
         ++OutputIDsSetIt)
    {
      cout << (*OutputIDsSetIt) << " ";
    }
    cout << endl;

    cout << __FUNCTION__ << "ResultingPartitions[" << Step;
    cout << "] NumberOfClusters = " << ResultingPartitions[Step].NumberOfClusters() << endl;
    */
    
  }
  
  return true;
}

/**
 * Collapses the current node if the subtree where it is root has no divisions.
 * It also add the non-colapsed children to the ClustersHierarchy vectors
 *
 * \param Node ClusterInformation node to be treated
 * \param Level The current level on the tree
 *
 * \return The total number of divisions in the subtree
 *
 */  
size_t ClusteringRefinement::ColapseNonDividedSubtrees(ClusterInformation* Node,
                                                       size_t              Level)
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
      NumberOfDivisions += ColapseNonDividedSubtrees(Children[i], Level+1);
    }
  }

  if (NumberOfDivisions == 1 || NumberOfDivisions == 0)
  { /* Delete the subtree! */

    /* DEBUG */
    cout << "** Collapsing node " << Node->GetID() << " on level " << Level+1;
    cout << " (" << Children[1] << ") **" << endl;

    ClustersHierarchy[Level+1].erase(Children[1]);

    /* EXPERIMENTAL, do not delete nodes, just mark them as discarded */
    Children[0]->Discard();
    if (NumberOfDivisions == 1)
    {
      Children[1]->Discard();
    }
    // delete Children[0];
    // delete Children[1];
    // Children.clear();

    /* Mark all assignment in next level as UNDEFINED */
    Partition& CurrentPartition = PartitionsHistory[Level];
    Partition& NextPartition    = PartitionsHistory[Level+1];
    
    vector<cluster_id_t>& CurrentAssignment = CurrentPartition.GetAssignmentVector();
    vector<cluster_id_t>& NextAssignment    = NextPartition.GetAssignmentVector();

    vector<CPUBurst*>& Bursts = BurstsPerStep[Level];

    for (size_t i = 0; i < Bursts.size(); i++)
    {
      

      if (CurrentAssignment[i] == Node->GetID())
      {
        size_t NextPosition = InstancesTracking[Bursts[i]->GetInstance()][Level+1];
        
        NextAssignment[NextPosition] = UNCLASSIFIED;
      }
    }
    
  }

  if (NumberOfDivisions == 0)
  {
    NumberOfDivisions++;
  }
  
  /* DEBUG */
  cout << "ID " << Node->GetID() << " on Level " << Level+1 << " ";
  cout << "has " << NumberOfDivisions << " divisions" << endl;
  
  return NumberOfDivisions;
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
bool ClusteringRefinement::IsIDInSet(cluster_id_t       ID,
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
 *
 * \return True ID is inside IDsSet, false otherwise
 *
 */ 
bool ClusteringRefinement::PrintPlots(vector<CPUBurst*>& Bursts,
                                      Partition&         CurrentPartition,
                                      size_t             Step)
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
    SetErrorMessage("number of points different of number of IDs");
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
  */
  
  PlotTitle << "REFINEMENT STEP " << Step+1 << " - ";
  PlotTitle << ClusteringCore->GetClusteringAlgorithmName();
  
  if (!Plots->PrintPlots(CurrentDataFileName.str(),
                         CurrentPlotFileNamePrefix.str(),
                         PlotTitle.str(),
                         CurrentPartition.NumberOfClusters(),
                         CurrentPartition.HasNoise()))
  {
    SetError(true);
    SetErrorMessage(Plots->GetLastError());
    return false;
  }
  
  return true;
}

/**
 * Generates a representation of the resulting partitioning tree to a file
 *
 * \return True if the tree was written correctly, false otherwise
 *
 */  
bool ClusteringRefinement::PrintTrees(void)
{
  ofstream* Output;
  set<ClusterInformation*>::iterator ClustersInfoIt;
  
  if (PrintStepsInformation)
  {
    ostringstream TreeFileName;
    TreeFileName << OutputFilePrefix << ".TREE";

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

  /* DEBUG 
  cout << "TOP LEVEL HAS " << ClustersHierarchy[0].size() << " NODES" << endl; */
  //for (size_t i = 0; i < ClustersHierarchy[0].size(); i++)

  for (size_t i = 0; i <= LastStep; i++)
  {
    (*Output) << setiosflags(ios::right);
    (*Output) << setfill(' ') << setw(26);
    (*Output) << i+1 << "|";
    (*Output) << "   ";
  }
  (*Output) << endl;
  
  for (ClustersInfoIt  = ClustersHierarchy[0].begin();
       ClustersInfoIt != ClustersHierarchy[0].end();
       ++ClustersInfoIt)
  {
    // if ((ClustersHierarchy[0][i]->GetID() != NOISE_CLUSTERID))
    if ((*ClustersInfoIt)->GetID() != NOISE_CLUSTERID)
    {
      // (*Output) << "***** Cluster " << ClustersHierarchy[0][i]->GetID() << " *****" << '\n';
      (*Output) << "***** Cluster " << (*ClustersInfoIt)->GetID() << " *****" << '\n';

      PrintNode((*Output), (*ClustersInfoIt), 0);
    }
  }

  Output->close();
  
  return true;
}

bool ClusteringRefinement::PrintNode(ofstream&           str,
                                     ClusterInformation* Node,
                                     size_t              Level)
{
  vector<ClusterInformation*>& Children = Node->GetChildren();

  if (Children.size() == 0)
  {
    if (Node->IsDiscarded())
    {
      str << " X ";
    }
    else
    {
      str << "   ";
    }
    
    str << setfill(' ') << setw(5);
    
    if (Node->GetID() == NOISE_CLUSTERID)
      str << "NOISE";
    else
      str << Node->GetID();
    
    str << " (";                                                        // 3
    str << setfill (' ') << setw(3) << Node->GetOccurrences();          // 3
    str << " : ";                                                       // 3
    str << setfill (' ') << setw(6) << setiosflags(ios::fixed);
    str << setprecision(2);
    str << (Node->GetScore()*100.0) << "%";                             // 6
    str << " )";                                                        // 2
    str << " |" << '\n';
  }
  else
  {
    for (size_t i = 0; i < Children.size(); i++)
    {
      if (i == 0)
      {
        if (Node->IsDiscarded())
        {
          str << " X ";
        }
        else
        {
          str << "   ";
        }
        
        str << setfill(' ') << setw(5);
        if (Node->GetID() == NOISE_CLUSTERID)
          str << "NOISE";
        else
          str << Node->GetID();

        str << " (";                                                        // 3
        str << setfill (' ') << setw(3) << Node->GetOccurrences();          // 3
        str << " : ";                                                       // 3
        str << setfill (' ') << setw(6) << setiosflags(ios::fixed);
        str << setprecision(2);
        str << (Node->GetScore()*100.0) << "%";                             // 6
        str << " )";                                                        // 2
        str << " --> ";
      }
      else
      {
        for (size_t j = 0; j < Level+1; j++)
        {
          // 30 chars in total
          str << "                         ";
          
          if (j < Level)
          {
            str << "     ";
          }
        }
        str << " |-> ";
      }

      PrintNode(str, Children[i], Level+1);
    }
  }
  
  return true;
}

