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

#ifndef _LIBTRACECLUSTERINGIMPLEMENTATION_HPP_
#define _LIBTRACECLUSTERINGIMPLEMENTATION_HPP_

#include <string>
using std::string;

#include <Error.hpp>
using cepba_tools::Error;

#include <libClustering.hpp>
#include <TraceData.hpp>
#include <ClusteringStatistics.hpp>

#include "trace_clustering_types.h"

class libTraceClusteringImplementation: public Error
{

    static const string  DataFilePostFix;
    static const string  SampledDataFilePostFix;

  private:

    TraceData           *Data;
    libClustering       *ClusteringCore;
    Partition            LastPartition, ClassificationPartition;
    ClusteringStatistics Statistics;

    string               InputFileName;
    input_file_t         InputFileType;

    bool                 SampleData;

    bool                 ClusteringExecuted;
    bool                 ClusteringRefinementExecution;

    bool                 PRVEventsParsing;
    bool                 ConsecutiveEvts;
    set<event_type_t>    EventsToDealWith;

    unsigned char  UseFlags;

  public:
    libTraceClusteringImplementation(bool verbose);

    bool InitTraceClustering(string        ClusteringDefinitionXML,
                             string        PCFFileName,
                             unsigned char UseFlags);

    bool ExtractData(string            InputFileName,
                     bool              SampleData = false,
                     unsigned int      MaxSamples = 0,
                     set<event_type_t> EventsToDealWith = set<event_type_t> (),
                     bool              ConsecutiveEvts  = false);

    bool FlushData(string OutputCSVFileNamePrefix);

    bool ClusterAnalysis(void);

    bool ClusterRefinementAnalysis(bool   Divisive,
                                   bool   PrintStepsInformation,
                                   string OutputFileNamePrefix);

    bool ClusterRefinementAnalysis(bool   Divisive,
                                   int    MinPoints,
                                   double MaxEps,
                                   double MinEps,
                                   int    Steps,
                                   bool   PrintStepsInformation,
                                   string OutputFileNamePrefix);

    bool FlushClustersInformation(string OutputClustersInfoFileName);

    bool ComputeSequenceScore(string OutputFilePrefix,
                              bool   FASTASequenceFile);

    bool ReconstructInputTrace(string OutputTraceName);

    bool PrintPlotScripts(string DataFileNamePrefix,
                          string ScriptsFileNamePrefix);

    bool ParametersApproximation(string              OutputFileNamePrefix,
                                 map<string, string> Parameters);

  private:
    bool GenericRefinement(bool           Divisive,
                           int            MinPoints,
                           vector<double> EpsilonPerLevel,
                           bool           PrintStepsInformation,
                           string         OutputFileNamePrefix);

    void GetTaskSet(size_t TotalTasksInTrace);

    bool GatherMPIPartition(void);

    bool GatherMaster(void);

    bool GatherSlave(void);

    bool ReconstructMasterPartition(vector<vector<vector<long> > >& GlobalLinesPerCluster);

private:

};

#endif // _LIBCLUSTERING_HPP_
