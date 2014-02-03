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

  $URL:: https://svn.bsc.es/repos/ClusteringSuite#$:  File
  $Rev::                                          $:  Revision of last commit
  $Author::                                       $:  Author of last commit
  $Date::                                         $:  Date of last commit

\* -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=- */


#include <libTraceClustering.hpp>

#include <FileNameManipulator.hpp>
using cepba_tools::FileNameManipulator;

#include <string>
using std::string;

#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <errno.h>

#include <iostream>
using std::cout;
using std::cerr;
using std::endl;

#include <vector>
using std::vector;

#include <map>
using std::map;

#include <sstream>
using std::stringstream;

bool   Verbose = true;

string ClusteringDefinitionXML;    /* Clustering definition XML file name */
bool   ClusteringDefinitionRead = false;

string InputTraceName;             /* Input trace name */
string InputSemanticCSV;           /* Special case to integrate with Paraver */
string InputTraceNamePrefix;
bool   InputTraceNameRead   = false;
bool   InputSemanticCSVRead = false;

string OutputFileName;             /* Data extracted from input trace */
bool   OutputFileNameRead = false;

string OutputDataFileNamePrefix;

map<string, string> Parameters;
bool   KNeighbourValuesRead = false;

bool              UseParaverEventParsing = false;
bool              ConsecutiveEvts        = false;
set<unsigned int> EventsToParse;

void GetEventParsingParameters(char* EventParsingArgs);

#define HELP \
"%s v%s (%s %s)\n"\
"(c) BSC Tools - Barcelona Supercomputing Center\n"\
"\n"\
"Usage:\n"\
"  %s -d <clustering_def.xml> -p  -i <input_trace> -o <output_data>\n"\
"\n"\
"  -a                         Information about the tool\n"\
"\n"\
"  -h                         This help\n"\
"\n"\
"  -s                         Silent mode (No progress messages)\n"\
"\n"\
"  -d <clustering_def_xml>    XML containing the parameters definition\n"\
"\n"\
"  -p <k>[,k_end]             Value of 'k' for the k-neighbour (or range) distance\n"\
"                             in terms of clustering parameter defined with '-d'.\n"\
"\n"\
"  -e Type1,Type2,...         When using an input Paraver trace, use this\n"\
"                             event types to determine the regions treated as\n"\
"                             bursts\n"\
"\n"\
"  -i <input_trace>           Input trace\n"\
"\n"\
"  -o <output_data>           Output data file prefix\n"\
"\n"\


#define ABOUT \
"\n"\
"%s v%s (%s %s)\n"\
"(c) BSC Tools - Barcelona Supercomputing Center\n"\
"Utility to generate the k-neighbour distance graph\n"\
"\n"

void PrintUsage(char* ApplicationName)
{
  cout << "Usage: " << ApplicationName << " -d <clustering_def.xml>";
  cout << "-p <k>[,<k_end>] -e Type1,Type2,... -i <input_trace> -o <output_data>" << endl;
}

void
ReadKValues(char* argument)
{
  char* InternalCopy, *SubString;
  string k_begin, k_end;

  InternalCopy = (char*) malloc(strlen(argument)+1);
  strcpy(InternalCopy, argument);

  SubString = strtok(InternalCopy, ",");
  k_begin  = string(SubString);

  SubString = strtok(NULL,",\n");

  if (SubString == NULL)
  {
    k_end = k_begin;
  }
  else
  {
    k_end = string(SubString);
  }

  Parameters["k_begin"] = k_begin;
  Parameters["k_end"]   = k_end;

  /* DEBUG
  if (KNeighbour.size() == 1)
  {
    cout << "TEST -> " << KNeighbour[0] << endl;
  }
  else
  {
    cout << "KNeighbours size: " << KNeighbour.size() << endl;

    for (INT32 i = 0; i < KNeighbour.size(); i++)
    {
      cout << "TEST -> " << KNeighbour[i] << endl;
    }
  }
*/

  KNeighbourValuesRead = true;

  return;
}

void ReadArgs(int argc, char *argv[])
{
  INT32 j = 1;

  string InputFiles;
  size_t Comma;

  if (argc == 1 ||
      argc == 2 &&
      ((strcmp(argv[1], "-h") == 0) || (strcmp(argv[1], "--help") == 0)))
  {
    fprintf(stdout, HELP, argv[0], VERSION, __DATE__, __TIME__, argv[0]);
    exit(EXIT_SUCCESS);
  }

  if (argc == 1 ||
      argc == 2 &&
      ((strcmp(argv[1], "-a") == 0)))
  {
    fprintf(stdout, ABOUT, argv[0], VERSION, __DATE__, __TIME__);
    exit(EXIT_SUCCESS);
  }

  if (argv[1][0] == '-')
  {
    for (j = 1; (j < argc - 1) && (argv[j][0] == '-'); j++)
    {
      switch (argv[j][1])
      {
        case 's':
          Verbose = false;
          break;
        case 'd':
          j++;
          ClusteringDefinitionXML  = argv[j];
          ClusteringDefinitionRead = true;
          break;
        case 'e':
          UseParaverEventParsing = true;

          if (argv[j][2] == 'c')
          {
            ConsecutiveEvts = true;
          }

          j++;
          GetEventParsingParameters(argv[j]);
          break;
        case 'i':
          j++;
          InputFiles = argv[j];
          Comma      = InputFiles.find(',');

          if (Comma != string::npos)
          {
            InputSemanticCSV      = InputFiles.substr(0, Comma);
            InputTraceName        = InputFiles.substr(Comma+1);
            InputSemanticCSVRead  = true;

            /* DEBUG */
            cout << "InputSemanticCSV = " << InputSemanticCSV << endl;
            cout << "InputTraceName = " << InputTraceName << endl;
          }
          else
          {
            InputTraceName     = InputFiles;
          }

          InputTraceNameRead = true;
          break;
        case 'p':
          j++;
          ReadKValues(argv[j]);
          break;
        case 'o':
          j++;
          OutputFileName     = argv[j];
          OutputFileNameRead = true;
          break;
        default:
          cerr << "Invalid parameter " << argv[j][1] << endl;
          PrintUsage(argv[0]);
          exit(EXIT_FAILURE);
          break;
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

  if (!KNeighbourValuesRead)
  {
    cerr << "K-Neighbour values missing ( \'-p\' parameter)" << endl;
    exit (EXIT_FAILURE);
  }

  return;
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

void GenerateOutputFileNamePrefix()
{
  string OutputFileExtension;

  OutputFileExtension = FileNameManipulator::GetExtension(InputTraceName);

  if (OutputFileExtension.compare("") == 0)
  {
    cerr << "Unable to determine input file type. Please use .prv/.trf extensions" << endl;
    exit (EXIT_FAILURE);
  }

  if (OutputFileExtension.compare("prv") == 0 ||
      OutputFileExtension.compare("trf") == 0)
  {
    FileNameManipulator NameManipulator(InputTraceName, OutputFileExtension);

    OutputDataFileNamePrefix = NameManipulator.GetChoppedFileName();

    return;
  }
  else
  {
    cerr << "Unknown input file type. Please use .prv/.trf to choose the input file type" << endl;
    exit(EXIT_FAILURE);
  }

  return;
}

int main(int argc, char *argv[])
{
  libTraceClustering Clustering = libTraceClustering(true);

  ReadArgs(argc, argv);

  GenerateOutputFileNamePrefix();

  if (!Clustering.InitTraceClustering(ClusteringDefinitionXML,
                                      OutputDataFileNamePrefix+".pcf",
                                      false, // IMPORTANTE: NEED TO BE CHECKED
                                      false, // IMPORTANTE: NEED TO BE CHECKED
                                      PARAMETER_APPROXIMATION))
  {
    cerr << "Error seting up clustering library: " << Clustering.GetErrorMessage() << endl;
    exit (EXIT_FAILURE);
  }

  if (UseParaverEventParsing)
  {
    if (!Clustering.ExtractData(InputTraceName, false, 0, EventsToParse)) // False -> No Sampling, 0 -> MaxSamples
    {
      cerr << "Error extracting data: " << Clustering.GetErrorMessage() << endl;
      exit (EXIT_FAILURE);
    }
  }
  else
  {
    if (!Clustering.ExtractData(InputTraceName,
                                EventsToParse,
                                ConsecutiveEvts,
                                InputSemanticCSV))
    {
      cerr << "Error extracting data: " << Clustering.GetErrorMessage() << endl;
      exit (EXIT_FAILURE);
    }
  }

  if (!Clustering.ParametersApproximation(OutputFileName, Parameters))
  {
    cerr << "Error printing plot scripts: " << Clustering.GetErrorMessage() << endl;
    exit (EXIT_FAILURE);
  }

  exit (EXIT_SUCCESS);
}
