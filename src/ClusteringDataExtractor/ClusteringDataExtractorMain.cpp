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


#include <types.h>
#include <libTraceClustering.hpp>

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

string ClusteringDefinitionXML;    /* Clustering definition XML file name */
bool   ClusteringDefinitionRead = false;

string InputTraceName;             /* Input trace name */
bool   InputTraceNameRead = false;

string OutputFileName;             /* Data extracted from input trace */
bool   OutputFileNameRead = false;

bool   ApplyCPIStack             = false;

string EigenMatrixFileName;

bool  ExtractNormalizedData      = false;

bool  ChangeBase                 = false;

#define HELP \
"Usage:\n"\
"  %s -d <clustering_def.xml> [-x] -i <input_trace> -o <output_data>\n"\
"\n"\
"  -a                         Information about the tool\n"\
"  -h                         This help\n"\
"  -s                         Silent mode (No progress messages)\n"\
"  -d <clustering_def_xml>    XML containing the parameters definition\n"\
"  -c                         Extract CPI stack data for PPC970 native counters\n"\
"  -x                         Generate plot for *normalized* data from input\n"\
"                             file (Default: *raw* data plot)\n"\
"  -m <eigen_matrix_file>     CSV file containing an eigenvectors matrix to\n"\
"                             transform the original space\n"\
"  -i <input_trace>           Input trace\n"\
"  -o <output_data>           Output data file\n"


#define ABOUT \
"\n"\
"%s v%s (%s)\n"\
"(c) CEPBA Tools - Barcelona Supercomputing Center\n"\
"\n"

void PrintUsage(char* ApplicationName)
{
  cout << "Usage: " << ApplicationName << " -d <clustering_def.xml>";
  cout << "[-x] [-m <eigen_matrix_file>] -i <input_trace> -o <output_data>" << endl;
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

  /*
  if (argc != 6 && argc != 7 && argc != 8)
  {
    PrintUsage(argv[0]);
    exit(EXIT_FAILURE);
  }
  */

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
        case 's':
          // State::SetSilentMode(false);
          break;
        case 'x':
          ExtractNormalizedData = false;
          break;
        case 'm':
          ChangeBase = true;
          j++;
          EigenMatrixFileName = argv[j];
          break;
        case 'o':
          j++;
          OutputFileName     = argv[j];
          OutputFileNameRead = true;
          break;
        /*
        case 'e':
          ExtractOnlyExtraDimensions = true;
          break;
        case 'g':
          GenerateGNUPlotScript = true;
          break;
        */
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
  
  return;
}

int main(int argc, char *argv[])
{
  libTraceClustering Clustering = libTraceClustering(true);
  
  ReadArgs(argc, argv);

  if (!Clustering.InitTraceClustering(ClusteringDefinitionXML, ApplyCPIStack, PLOTS))
  {
    cerr << "Error seting up clustering library: " << Clustering.GetErrorMessage() << endl;
    exit (EXIT_FAILURE);
  }

  if (!Clustering.ExtractData(InputTraceName))
  {
    cerr << "Error extracting data: " << Clustering.GetErrorMessage() << endl;
    exit (EXIT_FAILURE);
  }

  if (!Clustering.FlushData(OutputFileName))
  {
    cerr << "Error writing data points: " << Clustering.GetErrorMessage() << endl;
    exit (EXIT_FAILURE);
  }

  if (!Clustering.PrintPlotScripts(OutputFileName))
  {
    cerr << "Error printing plot scripts: " << Clustering.GetErrorMessage() << endl;
    exit (EXIT_FAILURE);
  }
  
  exit (EXIT_SUCCESS);
}