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


#ifndef _LIBTRACECLUSTERING_HPP_
#define _LIBTRACECLUSTERING_HPP_

#include "trace_clustering_types.h"

#include <string>
using std::string;

#include <map>
using std::map;

#include <set>
using std::set;

#define DO_NOTHING              0x00
#define CLUSTERING              0x01
#define PLOTS                   0x02
#define PARAMETER_APPROXIMATION 0x04
#define CLUSTERING_REFINEMENT   0x08
#define MPI                     0x10

#define USE_CLUSTERING(x)              (x & CLUSTERING)
#define USE_PLOTS(x)                   (x & PLOTS)
#define USE_PARAMETER_APPROXIMATION(x) (x & PARAMETER_APPROXIMATION)
#define USE_CLUSTERING_REFINEMENT(x)   (x & CLUSTERING_REFINEMENT)
#define USE_MPI(x)                     (x & MPI)


class libTraceClusteringImplementation;

class libTraceClustering
{
  private:
    libTraceClusteringImplementation* Implementation;

    bool   Error,        Warning;
    string ErrorMessage, WarningMessage;

  public:


    libTraceClustering(bool verbose);

    bool InitTraceClustering(string        ClusteringDefinitionXML,
                             unsigned char Flags);

    bool ExtractData(string            InputFileName,
                     bool              SampleData    = false,
                     unsigned int      MaxSamples    = 0,
                     set<unsigned int> EventsToParse = set<unsigned int> ());

    bool ExtractData(string            InputFileName,
                     string            OutputCSVFileName,
                     bool              SampleData    = false,
                     unsigned int      MaxSamples    = 0,
                     set<unsigned int> EventsToParse = set<unsigned int> ());

    bool ClusterAnalysis (void);

    bool ClusterRefinementAnalysis(bool   Divisive,
                                   bool   PrintStepsInformation,
                                   string OutputFileNamePrefix = "");

    bool ClusterRefinementAnalysis(bool   Divisive,
                                   int    MinPoints,
                                   double MaxEps,
                                   double MinEps,
                                   int    Steps,
                                   bool   PrintStepsInformation,
                                   string OutputFileNamePrefix = "");

    bool FlushClustersInformation(string OutputClustersInfoFileName);

    bool FlushData(string OutputCSVFileNamePrefix);

    bool ComputeSequenceScore(string OutputFilePrefix,
                              bool   FASTASequenceFile);

    bool ReconstructInputTrace(string OutputTraceName);

    bool PrintPlotScripts(string DataFileNamePrefix,
                          string ScriptsFileNamePrefix = "");

    bool ParametersApproximation(string              OutputFileNamePrefix,
                                 map<string, string> Parameters);

    bool   GetError(void) { return Error; };
    string GetErrorMessage(void);

    bool   GetWarning(void) { return Warning; };
    string GetWarningMessage(void);

  protected:

};

#endif // _LIBCLUSTERING_HPP_
