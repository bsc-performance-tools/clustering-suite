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

#include "SequenceScore.hpp"

#include <sstream>
using std::ostringstream;

#include <map>
using std::map;

#include <string>
using std::string;

#include <sstream>
using std::ostringstream;

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
int                     SequenceScore::LastAminoacidValue = 23;
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
    Clusters2Aminoacids.insert(make_pair(21, 'X'));
    Clusters2Aminoacids.insert(make_pair(22, 'Y'));

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
    Aminoacids2Clusters.insert(make_pair('X', 21));
    Aminoacids2Clusters.insert(make_pair('Y', 22));
  }

  SequenceScore::TablesLoaded = true;
}


/* Functor to compare the objects */
struct TraceObjectsCompare
{
  bool operator()(const pair<task_id_t, thread_id_t>& obj1,
                  const pair<task_id_t, thread_id_t>& obj2)
  {
    if (obj1.first < obj2.first)
    {
      return true;
    }
    else if (obj1.first > obj2.first)
    {
      return false;
    }
    else  // same task_id, compare thread_id
    {
      if (obj1.second < obj2.second)
      {
        return true;
      }
      else
      {
        return false;
      }
    }
  }
};

bool SequenceScore::ComputeScore(const vector<CPUBurst*>&    DataBursts,
                                 vector<cluster_id_t>&       ClusterIDs,
                                 vector<percentage_t>&       PercentageDurations,
                                 bool                        HasNoise,
                                 vector<SequenceScoreValue>& ClusterScores,
                                 double&                     GlobalScore,
                                 string                      FileNamePrefix,
                                 bool                        FASTA)
{
  vector<pair<task_id_t, thread_id_t> >                             ObjectIDs;
  
  map<pair<task_id_t, thread_id_t>, TSequence, TraceObjectsCompare> SequencesMap;
  map<pair<task_id_t, thread_id_t>, TSequence >::iterator           SequencesIterator;

  vector<vector<cluster_id_t> > SequencesMatrix;

  size_t NumberOfClusters = PercentageDurations.size();
  
  TAlign Alignment;

  // seqan::Score<int> TestScore(10, 5, -5, -10);
  seqan::Score<int> TestScore(5, 0, -1, -2);
  
  /* Convert the Partition into a set of sequences */
  for (size_t i = 0; i < DataBursts.size(); i++)
  {
    pair<task_id_t, thread_id_t> CurrentKey;

    CurrentKey.first  = DataBursts[i]->GetTaskId();
    CurrentKey.second = DataBursts[i]->GetThreadId();

    appendValue(SequencesMap[CurrentKey], (ClusterIDs[i]));
  }

  resize(rows(Alignment), SequencesMap.size());

  size_t SeqPosition;
  for (SequencesIterator  = SequencesMap.begin(), SeqPosition = 0;
       SequencesIterator != SequencesMap.end();
       ++SequencesIterator, ++SeqPosition)
  {
    ObjectIDs.push_back(SequencesIterator->first);
    assignSource(row(Alignment, SeqPosition), SequencesIterator->second);
  }

  /* Analyze the sequences using SeqAn */
  globalMsaAlignment(Alignment, TestScore);

  /* Generate the output vector */
  AlignmentToMatrix (Alignment, SequencesMatrix);

  /* Compute the actual score per cluster */
  EffectiveScoreComputation(SequencesMatrix, PercentageDurations, ClusterScores, GlobalScore);

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
    FlushSequences (ObjectIDs, SequencesMatrix, SequencesStream, FASTA);
    SequencesStream.close();

    ofstream ScoresStream(ScoresFileName.str().c_str(), ios_base::trunc);
    FlushScores(ClusterScores, GlobalScore, HasNoise, ScoresStream);
    ScoresStream.close();
  }
  
  return true;
}

void SequenceScore::AlignmentToMatrix(TAlign&                        Alignment,
                                      vector<vector<cluster_id_t> >& SequencesMatrix)
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

bool SequenceScore::EffectiveScoreComputation(vector<vector<cluster_id_t> >& SequencesMatrix,
                                              vector<percentage_t>&          PercentageDurations,
                                              vector<SequenceScoreValue>&    ClusterScores,
                                              double&                        GlobalScore)
{
  size_t SequencesLength;
  size_t NumberOfSequences;
  size_t NumberOfClusters = PercentageDurations.size();

  ClusterScores.clear();
  
  for (size_t Cluster = 0; Cluster < NumberOfClusters; Cluster++)
  {
    ClusterScores.push_back(SequenceScoreValue(Cluster,
                                               PercentageDurations[Cluster]));
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
    vector<bool>   ClusterAppears      = vector<bool> (NumberOfClusters, false);
    vector<size_t> NumberOfOccurrences = vector<size_t> (NumberOfClusters, 0);
    
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

    for (size_t Cluster = 0; Cluster < NumberOfClusters; Cluster++)
    {
      if (ClusterAppears[Cluster])
      {
        double ClusterAlignment = 
          (1.0*NumberOfOccurrences[Cluster])/NumberOfSequences;
        
        ClusterScores[Cluster].NewOccurrence(ClusterAlignment);
      }
    }
  }
  
  return true;
}

bool SequenceScore::FlushSequences(vector<pair<task_id_t, thread_id_t> >& ObjectIDs,
                                   vector<vector<cluster_id_t> >&         SequencesMatrix,
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
  
    str << ">Thread_" << ObjectIDs[i].first << "." << ObjectIDs[i].second << '\n';

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
          if (SequencesMatrix[i][j] <= SequenceScore::LastAminoacidValue)
          {
            str << SequenceScore::Clusters2Aminoacids[SequencesMatrix[i][j]];
          }
          else
          {
            str << '*';
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
                                bool                        HasNoise,
                                ofstream&                   str)
{
  str << "Cluster,Cluster Score,Occurrences,WeightedScore" << '\n';
  str << fixed;
  
  for (size_t i = 0; i < ClusterScores.size(); i++)
  {
    if (i == NOISE_CLUSTERID)
    {
      if (HasNoise)
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
    }
    else
    {
      str << "Cluster " << i;
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

