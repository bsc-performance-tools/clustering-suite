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

#include <trace_clustering_types.h>

#include <Utilities.hpp>

#include <SystemMessages.hpp>
using cepba_tools::system_messages;
#include <FileNameManipulator.hpp>
using cepba_tools::FileNameManipulator;
#include <Timer.hpp>
using cepba_tools::Timer;

/* That should be done using the 'libTraceClustering' library, but the
   interface has to be modified :( */
#include <CSVDataExtractor.hpp>
#include <Partition.hpp>
#include <ClusteringStatistics.hpp>
#include <SequenceScore.hpp>

#include <cstring>
#include <cstdlib>
#include <cstdio>
#include <cerrno>

#include <unistd.h>
#include <getopt.h>

#include <iostream>
#include <iomanip>
using std::cout;
using std::cerr;
using std::endl;
using std::fixed;
using std::setprecision;
using std::setw;
using std::setfill;

#include <string>
#include <sstream>
using std::stringstream;

#include <vector>
using std::vector;

#include <set>
using std::set;



struct globalArgs_t {
  bool   verbosity;              /* -v option */
  string ClustersFile;
  string ClustersFilePrefix;
  bool   GenerateOutputs;
} globalArgs;

#define ABOUT \
"ClustersSequenceScore version 1.0\n"\
"(c) CEPBA-Tools - Barcelona Supercomputing Center\n"\
"Cluster Sequence Score a cluster analysis results\n"

#define HELP \
"Usage: ClustersSequenceScore [options] <clusters_data_csv>\n"\
"  -f  : Generates a FASTA file with the MSA results and a CSV file with\n"\
"        the detailed score results (using the input file a file name prefix\n"\
"  -v  : Verbose mode\n"\
"  -h  : Print this help\n"

void PrintUsage(void)
{
  cout << ABOUT;
  cout << HELP;
}

void CheckInput(void)
{
  string InputFileExtension =
    FileNameManipulator::GetExtension(globalArgs.ClustersFile);

  if (InputFileExtension.compare("csv") == 0 ||
      InputFileExtension.compare("CSV") == 0)
  {
    if (globalArgs.GenerateOutputs)
    {
      FileNameManipulator NameManipulator(globalArgs.ClustersFile,
                                          InputFileExtension);

      globalArgs.ClustersFilePrefix = NameManipulator.GetChoppedFileName();
    }
    else
    {
      globalArgs.ClustersFilePrefix = "";
    }
  }
  else
  {
    cerr << "Input file does not have 'csv' extension, please rename it" << endl;
    exit (EXIT_FAILURE);
  }

  return;
}


void ReadArgs(int argc, char *argv[])
{
  int opt = 0;
  int longIndex;

  if (argc == 1)
  {
    PrintUsage();
    exit(EXIT_SUCCESS);
  }

  /* Initialize globalArgs before we get to work. */
  globalArgs.verbosity       = false;
  globalArgs.ClustersFile    = "";
  globalArgs.GenerateOutputs = false;


  while( (opt = getopt( argc, argv, "fvh")) != -1 )
  {
    switch( opt )
    {
      case 'f':
        globalArgs.GenerateOutputs = true;
        break;

      case 'v':
        globalArgs.verbosity = true;
        break;

      case 'h':   /* fall-through is intentional */
      case '?':
        PrintUsage();
        exit (EXIT_SUCCESS);

      default:
        cerr << "Wrong parameters!" << endl << endl;
        PrintUsage ();
        exit(EXIT_FAILURE);
        break;
    }
  }

  if ( (argc - optind) != 1)
  {
    cerr << "Wrong parameters!" << endl << endl;
    PrintUsage ();
    exit(EXIT_FAILURE);
  }

  if (globalArgs.verbosity)
  {
    system_messages::verbose = true;
  }

  /* Las parameter is the clusters csv file name */
  globalArgs.ClustersFile = argv[optind];
  CheckInput();

  return;
}

int main(int argc, char *argv[])
{
  Timer      T;
  TraceData *DataSet = TraceData::GetInstance();

  map<cluster_id_t, percentage_t> PercentageDurations;
  SequenceScore                   Scoring;
  vector<SequenceScoreValue>      ScoresPerCluster;
  double                          GlobalScore;

  ClusteringStatistics Statistics;
  Partition            DataPartition;

  ReadArgs(argc, argv);

  CSVDataExtractor CSVParser (globalArgs.ClustersFile);

  if (CSVParser.GetError())
  {
    cerr << "Error reading file: " << CSVParser.GetErrorMessage() << endl;
    exit(EXIT_FAILURE);
  }

  /* Extract the data from the CSV file */
  system_messages::information("**** READING INPUT ****\n");
  if (!CSVParser.ExtractData(DataSet))
  {
    cerr << "Error reading file: " << CSVParser.GetErrorMessage() << endl;
    exit(EXIT_FAILURE);
  }

  /* Retrieve the data partition present on the file */
  if (!CSVParser.GetPartition(DataPartition))
  {
    cerr << "Input file does not contain a cluster analysis results" << endl;
    exit(EXIT_FAILURE);
  }

  /* Compute the statistics */
  system_messages::information("**** COMPUTING SCORE ****\n");
  Statistics.InitStatistics(DataPartition.GetIDs());

  if (!Statistics.ComputeStatistics(DataSet->GetCompleteBursts(),
                                    DataPartition.GetAssignmentVector()))
  {
    cerr << "Unable to compute clusters statistics: ";
    cerr << Statistics.GetErrorMessage();
    cerr << endl;
    exit(EXIT_FAILURE);
  }

  /* Compute the Sequence Score */

  PercentageDurations = Statistics.GetPercentageDurations();

  if(!Scoring.ComputeScore(DataSet->GetClusteringBursts(),
                           DataPartition.GetAssignmentVector(),
                           PercentageDurations,
                           ScoresPerCluster,
                           GlobalScore,
                           globalArgs.ClustersFilePrefix,
                           true)) // Generate a FASTA file
  {
    cerr << "Unable to compute the Cluster Sequence Score: ";
    cerr << Scoring.GetErrorMessage() << endl;
    exit (EXIT_FAILURE);
  }

  system_messages::information("**** RESULTS ****\n");
  if (system_messages::verbose)
  {
    for (vector<SequenceScoreValue>::size_type i = 0;
         i < ScoresPerCluster.size();
         i++)
  {
    cout << fixed;
    if (i == NOISE_CLUSTERID)
    {
      cout << "NOISE      = ";
      cout << setprecision(6);
      cout << ScoresPerCluster[i].GetClusterScore() << endl;
    }
    else
    {
      cout << "Cluster " << setw(2) << setfill(' ');
      cout << ScoresPerCluster[i].GetID() << " = ";
      cout << setprecision(6);
      cout << ScoresPerCluster[i].GetClusterScore() << endl;

    }
  }
    cout << "-> Cluster Sequence Score = " << GlobalScore << endl;
  }
  else
  {
    cout << fixed;
    cout << setprecision(6);
    cout << GlobalScore << endl;
  }

  exit(EXIT_SUCCESS);
}
