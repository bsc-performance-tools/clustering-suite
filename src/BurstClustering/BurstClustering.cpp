/*
 * main.cc
 * Copyright (C) Juan Gonzalez 2009 <juan.gonzalez@bsc.es>
 * 
 */

#include <types.h>
#include <libTraceClustering.hpp>

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

bool   Verbose = true;

string ClusteringDefinitionXML;    /* Clustering definition XML file name */
bool   ClusteringDefinitionRead = false;

string InputTraceName;             /* Input trace name */
bool   InputTraceNameRead = false;

string OutputFileName;             /* Data extracted from input trace */
bool   OutputFileNameRead = false;

string OutputDataFileName;
bool   ReconstructTrace   = true;

bool   ApplyCPIStack = false;

#define HELP \
"\n"\
"Usage:\n"\
"  %s -d <clustering_def.xml> [OPTIONS] -i <input_trace> [<output_trace>]\n"\
"\n"\
"  -a                         Information about the tool\n"\
"\n"\
"  -h                         This help\n"\
"\n"\
"  -s                         Do not show information messages (silent mode)\n"\
"\n"\
"  -d <clustering_def_xml>    XML containing the clustering process\n"\
"                             definition\n"\
"\n"\
"  -b                         When using Dimemas traces, print \"block begin\"\n"\
"                             and \"block end\" records for each burst on\n"\
"                             output trace\n"\
"\n"\
"  -c                         Generate the IBM PPC970 (r) CPIStack model report\n"\
"                             for each cluster found, if required hardware\n"\
"                             counters data are present on the input file\n"\
"\n"\
"  -p <k>[,k_end]             Computes the k-neighbour (or range) distance in\n"\
"                             terms of clustering parameter defined with '-d'.\n"\
"                             Generates an GNUPlot to easily select the DBScan\n"\
"                             parameters\n"\
"\n"\
"  -m <eigen_matrix_file>     CSV file containing an eigenvectors matrix to\n"\
"                             transform the original space\n"\
"\n"\
"  -a[n]                      Create cluster sequence to compute the alignment\n"\
"                             Using '-an' noise points are NOT FLUSHED in the\n"\
"                             resulting sequence\n"\
"\n"\
"  -t                         Generate the file used to create a tree trough\n"\
"                             successive clusterings\n"\
"\n"\
"  -i <input_file>            Input CSV / Dimemas trace / Paraver trace\n"\
"\n"\
"  -o <output_file>           Output CSV file / Dimemas trace / Paraver trace\n"\
"                             clustering process or output data file if\n"\
"                             parameters '-x' or '-r' are used\n"\
"\n"


#define ABOUT \
"\n"\
"%s v%s (%s)\n"\
"(c) CEPBA-Tools - Barcelona Supercomputing Center\n"\
"\n"\
"Automatic clustering analysis of Paraver/Dimemas traces and CSV files\n"\
"\n"

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

  if (argc == 1 ||
      argc == 2 &&
      ((strcmp(argv[1], "-h") == 0) || (strcmp(argv[1], "--help") == 0)))
  {
    fprintf(stdout, HELP, argv[0]);
    exit(EXIT_SUCCESS);
  }

  if (argc == 1 ||
      argc == 2 &&
      ((strcmp(argv[1], "-a") == 0)))
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
  libTraceClustering Clustering = libTraceClustering(true);
  
  ReadArgs(argc, argv);

  CheckOutputFile();

  if (!Clustering.InitTraceClustering(ClusteringDefinitionXML, ApplyCPIStack, CLUSTERING|PLOTS))
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
  
  return 0;
}
