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

  $URL:: https://svn.bsc.es/repos/ptools/prv2dim/                          $:

  $Rev:: 478                        $:  Revision of last commit
  $Author:: jgonzale                $:  Author of last commit
  $Date:: 2010-10-28 13:58:59 +0200 $:  Date of last commit

\* -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=- */

#include <types.h>
#include <libTraceClustering.hpp>

#include <SystemMessages.hpp>
using cepba_tools::system_messages;
#include <FileNameManipulator.hpp>
using cepba_tools::FileNameManipulator;
#include <Timer.hpp>

#include <iostream>
using std::cout;
using std::cerr;
using std::endl;

#include <sstream>

#include <cstring>
#include <cstdlib>
#include <cstdio>
#include <cerrno>

#include <mpi.h>

bool   Verbose = true;

bool   MessagesFromAllRanks = false;

string ClusteringDefinitionXML;    /* Clustering definition XML file name */
bool   ClusteringDefinitionRead = false;

string InputTraceName;             /* Input trace name */
bool   InputTraceNameRead = false;

string OutputFileName;             /* Data extracted from input trace */
bool   OutputFileNameRead = false;

string OutputDataFileName;
bool   ReconstructTrace   = true;


#define HELP \
"\n"\
"Usage:\n"\
"  %s -d <clustering_def.xml> [OPTIONS] -i <input_trace> [<output_trace>]\n"\
"\n"\
"  -v|--version               Information about the tool\n"\
"\n"\
"  -h                         This help\n"\
"\n"\
"  -s                         Do not show information messages (silent mode)\n"\
"\n"\
"  -v                         Show information messages from all ranks\n"\
"\n"\
"  -d <clustering_def_xml>    XML containing the clustering process\n"\
"                             definition\n"\
"\n"\
"  -b                         When using Dimemas traces, print \"block begin\"\n"\
"                             and \"block end\" records for each burst on\n"\
"                             output trace\n"\
"\n"\
"  -a[n]                      Create cluster sequence to compute the alignment\n"\
"                             Using '-an' noise points are NOT FLUSHED in the\n"\
"                             resulting sequence\n"\
"\n"\
"  -i <input_file>            Input CSV / Dimemas trace / Paraver trace\n"\
"\n"\
"  -o <output_file>           Output CSV file / Dimemas trace / Paraver trace\n"\
"                             clustering process or output data file if\n"\
"                             parameters '-x' or '-r' are used\n"\
"\n"


#define ABOUT \
"%s v%s (%s)\n"\
"(c) CEPBA-Tools - Barcelona Supercomputing Center\n"\
"Automatic clustering analysis of Paraver/Dimemas traces and CSV files\n"

void PrintUsage(char* ApplicationName)
{
  cout << "Usage: " << ApplicationName << " [-s] [-v] -d <clustering_def.xml> [-b]";
  cout << "-i <input_trace> <output_trace|output_data>" << endl;
}

void
ReadArgs(int argc, char *argv[])
{
  INT32 j = 1;
  INT32 ParametersRequired = 1;

  if (argc == 1 ||
      argc == 2 &&
      ((strcmp(argv[1], "-h") == 0) || (strcmp(argv[1], "--help") == 0)))
  {
    fprintf(stdout, HELP, argv[0]);
    exit(EXIT_SUCCESS);
  }

  if (argc == 1 ||
      argc == 2 &&
      ((strcmp(argv[1], "-v") == 0 || (strcmp(argv[1], "--version") == 0))))
  {
    fprintf(stdout, ABOUT, argv[0], VERSION, __DATE__);
    exit(EXIT_SUCCESS);
  }

  if (argv[1][0] == '-')
  {
    for (j = 1; (j < argc - 1) && (argv[j][0] == '-'); j++)
    {
      switch (argv[j][1])
      {
        case 'd':
          j++;
          ClusteringDefinitionXML  = argv[j];
          ClusteringDefinitionRead = true;
          break;
        case 'i':
          j++;
          InputTraceName     = argv[j];
          InputTraceNameRead = true;
          break;
        case 'o':
          j++;
          OutputFileName     = argv[j];
          OutputFileNameRead = true;
          break;
        case 's':
          Verbose = false;
          break;
        case 'v':
          if (Verbose)
          {
            MessagesFromAllRanks = true;
          }
          break;
        default:
          cerr << "**** INVALID PARAMETER " << argv[j][1] << " **** " << endl << endl;
          PrintUsage(argv[0]);
          exit(EXIT_FAILURE);
      }
    }
  }

  if (!ClusteringDefinitionRead)
  {
    cerr << "Definition XML file missing ( \'-d\' parameter)" << endl;
    exit (EXIT_FAILURE);
  }

  if (!InputTraceNameRead)
  {
    cerr << "Input trace missing ( \'-i\' parameter)" << endl;
    exit (EXIT_FAILURE);
  }

  if (!OutputFileNameRead)
  {
    cerr << "Output data file file missing ( \'-o\' parameter)" << endl;
    exit (EXIT_FAILURE);
  }

  return;
}

void CheckOutputFile()
{
  string OutputFileExtension;

  OutputFileExtension = FileNameManipulator::GetExtension(OutputFileName);

  if (OutputFileExtension.compare("") == 0)
  {
    cerr << "Unable to determine output file type. Please use .prv/.trf/.csv extensions" << endl;
    exit (EXIT_FAILURE);
  }

  if (OutputFileExtension.compare("prv") == 0 ||
      OutputFileExtension.compare("trf") == 0)
  {
    FileNameManipulator NameManipulator(OutputFileName, OutputFileExtension);
    OutputDataFileName = NameManipulator.AppendStringAndExtension("DATA", "csv");
    return;
  }
  else if (OutputFileExtension.compare("csv") == 0)
  {
    ReconstructTrace = false;
    OutputDataFileName = OutputFileName;
  }
  else
  {
    cerr << "Unknown output file type. Please use .prv/.trf/.csv to choose the output file type" << endl;
    exit(EXIT_FAILURE);
  }

  return;
}

int main(int argc, char *argv[])
{
  int my_rank;
  
  MPI_Init(&argc, &argv);
  
  libTraceClustering Clustering = libTraceClustering(true);
  
  ReadArgs(argc, argv);

  /* Set the system messages variables */
  MPI_Comm_rank(MPI_COMM_WORLD, &my_rank);
  
  system_messages::verbose                 = Verbose;
  system_messages::distributed             = true;
  system_messages::my_rank                 = my_rank;
  system_messages::messages_from_all_ranks = MessagesFromAllRanks;
  
  CheckOutputFile();

  if (!Clustering.InitTraceClustering(ClusteringDefinitionXML, CLUSTERING|PLOTS|MPI))
  {
    cerr << "Error setting up clustering library: " << Clustering.GetErrorMessage() << endl;
    exit (EXIT_FAILURE);
  }

  system_messages::information("** DATA EXTRACTION **\n");
  if (!Clustering.ExtractData(InputTraceName))
  {
    cerr << "Error extracting data: " << Clustering.GetErrorMessage() << endl;
    exit (EXIT_FAILURE);
  }

  system_messages::information("** CLUSTER ANALYSIS **\n");
  if (!Clustering.ClusterAnalysis())
  {
    cerr << "Error clustering data: " << Clustering.GetErrorMessage() << endl;
    exit (EXIT_FAILURE);
  }

  // exit (EXIT_SUCCESS);

  if (my_rank == 0)
  {
  
  system_messages::information("** FLUSHING DATA **\n");
  if (!Clustering.FlushData(OutputDataFileName))
  {
    cerr << "Error writing data points: " << Clustering.GetErrorMessage() << endl;
    exit (EXIT_FAILURE);
  }

  system_messages::information("** GENERATING GNUPlot SCRIPTS **\n");
  if (!Clustering.PrintPlotScripts(OutputDataFileName))
  {
    cerr << "Error printing plot scripts: " << Clustering.GetErrorMessage() << endl;
    exit (EXIT_FAILURE);
  }

  if (ReconstructTrace)
  {
    system_messages::information("** RECONSTRUCTING INPUT TRACE **\n");
    if (!Clustering.ReconstructInputTrace(OutputFileName))
    {
      cerr << "Error writing output trace: " << Clustering.GetErrorMessage() << endl;
      exit (EXIT_FAILURE);
    }
  }

  }

  MPI_Finalize();
  exit (EXIT_SUCCESS);
  
  
  return 0;
}
