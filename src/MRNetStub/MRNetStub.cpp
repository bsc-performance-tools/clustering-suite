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
#include <libDistributedClustering.hpp>

#include <SystemMessages.hpp>
using cepba_tools::system_messages;
#include <FileNameManipulator.hpp>
using cepba_tools::FileNameManipulator;

#include <iostream>
using std::cout;
using std::cerr;
using std::endl;

#include <sstream>
using std::ostringstream;

#include <cstring>
#include <cstdlib>
#include <cstdio>
#include <cerrno>

#include <Timer.hpp>

bool   Verbose = true;

string ClusteringDefinitionXML;    /* Clustering definition XML file name */
bool   ClusteringDefinitionRead = false;

double Epsilon;
bool   EpsilonRead = false;

int    MinPoints;
bool   MinPointsRead = false;

string InputTraceName;             /* Input trace name */
bool   InputTraceNameRead = false;

string OutputFileName;             /* Data extracted from input trace */
bool   OutputFileNameRead = false;

string OutputLocalDataFileName;
string OutputClassificationDataFileName;
string OutputModelsFileName;
bool   ReconstructTrace   = true;

bool   ApplyCPIStack = false;

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
"  -d <clustering_def_xml>    XML containing the clustering process\n"\
"                             definition\n"\
"\n"\
"  -e <epsilon>               Value of the epsilon parameter to use in DBSCAN\n"\
"\n"\
"  -m <min_points>            Value of the min_points parameter to use in DBSCAN\n"\
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
  cout << "Usage: " << ApplicationName << " [-s] -d <clustering_def.xml> [-b] [-m <eigen_matrix_file>";
  cout << "[-c] [-p <k>[,<k_end>]] [-x] [-r] -i <input_trace> <output_trace|output_data>" << endl;
}

void
ReadArgs(int argc, char *argv[])
{
  INT32 j = 1;
  INT32 ParametersRequired = 1;
  char* err;

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
        case 'e':
          j++;
          Epsilon = strtod(argv[j], &err);

          if (*err)
          {
            cerr << "Wrong epsilon value " << argv[j] << endl;
            exit(EXIT_FAILURE);
          }

          EpsilonRead = true;
          break;
        case 'm':
          j++;
          MinPoints = strtol(argv[j], &err, 0);

          if (*err)
          {
            cerr << "Wrong min_points value " << argv[j] << endl;
            exit(EXIT_FAILURE);
          }

          MinPointsRead = true;
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
        case 'c':
          ApplyCPIStack = true;
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

  if (!EpsilonRead)
  {
    cerr << "Epsilon value not defined ( \'-e\' parameter)" << endl;
    exit (EXIT_FAILURE);
  }

  if (!MinPointsRead)
  {
    cerr << "Min_Points value not defined ( \'-m\' parameter)" << endl;
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
    OutputLocalDataFileName          = NameManipulator.AppendStringAndExtension("LOCAL_CLUSTERING_DATA", "csv");
    OutputClassificationDataFileName = NameManipulator.AppendStringAndExtension("GLOBAL_CLASSIFICATION_DATA", "csv");
    OutputModelsFileName             = NameManipulator.AppendStringAndExtension ("MODELS", "csv");
    return;
  }
  else if (OutputFileExtension.compare("csv") == 0)
  {
    ReconstructTrace = false;
    OutputLocalDataFileName          = OutputFileName;
    OutputClassificationDataFileName = OutputFileName; 
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
  Timer Timing;
  set<int> TasksToRead;
  vector<ConvexHullModel> Hulls;

  ostringstream Messages;
  
  libDistributedClustering Clustering = libDistributedClustering(VERBOSE);
  
  ReadArgs(argc, argv);
  CheckOutputFile();
  
  if (!Clustering.InitClustering(ClusteringDefinitionXML,
                                 Epsilon,
                                 MinPoints,
                                 true,
                                 0,
                                 1)) // true == Root Task, 0 == MyRank, 1 = TotalTasks
  {
    cerr << "Error setting up clustering library: " << Clustering.GetErrorMessage() << endl;
    exit (EXIT_FAILURE);
  }

  system_messages::information("** DATA EXTRACTION **\n");
  if (!Clustering.ExtractData(InputTraceName, TasksToRead))
  {
    cerr << "Error extracting data: " << Clustering.GetErrorMessage() << endl;
    exit (EXIT_FAILURE);
  }
  
  Timing.begin();
  system_messages::information("** CLUSTER ANALYSIS **\n");
  if (!Clustering.ClusterAnalysis(Hulls))
  {
    cerr << "Error clustering data: " << Clustering.GetErrorMessage() << endl;
    exit (EXIT_FAILURE);
  }

  system_messages::information("** CLASSIFICATION **\n");
  if (!Clustering.ClassifyData(Hulls))
  {
    cerr << "Error classifying data: " << Clustering.GetErrorMessage() << endl;
    exit (EXIT_FAILURE);
  }
  Messages << "--> Analysis/Classification End Time " << Timing.end() << endl;
  system_messages::information(Messages.str().c_str());
  
  system_messages::information("** FLUSHING DATA **\n");
  if (!Clustering.PrintModels(Hulls, OutputModelsFileName))
  {
    cerr << "Error writing data points: " << Clustering.GetErrorMessage() << endl;
    exit (EXIT_FAILURE);
  }

  system_messages::information("** GENERATING LOCAL DATA GNUPlot SCRIPTS **\n");
  if (!Clustering.PrintPlotScripts(OutputLocalDataFileName, "", true )) // true = Local Partition
  {
    cerr << "Error printing plot scripts: " << Clustering.GetErrorMessage() << endl;
    exit (EXIT_FAILURE);
  }

  system_messages::information("** GENERATING CLASSIFICATION DATA GNUPlot SCRIPTS **\n");
  if (!Clustering.PrintPlotScripts(OutputClassificationDataFileName, "", false)) // false = Global Classification
  {
    cerr << "Error printing plot scripts: " << Clustering.GetErrorMessage() << endl;
    exit (EXIT_FAILURE);
  }

  exit(EXIT_SUCCESS);
  
  if (ReconstructTrace)
  {
    system_messages::information("** RECONSTRUCTING INPUT TRACE **\n");
    if (!Clustering.ReconstructInputTrace(OutputFileName))
    {
      cerr << "Error writing output trace: " << Clustering.GetErrorMessage() << endl;
      exit (EXIT_FAILURE);
    }
  }
  
  return 0;
}
