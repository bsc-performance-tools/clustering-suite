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

#include "SequenceScore.hpp"
#include "SequenceScoreMatrices.hpp"

#include <sstream>
using std::ostringstream;

#include <map>
using std::map;

#include <string>
using std::string;

#include <sstream>
using std::ostringstream;

#include "kalign2/kalign2.h"

/******************************************************************************
 * CLASS 'SequenceScoreValue'
 *****************************************************************************/
SequenceScoreValue::SequenceScoreValue(cluster_id_t ID, percentage_t Weight)
{
  this->ID     = ID;
  Occurrences  = 0;
  AlignmentSum = 0.0;
  this->Weight = Weight;
}

void SequenceScoreValue::NewOccurrence(percentage_t PositionAlignment)
{
  Occurrences++;
  AlignmentSum += PositionAlignment;
}

double SequenceScoreValue::GetClusterScore(void)
{
  if (Occurrences != 0)
  {
    return (AlignmentSum/Occurrences);
  }
  else
  {
    return 0.0;
  }
}

double SequenceScoreValue::GetWeightedScore(void)
{
  return GetClusterScore()*Weight;
}

/******************************************************************************
 * CLASS 'SequenceScore'
 *****************************************************************************/
bool                    SequenceScore::TablesLoaded = false;
int                     SequenceScore::LastAminoacidValue = 22;
map<cluster_id_t, char> SequenceScore::Clusters2Aminoacids;
map<char, cluster_id_t> SequenceScore::Aminoacids2Clusters;

void SequenceScore::LoadTranslationTables(void)
{
  if (!SequenceScore::TablesLoaded)
  {
    Clusters2Aminoacids.insert(make_pair(NOISE_CLUSTERID, 'Z'));
    Clusters2Aminoacids.insert(make_pair(1, 'A'));
    Clusters2Aminoacids.insert(make_pair(2, 'B'));
    Clusters2Aminoacids.insert(make_pair(3, 'C'));
    Clusters2Aminoacids.insert(make_pair(4, 'D'));
    Clusters2Aminoacids.insert(make_pair(5, 'E'));
    Clusters2Aminoacids.insert(make_pair(6, 'F'));
    Clusters2Aminoacids.insert(make_pair(7, 'G'));
    Clusters2Aminoacids.insert(make_pair(8, 'H'));
    Clusters2Aminoacids.insert(make_pair(9, 'I'));
    Clusters2Aminoacids.insert(make_pair(10, 'K'));
    Clusters2Aminoacids.insert(make_pair(11, 'L'));
    Clusters2Aminoacids.insert(make_pair(12, 'M'));
    Clusters2Aminoacids.insert(make_pair(13, 'N'));
    Clusters2Aminoacids.insert(make_pair(14, 'P'));
    Clusters2Aminoacids.insert(make_pair(15, 'Q'));
    Clusters2Aminoacids.insert(make_pair(16, 'R'));
    Clusters2Aminoacids.insert(make_pair(17, 'S'));
    Clusters2Aminoacids.insert(make_pair(18, 'T'));
    Clusters2Aminoacids.insert(make_pair(19, 'V'));
    Clusters2Aminoacids.insert(make_pair(20, 'W'));
    Clusters2Aminoacids.insert(make_pair(21, 'Y'));

    Aminoacids2Clusters.insert(make_pair('Z', NOISE_CLUSTERID));
    Aminoacids2Clusters.insert(make_pair('A', 1));
    Aminoacids2Clusters.insert(make_pair('B', 2));
    Aminoacids2Clusters.insert(make_pair('C', 3));
    Aminoacids2Clusters.insert(make_pair('D', 4));
    Aminoacids2Clusters.insert(make_pair('E', 5));
    Aminoacids2Clusters.insert(make_pair('F', 6));
    Aminoacids2Clusters.insert(make_pair('G', 7));
    Aminoacids2Clusters.insert(make_pair('H', 8));
    Aminoacids2Clusters.insert(make_pair('I', 9));
    Aminoacids2Clusters.insert(make_pair('K', 10));
    Aminoacids2Clusters.insert(make_pair('L', 11));
    Aminoacids2Clusters.insert(make_pair('M', 12));
    Aminoacids2Clusters.insert(make_pair('N', 13));
    Aminoacids2Clusters.insert(make_pair('P', 14));
    Aminoacids2Clusters.insert(make_pair('Q', 15));
    Aminoacids2Clusters.insert(make_pair('R', 16));
    Aminoacids2Clusters.insert(make_pair('S', 17));
    Aminoacids2Clusters.insert(make_pair('T', 18));
    Aminoacids2Clusters.insert(make_pair('V', 19));
    Aminoacids2Clusters.insert(make_pair('W', 20));
    Aminoacids2Clusters.insert(make_pair('Y', 21));
  }

  SequenceScore::TablesLoaded = true;
}



bool SequenceScore::ComputeScore(const vector<CPUBurst*>&         DataBursts,
                                 vector<cluster_id_t>&            AssignmentVector,
                                 map<cluster_id_t, percentage_t>& PercentageDurations,
                                 vector<SequenceScoreValue>&      ClusterScores,
                                 double&                          GlobalScore,
                                 string                           FileNamePrefix,
                                 bool                             FASTA)
{
  vector<pair<task_id_t, thread_id_t> >                             ObjectIDs;

  // map<pair<task_id_t, thread_id_t>, TSequence, TraceObjectsCompare> SequencesMap;
  // map<pair<task_id_t, thread_id_t>, TSequence >::iterator           SequencesIterator;
  // map<pair<task_id_t, thread_id_t>, vector<cluster_id_t>, TraceObjectsCompare> SequencesMap;
  // map<pair<task_id_t, thread_id_t>, vector<cluster_id_t> >::iterator           SequencesIterator;
  TSequenceMap         Sequences;
  TSequenceMapIterator SequencesIt;

  vector<vector<cluster_id_t> > SequencesMatrix;

  set<cluster_id_t>::iterator   DifferentIDsIterator, DifferentIDsIterator2;

  size_t NumberOfClusters = PercentageDurations.size();

  // TAlign Alignment;

  int const GapOpen   = -1;
  int const GapExtend = -2;

  // seqan::Score<int> TestScore(10, 5, -5, -10);
  //seqan::Score<int> TestScore(5, 0, -1, -2);
  // seqan::Score<int, seqan::ScoreMatrix<cluster_id_t> > TestScore (GapOpen, GapExtend);
  //seqan::Score<int, seqan::EditDistance> TestScore;

  /* Convert the Partition into a set of sequences */
  for (size_t i = 0; i < DataBursts.size(); i++)
  {
    pair<task_id_t, thread_id_t> CurrentKey;

    CurrentKey.first  = DataBursts[i]->GetTaskId();
    CurrentKey.second = DataBursts[i]->GetThreadId();

    // appendValue(SequencesMap[CurrentKey], (AssignmentVector[i]));
    Sequences[CurrentKey].push_back(AssignmentVector[i]);

    DifferentIDs.insert(AssignmentVector[i]);
  }

  if (DifferentIDs.size() != PercentageDurations.size())
  {
    ostringstream Message;

    Message << "Number of IDs used (" << DifferentIDs.size() << ") ";
    Message << "different from number of durations given (" << PercentageDurations.size() << ")";

    SetErrorMessage(Message.str());
    SetError(true);
  }

  Kalign2Score(Sequences, DifferentIDs, PercentageDurations);

  // resize(rows(Alignment), SequencesMap.size());

  size_t SeqPosition;
  for (SequencesIt  = Sequences.begin(), SeqPosition = 0;
       SequencesIt != Sequences.end();
       ++SequencesIt, ++SeqPosition)
  {
    ObjectIDs.push_back(SequencesIt->first);
    // assignSource(row(Alignment, SeqPosition), SequencesIterator->second);
  }

  /* Generate the dynamic score matrix
  for (DifferentIDsIterator  = DifferentIDs.begin();
       DifferentIDsIterator != DifferentIDs.end();
       ++DifferentIDsIterator)
  {
    for (DifferentIDsIterator2  = DifferentIDs.begin();
         DifferentIDsIterator2 != DifferentIDs.end();
         ++DifferentIDsIterator2)
    {
      cluster_id_t i, j;

      i = (*DifferentIDsIterator);
      j = (*DifferentIDsIterator2);

      cout << "[" << i << "|" << j << "] ";


      // if (i == 0 || j == 0)
      // {
        //seqan::setScore(TestScore, i, j, 1);
        //seqan::setScore(TestScore, i, j, 0);
      // }
      // else
      if (i == j)
      {
        //seqan::setScore(TestScore, i, j, static_cast<int>(ceil(PercentageDurations[i]*100)));
        // seqan::setScore(TestScore, i, j, 5);
      }
      else
      {
        //seqan::setScore(TestScore, i, j, 1);
        // seqan::setScore(TestScore, i, j, 0);
      }

      // cout << seqan::score(TestScore, i, j) << "\t";
    }
    cout << endl;
  }

  /* Analyze the sequences using SeqAn */
  // globalMsaAlignment(Alignment, TestScore);

  /* Generate the output vector
  AlignmentToMatrix (Alignment);

  /* Compute the actual score per cluster */
  EffectiveScoreComputation(PercentageDurations, ClusterScores, GlobalScore);

  /* Compute the global score */
  GlobalScore = 0.0;
  for (size_t i = 0; i < ClusterScores.size(); i++)
  {
    GlobalScore += ClusterScores[i].GetWeightedScore();
  }

  /* If needed, generate the sequences file */
  if (FileNamePrefix.compare("") != 0)
  {
    ostringstream SequencesFileName;
    ostringstream ScoresFileName;

    SequencesFileName << FileNamePrefix << ".seq";
    ScoresFileName    << FileNamePrefix << ".SCORES.csv";

    ofstream SequencesStream(SequencesFileName.str().c_str(), ios_base::trunc);
    FlushSequences (ObjectIDs, SequencesStream, FASTA);
    SequencesStream.close();

    ofstream ScoresStream(ScoresFileName.str().c_str(), ios_base::trunc);
    FlushScores(ClusterScores, GlobalScore, ScoresStream);
    ScoresStream.close();
  }

  return true;
}

extern "C"
{
  unsigned int numseq      = 0;
  unsigned int numprofiles = 0;
  float        gpo         = -5;
  float        gpe         = -2;
  float        tgpe        = -1;
}

void SequenceScore::Kalign2Score(TSequenceMap&                    Sequences,
                                 set<cluster_id_t>                DifferentIDs,
                                 map<cluster_id_t, percentage_t>& PercentageDurations)
{
  TSequenceMapIterator SequencesIt;

  int* tree = NULL;

  int a, b, c, i, j, f, tmp;

  struct parameters* param    = NULL;
  struct alignment* aln       = NULL;
  struct aln_tree_node* tree2 = NULL;

  float **submatrix = NULL;

  param = (struct parameters*) malloc (sizeof (struct parameters) );

  numseq      = Sequences.size();
  numprofiles = (numseq << 1) - 1;

  aln    = aln_alloc (aln);

  cluster_id_t MaxClusterId = 0;

  /* Check maximum cluster id */
  set<cluster_id_t>::iterator DiffIDsIt;

  for (DiffIDsIt  = DifferentIDs.begin();
       DiffIDsIt != DifferentIDs.end();
       ++DiffIDsIt)
  {
    if ((*DiffIDsIt) > MaxClusterId)
    {
      MaxClusterId = (*DiffIDsIt);
    }
  }

  MaxClusterId += 1;

  /* Fill matrix */
  if ((submatrix = (float**) malloc (sizeof(float*) * (MaxClusterId+1))) == NULL)
  {
    std::cerr << "Error Allocating Submatrix" << std::endl;
    exit(EXIT_FAILURE);
  }

  submatrix[0] = (float*) malloc (sizeof(float) * (MaxClusterId+1));
  for (j = 0; j <= MaxClusterId; j++)
  {
    submatrix[0][j] = 0;
  }

  if (MaxClusterId >= 1)
  {
    gpo  = 10;
    gpe  = 5;
    tgpe = 5;
  }

  for (i = MaxClusterId; i--;)
  {
    submatrix[i+1]    = (float*) malloc (sizeof (float) * (MaxClusterId+1));
    // submatrix[i+1][0] = 0;

    if (DifferentIDs.count(i) != 0)
    {
      for (j = MaxClusterId; j--;)
      {
        if (DifferentIDs.count(j) != 0)
        {
          if ( i == NOISE_CLUSTERID || j == NOISE_CLUSTERID)
          {
            // submatrix[i+1][j+1] = PercentageDurations[NOISE_CLUSTERID]*100;
            submatrix[i+1][j+1] = 5;
          }
          else if ( i == j )
          {
            // submatrix[i+1][j+1] = PercentageDurations[i]*100;
             submatrix[i+1][j+1] = (MaxClusterId-i)+5;
          }
          else
          {
            // submatrix[i+1][j+1] = PercentageDurations[NOISE_CLUSTERID]*100;
            submatrix[i+1][j+1] = 3;
          }
        }
      }
    }
    else
    {
      for ( j = MaxClusterId; j--;)
      {
        submatrix[i+1][j+1] = 0;
      }
    }
  }

  /* Prepare sequences */
  for (SequencesIt  = Sequences.begin(), i = 0;
       SequencesIt != Sequences.end();
       ++SequencesIt, ++i)
  {
    vector<cluster_id_t>& SequenceVector = SequencesIt->second;

    aln->sn[i]  = NULL;
    aln->sl[i]  = SequenceVector.size();
    aln->s[i]   = (int*) malloc (sizeof (int) * (SequenceVector.size()+1));
    aln->seq[i] = (int*) malloc (sizeof (int) * (SequenceVector.size()+1));

    for (size_t j = 0; j < SequenceVector.size(); j++)
    {
      aln->s[i][j]   = SequenceVector[j]+1;
      aln->seq[i][j] = SequenceVector[j]+1;
    }

    aln->s[i][aln->sl[i]]   = 0;
    aln->seq[i][aln->sl[i]] = 0;
  }

  /* Perform the alignment */

  /* First step: pairwise alignment */
  float** dm  = NULL;
  int**   map = NULL;

  //dm    = protein_wu_distance (aln, dm, param, 0);
  dm    = protein_pairwise_alignment_distance(aln, dm, param, submatrix, 0);
  tree2 = real_upgma (dm, 2); // 2 trees

  tree = (int*) malloc (sizeof (int) * (numseq * 3 + 1) );
  for ( i = 1; i < (numseq * 3) + 1; i++)
  {
    tree[i] = 0;
  }
  tree[0] = 1;

  tree = readtree (tree2, tree);

  for (i = 0; i < (numseq * 3); i++)
  {
    tree[i] = tree[i+1];
  }

  free (tree2->links);
  free (tree2->internal_lables);
  free (tree2);

  /* Second step: global alignment */
  map =  default_alignment (aln, tree, submatrix, map);
  /* map   =  hirschberg_alignment (aln,
                                 tree,
                                 submatrix,
                                 map,
                                 1, // Use smooth window
                                 -2); */

//clear up sequence array to be reused as gap array....
  int *p = 0;

  for (i = 0; i < numseq; i++)
  {
    p = aln->s[i];

    for (a = 0; a < aln->sl[i]; a++)
    {
      p[a] = 0;
    }
  }

  //clear up

  for (i = 0; i < (numseq - 1) * 3; i += 3)
  {
    a   = tree[i];
    b   = tree[i+1];
    aln = make_seq (aln, a, b, map[tree[i+2]]);
  }

  //for (i = 0; i < numseq;i++){
  //  fprintf(stderr,"%s  %d\n",aln->sn[i],aln->nsip[i]);
  //}

  for (i = 0; i < numseq; i++)
  {
    aln->nsip[i] = 0;
  }

  aln =  sort_sequences (aln, tree, "input");

  /* Fill the 'SequencesMatrix' */
  SequencesMatrix.clear();
  SequencesMatrix = vector<vector<cluster_id_t> > (numseq);

  for (i = 0; i < numseq; i++)
  {
    f = aln->nsip[i];

    for (j = 0; j < aln->sl[f]; j++)
    {
      tmp = aln->s[f][j];
      while (tmp)
      {
        SequencesMatrix[i].push_back(SEQUENCE_GAP);
        tmp--;
      }

      SequencesMatrix[i].push_back(aln->seq[f][j]-1);
    }

    tmp = aln->s[f][aln->sl[f]];

    while (tmp)
    {
      SequencesMatrix[i].push_back(SEQUENCE_GAP);
      tmp--;
    }
  }

  for (i = MaxClusterId+1; i--;)
  {
    free (submatrix[i]);
  }

  free (submatrix);
  free_aln(aln);
  free(map);
  free(tree);

  // exit(EXIT_SUCCESS);

  return;


}

/*
void SequenceScore::AlignmentToMatrix(TAlign& Alignment)
{
  TPosition begin = beginPosition(cols(Alignment));
  TPosition end   = endPosition(cols(Alignment));

  SequencesMatrix.clear();
  SequencesMatrix = vector<vector<cluster_id_t> > (length(rows(Alignment)));

  for(size_t i = 0; i < length(rows(Alignment)); ++i)
  {
    TRow& CurrentRow = row(Alignment, i);

    TIterator SequencePosition = iter(CurrentRow, begin);
    TIterator SequenceEnd      = iter(CurrentRow, end);

    while (SequencePosition != SequenceEnd)
    {
      if (isGap(SequencePosition))
      {
        SequencesMatrix[i].push_back(SEQUENCE_GAP);
      }
      else
      {
        SequencesMatrix[i].push_back((cluster_id_t) value(SequencePosition));
      }

      SequencePosition++;
    }
  }
}
*/


bool SequenceScore::EffectiveScoreComputation(map<cluster_id_t, percentage_t>& PercentageDurations,
                                              vector<SequenceScoreValue>&      ClusterScores,
                                              double&                          GlobalScore)
{
  size_t SequencesLength;
  size_t NumberOfSequences;
  size_t NumberOfClusters = PercentageDurations.size();

  map<cluster_id_t, bool>     ClusterAppears;
  map<cluster_id_t, size_t>   NumberOfOccurrences;
  set<cluster_id_t>::iterator DifferentIDsIterator;
  map<cluster_id_t, size_t>   ScorePosition;
  size_t                      Position;

  ClusterScores.clear();

  for (DifferentIDsIterator  = DifferentIDs.begin(), Position = 0;
       DifferentIDsIterator != DifferentIDs.end();
       ++DifferentIDsIterator, Position++)
  // for (size_t Cluster = 0; Cluster < DifferentIDs.size(); Cluster++)
  {
    cluster_id_t CurrentID = (*DifferentIDsIterator);
    ClusterScores.push_back(SequenceScoreValue(CurrentID,
                                               PercentageDurations[CurrentID]));


    ScorePosition[CurrentID]       = Position;
  }

  NumberOfSequences = SequencesMatrix.size();
  if (NumberOfSequences == 0)
  {
    return false;
  }
  SequencesLength = SequencesMatrix[0].size();

  /* DEBUG
  cout << "Number of Sequences = " << NumberOfSequences << " ";
  cout << "Sequences Length = " << SequencesLength << endl; */

  /* i -> iterator over sequences length */
  for (size_t i = 0; i < SequencesLength; i++)
  {
    for (DifferentIDsIterator  = DifferentIDs.begin();
         DifferentIDsIterator != DifferentIDs.end();
         ++DifferentIDsIterator)
    {
      cluster_id_t CurrentID = (*DifferentIDsIterator);
      ClusterAppears[CurrentID]      = false;
      NumberOfOccurrences[CurrentID] = 0;
    }

    /* j -> iterator over sequence positions (columns) */
    for (size_t j = 0; j < NumberOfSequences; j++)
    {
      if (SequencesMatrix[j][i] != SEQUENCE_GAP)
      {
        cluster_id_t CurrentCluster    = SequencesMatrix[j][i];
        ClusterAppears[CurrentCluster] = true;
        NumberOfOccurrences[CurrentCluster]++;
      }
    }

    for (DifferentIDsIterator  = DifferentIDs.begin();
         DifferentIDsIterator != DifferentIDs.end();
         ++DifferentIDsIterator)
    {
      cluster_id_t CurrentID = (*DifferentIDsIterator);

      if (ClusterAppears[CurrentID])
      {
        double ClusterAlignment =
          (1.0*NumberOfOccurrences[CurrentID])/NumberOfSequences;

        ClusterScores[ScorePosition[CurrentID]].NewOccurrence(ClusterAlignment);
      }
    }
  }

  return true;
}

bool SequenceScore::FlushSequences(vector<pair<task_id_t, thread_id_t> >& ObjectIDs,
                                   ofstream&                              str,
                                   bool                                   FASTA)
{

  size_t NumSequences, SequencesLength;

  if (FASTA)
  {
    LoadTranslationTables();
  }

  NumSequences    = ObjectIDs.size();

  if (NumSequences == 0)
  {
    // SetError(true);
    // SetErrorMessage("no sequences available to flush");
    return false;
  }

  SequencesLength = SequencesMatrix[0].size();

  if (SequencesLength == 0)
  {
    // SetError(true);
    // SetErrorMessage("trying to flush a set of empty sequence");
    return true;
  }


  for (size_t i = 0; i < NumSequences; i++)
  {

    str << ">Thread_" << (ObjectIDs[i].first+1) << "." << (ObjectIDs[i].second+1) << '\n';

    for (size_t j = 0; j < SequencesLength; j++)
    {

      if (SequencesMatrix[i][j] == SEQUENCE_GAP)
      {
        str << '-';
      }
      else
      {
        if (FASTA)
        {
          if (SequencesMatrix[i][j] < SequenceScore::LastAminoacidValue)
          {
            str << SequenceScore::Clusters2Aminoacids[SequencesMatrix[i][j]];
          }
          else
          {
            str << 'X';
          }
        }
        else
        {
          str << SequencesMatrix[i][j];
        }
      }

      if (j < SequencesLength-1)
      {
        if (!FASTA)
        {
          str << ",";
        }
      }
    }
    str << '\n';
  }
  return true;
}

bool SequenceScore::FlushScores(vector<SequenceScoreValue>& ClusterScores,
                                double&                     GlobalScore,
                                ofstream&                   str)
{
  str << "Cluster,Cluster Score,Occurrences,WeightedScore" << '\n';
  str << fixed;

  for (size_t i = 0; i < ClusterScores.size(); i++)
  {
    if (i == NOISE_CLUSTERID)
    {
      str << "NOISE";

      str.precision(5);
      str << "," << ClusterScores[i].GetClusterScore();
      str.precision(0);
      str << "," << ClusterScores[i].GetOccurrences();
      str.precision(5);
      str << "," << ClusterScores[i].GetWeightedScore();
      str << '\n';
    }
    else
    {
      str << "Cluster " << ClusterScores[i].GetID();
      str.precision(5);
      str << "," << ClusterScores[i].GetClusterScore();
      str.precision(0);
      str << "," << ClusterScores[i].GetOccurrences();
      str.precision(5);
      str << "," << ClusterScores[i].GetWeightedScore();
      str << '\n';

    }
  }

  str << "GLOBAL," << GlobalScore << '\n';

  return true;
}

