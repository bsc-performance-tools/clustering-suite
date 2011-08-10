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

#include <types.h>
#include <libTraceClustering.hpp>

#include <Utilities.hpp>
#include <SystemMessages.hpp>
using cepba_tools::system_messages;
#include <FileNameManipulator.hpp>
using cepba_tools::FileNameManipulator;

#include <iostream>
using std::cout;
using std::cerr;
using std::endl;

#include <cstring>
#include <cstdlib>
#include <cstdio>
#include <cerrno>

#include <string>
#include <sstream>
using std::stringstream;

#include <vector>
using std::vector;

#include <set>
using std::set;

bool   Verbose = true;

string ClusteringDefinitionXML;    /* Clustering definition XML file name */
bool   ClusteringDefinitionRead = false;

string InputTraceName;             /* Input trace name */
bool   InputTraceNameRead = false;

string OutputFileName;             /* Data extracted from input trace */
bool   OutputFileNameRead = false;

string OutputDataFileName;         /* File where the burst data is stored */

string ClusterSequencesFileName;
bool   GenerateClusterSequences = false;

bool   ClusteringRefinement = false;
bool   DivisiveRefinement   = true;
bool   AutomaticRefinement  = false;

double MaxEps, MinEps;
int    MinPoints, Steps;

bool   ReconstructTrace   = true;

string OutputClustersInformationFileName;
bool   FASTASequences = false;

string RefinementPrefixFileName = "";
bool   PrintRefinementSteps     = false;

bool              UseParaverEventParsing = false;
set<unsigned int> EventsToParse;

bool   ApplyCPIStack = false;

#define HELP \
"\n"\
"Usage:\n"\
"  %s -d <clustering_def.xml> [OPTIONS] -i <input_trace> [<output_trace>]\n"\
"\n"\
"  -v |--version               Information about the tool\n"\
"\n"\
"  -h                          This help\n"\
"\n"\
"  -s                          Do not show information messages (silent mode)\n"\
"\n"\
"  -e Type1,Type2,...          When using an input Paraver trace, use this\n"\
"                              event types to determine the regions treated as\n"\
"                              bursts\n"\
"\n"\
"  -d <clustering_def_xml>     XML containing the clustering process\n"\
"                              definition\n"\
"\n"\
"  -a[f]                       Generate a file containing the cluster sequences\n"\
"                              (using 'f' generates a FASTA aminoacid sequences)\n"\
"\n"\
"  -r<d|a>[p] <min_points>,<max_eps>,<min_eps>,<steps>\n"\
"\n"\
"                              Applies a clustering refinement (d = divisve,\n"\
"                              a = aggregative) based on DBSCAN using min_points\n"\
"                              the epsilon range introduced\n"\
"                              Ignores the cluster algorithm used in the XML\n"\
"                              If 'p' option is included, plots traces and trees\n"\
"                              of each step are printed\n"\
"\n"\
"  -i <input_file>             Input CSV / Dimemas trace / Paraver trace\n"\
"\n"\
"  -o <output_file>            Output CSV file / Dimemas trace / Paraver trace\n"\
"                              clustering process or output data file if\n"\
"                              parameters '-x' or '-r' are used\n"\
"\n"


#define ABOUT \
"%s v%s (%s)\n"\
"(c) CEPBA-Tools - Barcelona Supercomputing Center\n"\
"Automatic clustering analysis of Paraver/Dimemas traces and CSV files\n"

void GetRefinementParameters(char*);
void GetEventParsingParameters(char*);

void PrintUsage(char* ApplicationName)
{
  cout << "Usage: " << ApplicationName << " [-s] -d <clustering_def.xml> ";
  cout << "-a[f] -r<d|a>[p] <min_points>,<max_eps>,<min_eps>,<steps> ";
  cout << "-i <input_file> -o <output_file>" << endl;
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
        case 'a':
          GenerateClusterSequences = true;
          if (argv[j][2] == 'f')
          {
            FASTASequences = true;
          }
          break;
        case 'r':
          ClusteringRefinement = true;
          if (argv[j][2] == 'd')
          {
            DivisiveRefinement = true;
          }
          else if (argv[j][2] == 'a')
          {
            DivisiveRefinement = false;
          }
          else
          {
            cerr << "Wrong type of refinement \'" << argv[j][2] << "\'" << endl;
            cerr << "It should be 'd' for divise or 'a' for aggregative" << endl;
            exit(EXIT_FAILURE);
          }
            
          if (argv[j][3] == 'p')
          {
            PrintRefinementSteps = true;
          }
          j++;
          
          if (argv[j][0] != '-')
          {
            GetRefinementParameters(argv[j]);
          }
          else
          {
            AutomaticRefinement = true;
            j--;
          }
          
          break;
        case 'e':

          UseParaverEventParsing = true;
          j++;
          GetEventParsingParameters(argv[j]);
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

void GetRefinementParameters(char* RefinementArgs)
{
  char* err;
  string       ArgsString (RefinementArgs);
  stringstream ArgsStream (ArgsString);

  string         Buffer;
  vector<string> Args;

  while(std::getline(ArgsStream, Buffer, ','))
  {
    Args.push_back(Buffer);
  }

  if (Args.size() != 4)
  {
    cerr << "Refinement parameters (\'-r\') not correctly defined (";
    cerr << RefinementArgs << ")" << endl;
    exit (EXIT_FAILURE);
  }

  MinPoints = strtol(Args[0].c_str(), &err, 0);
  if (*err)
  {
    cerr << "Error on refinement parameters (\'-r\'): Incorrect value of min_points ";
    cerr << "(" << Args[0] << ")" << endl;
    exit (EXIT_FAILURE);
  }
  
  MaxEps = strtod(Args[1].c_str(), &err);
  if (*err)
  {
    cerr << "Error on refinement parameters (\'-r\'): Incorrect maximum epsilon ";
    cerr << "(" << Args[1] << ")" << endl;
    exit (EXIT_FAILURE);
  }

  MinEps = strtod(Args[2].c_str(), &err);
  if (*err)
  {
    cerr << "Error on refinement parameters (\'-r\'): Incorrect minimum epsilon ";
    cerr << "(" << Args[2] << ")" << endl;
    exit (EXIT_FAILURE);
  }

  if (MinEps > MaxEps)
  {
    cerr << "Error on refinement parameters (\'-r\'): maximum epsilon smaller than minimum epsilon" << endl;
    exit(EXIT_FAILURE);
  }

  Steps = strtol(Args[3].c_str(), &err, 0);

  if (*err)
  {
    cerr << "Error on refinement parameters (\'-r\'): Incorrect number of steps ";
    cerr << "(" << Args[3] << ")" << endl;
    exit (EXIT_FAILURE);
  }
}

void GetEventParsingParameters(char* EventParsingArgs)
{
  char* err;
  
  string       ArgsString (EventParsingArgs);
  stringstream ArgsStream (ArgsString);
  string       Buffer;

  set<unsigned int>::iterator EventsIterator;

  while(std::getline(ArgsStream, Buffer, ','))
  {
    unsigned int CurrentType;
    
    CurrentType = strtoul(Buffer.c_str(), &err, 0);
    if (*err)
    {
      cerr << "Error on event parsing parameters (\'-e\'): Incorrect type ";
      cerr << "(" << Buffer << ")" << endl;
      exit (EXIT_FAILURE);
    }
    else
    {
      EventsToParse.insert(CurrentType);
    }
  }

  /* DEBUG */
  cout << "Events to parse: ";
  for (EventsIterator  = EventsToParse.begin();
       EventsIterator != EventsToParse.end();
       ++EventsIterator)
  {
    cout << (*EventsIterator) << " ";
  }
  cout << endl;
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
    OutputDataFileName                 = NameManipulator.AppendStringAndExtension("DATA", "csv");
    OutputClustersInformationFileName  = NameManipulator.AppendStringAndExtension("clusters_info", "csv");
    // ClusterSequencesFileName           = NameManipulator.AppendString("seq");
    ClusterSequencesFileName           = NameManipulator.GetChoppedFileName();
    if (PrintRefinementSteps)
    {
      RefinementPrefixFileName           = NameManipulator.GetChoppedFileName();
    }
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
  libTraceClustering Clustering = libTraceClustering(true);

  ReadArgs(argc, argv);

  CheckOutputFile();

  if (ClusteringRefinement)
  {
    if (!Clustering.InitTraceClustering(ClusteringDefinitionXML, CLUSTERING_REFINEMENT|PLOTS))
    {
      cerr << "Error setting up clustering library: " << Clustering.GetErrorMessage() << endl;
      exit (EXIT_FAILURE);
    }
  }
  else
  {
    if (!Clustering.InitTraceClustering(ClusteringDefinitionXML, CLUSTERING|PLOTS))
    {
      cerr << "Error setting up clustering library: " << Clustering.GetErrorMessage() << endl;
      exit (EXIT_FAILURE);
    }
  }

  if (Clustering.GetWarning())
  {
    cerr << "WARNING: " << Clustering.GetWarningMessage() << endl;
  }

  system_messages::information("** DATA EXTRACTION **\n");
  if (UseParaverEventParsing)
  {
    if (!Clustering.ExtractData(InputTraceName, EventsToParse))
    {
      cerr << "Error extracting data: " << Clustering.GetErrorMessage() << endl;
      exit (EXIT_FAILURE);
    }
  }
  else
  {
    if (!Clustering.ExtractData(InputTraceName))
    {
      cerr << "Error extracting data: " << Clustering.GetErrorMessage() << endl;
      exit (EXIT_FAILURE);
    }
  }

  if (ClusteringRefinement)
  {
    system_messages::information("** CLUSTER REFINEMENT ANALYSIS **\n");
    if (AutomaticRefinement)
    {
      if (!Clustering.ClusterRefinementAnalysis(DivisiveRefinement,
                                                RefinementPrefixFileName))
      {
        cerr << "Error clustering data: " << Clustering.GetErrorMessage() << endl;
        exit (EXIT_FAILURE);
      }
    }
    else
    {
      if (!Clustering.ClusterRefinementAnalysis(DivisiveRefinement,
                                                MinPoints,
                                                MaxEps,
                                                MinEps,
                                                Steps,
                                                RefinementPrefixFileName))
      {
        cerr << "Error clustering data: " << Clustering.GetErrorMessage() << endl;
        exit (EXIT_FAILURE);
      }
    }
  }
  else
  {
    system_messages::information("** CLUSTER ANALYSIS **\n");
    if (!Clustering.ClusterAnalysis())
    {
      cerr << "Error clustering data: " << Clustering.GetErrorMessage() << endl;
      exit (EXIT_FAILURE);
    }
  }

  if (GenerateClusterSequences)
  {
    system_messages::information("** GENERATING CLUSTER SEQUENCES **\n");
    if (!Clustering.ComputeSequenceScore(ClusterSequencesFileName, 
                                         FASTASequences))
    {
      cerr << "Error clustering data: " << Clustering.GetErrorMessage() << endl;
      exit (EXIT_FAILURE);
    }
  }
  
  system_messages::information("** FLUSHING DATA **\n");
  if (!Clustering.FlushData(OutputDataFileName))
  {
    cerr << "Error writing data points: " << Clustering.GetErrorMessage() << endl;
    exit (EXIT_FAILURE);
  }

  system_messages::information("** GENERATING CLUSTERS INFORMATION FILE **\n");
  if (!Clustering.FlushClustersInformation(OutputClustersInformationFileName))
  {
    cerr << "Error writing clusters information file: " << Clustering.GetErrorMessage() << endl;
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
  
  return 0;
}
