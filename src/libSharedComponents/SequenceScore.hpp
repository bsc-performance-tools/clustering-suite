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

#ifndef _SEQUENCESCORE_HPP_
#define _SEQUENCESCORE_HPP_

#include <Error.hpp>
using cepba_tools::Error;

#include <CPUBurst.hpp>

#include <trace_clustering_types.h>

#include <vector>
using std::vector;

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

// using namespace seqan;
typedef seqan::String<cluster_id_t>                      TSequence;
typedef seqan::Align<TSequence, seqan::ArrayGaps>        TAlign;
typedef seqan::Row<TAlign>::Type                         TRow;
typedef seqan::Iterator<TRow, seqan::Rooted>::Type       TIterator;
typedef seqan::Position<seqan::Rows<TAlign>::Type>::Type TRowsPosition;
typedef seqan::Position<TAlign>::Type                    TPosition;

static const cluster_id_t SEQUENCE_GAP = NOISE_CLUSTERID-1;

class SequenceScoreValue
{
  private:
    cluster_id_t ID;
    size_t       Occurrences;
    double       AlignmentSum;
    percentage_t Weight;

  public:
    SequenceScoreValue(cluster_id_t ID, percentage_t Weight);

    void NewOccurrence(percentage_t PositionAlignment);

    size_t GetOccurrences(void) { return Occurrences; };

    double GetClusterScore(void);

    double GetWeightedScore(void);
};

class SequenceScore: public Error 
{
  public:

    static bool                    TablesLoaded;
    static int                     LastAminoacidValue;
    static map<cluster_id_t, char> Clusters2Aminoacids;
    static map<char, cluster_id_t> Aminoacids2Clusters;

    static void LoadTranslationTables(void);
    
    bool ComputeScore(const vector<CPUBurst*>&   DataBursts,
                     vector<cluster_id_t>&       ClusterIDs,
                     vector<percentage_t>&       PercentageDurations,
                     bool                        HasNoise,
                     vector<SequenceScoreValue>& ClusterScores,
                     double&                     GlobalScore,
                     string                      FileNamePrefix = "",
                     bool                        FASTA = false);

  private:
    void AlignmentToMatrix(TAlign&                        Alignment,
                           vector<vector<cluster_id_t> >& SequencesMatrix);

    bool EffectiveScoreComputation(vector<vector<cluster_id_t> >& SequencesMatrix,
                                   vector<percentage_t>&          PercentageDurations,
                                   vector<SequenceScoreValue>&    ClusterScores,
                                   double&                        GlobalScore);
    
    bool FlushSequences(vector<pair<task_id_t, thread_id_t> >& ObjectIDs,
                        vector<vector<cluster_id_t> >&         SequencesMatrix,
                        ofstream&                              str,
                        bool                                   FASTA);

    bool FlushScores(vector<SequenceScoreValue>& ClusterScores,
                     double&                     GlobalScore,
                     bool                        HasNoise,
                     ofstream&                   str);
                               
  protected:
};

#endif // _SEQUENCESCORE_HPP_
