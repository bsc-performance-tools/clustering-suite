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

#ifndef CSVDATAEXTRACTOR_H
#define CSVDATAEXTRACTOR_H

#include <trace_clustering_types.h>

#include "DataExtractor.hpp"

#include <Partition.hpp>

#include <cmath>
#include <string>

#include <fstream>
using std::ifstream;

class CSVDataExtractor: public DataExtractor
{
  private:
    string              CSVFileName;
    ifstream            CSVFile;

    ifstream::pos_type FirstPos;
    ifstream::pos_type EndPos;

    set<cluster_id_t>    IDs;
    vector<cluster_id_t> AssignmentVector;

    vector<string>       ClusteringParameters;
    vector<string>       NormalizedParameters;
    vector<string>       ExtrapolationParameters;

    vector<string>::size_type RecordSize;
    UINT32                    CurrentLine;

  public:

    CSVDataExtractor(string CSVFileName);
    ~CSVDataExtractor();

    bool SetEventsToDealWith(set<event_type_t>& EventsToDealWith);

    bool ExtractData(TraceData* TraceDataSet);

    bool GetPartition(Partition& ReadPartition);

    input_file_t GetFileType(void) { return ClusteringCSV; };

  private:

    void PopulateRecord(vector<string> &Record,
                        const string   &Line,
                        char            Delimiter);

    bool ParseHeader(vector<string> &Record);

    bool ParseRecord(vector<string> &Record, TraceData* TraceDataSet);

};

#endif /* CSVDATAEXTRACTOR_H */
