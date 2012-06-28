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

#ifndef _SEQUENCESCORE_HPP_
#define _SEQUENCESCORE_HPP_

#include <Error.hpp>
using cepba_tools::Error;

#include <CPUBurst.hpp>

#include <trace_clustering_types.h>

#include <vector>
using std::vector;

#include <map>
using std::map;

#include <set>
using std::set;

#include <utility>
using std::pair;
using std::make_pair;

#include <fstream>
using std::ofstream;
using std::ios_base;

#include <seqan/align.h>
#include <seqan/graph_align.h>
#include <seqan/graph_msa.h>
#include <seqan/score.h>
#include <seqan/basic.h>

static const cluster_id_t SEQUENCE_GAP = NOISE_CLUSTERID-1;

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

class SequenceScoreValue
{
  private:
    cluster_id_t ID;
    size_t       Occurrences;
    double       AlignmentSum;
    percentage_t Weight;

  public:
    SequenceScoreValue(void) {};

    SequenceScoreValue(cluster_id_t ID, percentage_t Weight);

    cluster_id_t GetID(void) { return ID; };

    void NewOccurrence(percentage_t PositionAlignment);

    size_t GetOccurrences(void) { return Occurrences; };

    double GetClusterScore(void);

    double GetWeightedScore(void);
};

class SequenceScore: public Error
{
  // using namespace seqan;
  typedef seqan::String<cluster_id_t>                      TSequence;
  typedef seqan::Align<TSequence>                          TAlign;
  // typedef seqan::Align<TSequence, seqan::ArrayGaps>     TAlign;
  typedef seqan::Row<TAlign>::Type                         TRow;
  typedef seqan::Iterator<TRow, seqan::Rooted>::Type       TIterator;
  typedef seqan::Position<seqan::Rows<TAlign>::Type>::Type TRowsPosition;
  typedef seqan::Position<TAlign>::Type                    TPosition;

  typedef map<pair<task_id_t, thread_id_t>, vector<cluster_id_t>, TraceObjectsCompare> TSequenceMap;
  typedef TSequenceMap::iterator TSequenceMapIterator;


  private:

    static bool                    TablesLoaded;
    static int                     LastAminoacidValue;
    static map<cluster_id_t, char> Clusters2Aminoacids;
    static map<char, cluster_id_t> Aminoacids2Clusters;

    vector<vector<cluster_id_t> > SequencesMatrix;
    set<cluster_id_t>             DifferentIDs;

    static void LoadTranslationTables(void);

  public:
    bool ComputeScore(const vector<CPUBurst*>&         DataBursts,
                      vector<cluster_id_t>&            AssignmentVector,
                      map<cluster_id_t, percentage_t>& PercentageDurations,
                      vector<SequenceScoreValue>&      ClusterScores,
                      double&                          GlobalScore,
                      string                           FileNamePrefix = "",
                      bool                             FASTA = false);

    vector<vector<cluster_id_t> >& GetSequencesMatrix(void) { return SequencesMatrix; };

  private:
    void Kalign2Score(TSequenceMap&                    Sequences,
                      set<cluster_id_t>                DifferentIDs,
                      map<cluster_id_t, percentage_t>& PercentageDurations);

    void AlignmentToMatrix(TAlign& Alignment);

    bool EffectiveScoreComputation(map<cluster_id_t, percentage_t>& PercentageDurations,
                                   vector<SequenceScoreValue>&      ClusterScores,
                                   double&                          GlobalScore);

    bool FlushSequences(vector<pair<task_id_t, thread_id_t> >& ObjectIDs,
                        ofstream&                              str,
                        bool                                   FASTA);

    bool FlushScores(vector<SequenceScoreValue>& ClusterScores,
                     double&                     GlobalScore,
                     ofstream&                   str);

  protected:
};

#endif // _SEQUENCESCORE_HPP_
