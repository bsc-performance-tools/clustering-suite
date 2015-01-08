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

  $Id::                                       $:  Id
  $Rev::                                          $:  Revision of last commit
  $Author::                                       $:  Author of last commit
  $Date::                                         $:  Date of last commit

\* -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=- */

#include <FrontEnd.h>
#include <stdlib.h>
#include <unistd.h>
#include "TDBSCAN_FE.h"
#include "TDBSCANRoot.h"

#include <iostream>
using std::cout;
using std::cerr;
using std::endl;

#include <cstring>

#include <fstream>

/* Configuration variables */
double Epsilon   = -1;
int    MinPoints = -1;
string ClusteringDefinitionXML;  /* Clustering definition XML file name -- Epsilon and MinPoints are ignored! */
string InputTraceName;           /* Input trace name */
string OutputFileName;           /* Data extracted from input trace */
bool   Verbose          = true;
bool   ReconstructTrace = false;
string TDBSCAN_HOME;

#if defined(BACKEND_ATTACH)

int main (int argc, char *argv[])
{
  int status;

  /* Parse input argumens */
  ReadArgs (argc, argv);

  /* Create an MRNet front-end */
  FrontEnd *FE = new FrontEnd();

  if (FE->Init () == -1)
  {
    cerr << "MRNet front-end could not be initialized due to previous errors." << endl;
    exit (EXIT_FAILURE);
  }

  /* Load the clustering protocol */
  FrontProtocol *protClustering = new TDBSCANRoot (Epsilon, MinPoints, ClusteringDefinitionXML, InputTraceName, OutputFileName, Verbose, ReconstructTrace);
  FE->LoadProtocol ( protClustering );

  /* Tell the back-ends to run the clustering protocol */
  FE->Dispatch ("TDBSCAN", status);

  /* Shutdown the network */
  FE->Shutdown();

  return 0;
}

#else

/**
 * The front-end application parses the configuration parameters,
 * loads the TDBSCAN protocol and starts the analysis right away.
 * @param argc Number of arguments.
 * @param argv Array of arguments.
 * @return 0 on success; -1 otherwise.
 */
int main (int argc, char *argv[])
{
  int status;

  /* Parse input argumens */
  ReadArgs (argc, argv);

  /* Create an MRNet front-end */
  FrontEnd *FE = new FrontEnd();
  const char **BE_argv = (const char **) (& (argv[1]) );

  cout << "Starting the network..." << endl;
  cout.flush();
  if (FE->Init (string (TDBSCAN_HOME + "/bin/TDBSCAN_BE").c_str(), BE_argv) == -1)
  {
    cerr << "MRNet front-end could not be initialized due to previous errors." << endl;
    exit (EXIT_FAILURE);
  }
  cout << "Network started." << endl;
  cout.flush();

  cout << "Loading protocols..." << endl;
  cout.flush();
  /* Load the clustering protocol */
  FrontProtocol *protClustering = new TDBSCANRoot (Epsilon, MinPoints, ClusteringDefinitionXML, InputTraceName, OutputFileName, Verbose, ReconstructTrace);
  FE->LoadProtocol ( protClustering );

  /* Tell the back-ends to run the clustering protocol */
  FE->Dispatch ("TDBSCAN", status);

  /* Shutdown the network */
  FE->Shutdown();

  return 0;
}

#endif

/**
 * Parse the input parameters.
 * @param argc Number of arguments.
 * @param argv Array of arguments.
 */
void ReadArgs (int argc, char *argv[])
{
  char *env_TDBSCAN_HOME     = NULL;
  bool  ClusteringDefinitionRead = false;
  bool  InputTraceNameRead       = false;
  bool  OutputFileNameRead       = false;

  INT32 j = 1;
  INT32 ParametersRequired = 1;

  if (argc == 1 ||
      argc == 2 &&
      ( (strcmp (argv[1], "-h") == 0) || (strcmp (argv[1], "--help") == 0) ) )
  {
    fprintf (stdout, HELP, argv[0]);
    exit (EXIT_SUCCESS);
  }

  if (argc == 1 ||
      argc == 2 &&
      ( (strcmp (argv[1], "-v") == 0 || (strcmp (argv[1], "--version") == 0) ) ) )
  {
    fprintf (stdout, ABOUT, argv[0], VERSION, __DATE__);
    exit (EXIT_SUCCESS);
  }

  if (argv[1][0] == '-')
  {
    for (j = 1; (j < argc) && (argv[j][0] == '-'); j++)
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
        case 'r':
          ReconstructTrace = true;
          break;
        case 'e':
          j++;
          Epsilon = atof (argv[j]);

          if (Epsilon <= 0)
          {
            cerr << "**** INVALID PARAMETER '-e " << argv[j] << "' **** " << endl << endl;
            exit (EXIT_FAILURE);
          }

          break;
        case 'm':
          j++;
          MinPoints = atoi (argv[j]);

          if (MinPoints <= 0)
          {
            cerr << "**** INVALID PARAMETER '-m " << argv[j] << "' **** " << endl << endl;
            exit (EXIT_FAILURE);
          }

          break;
        default:
          cerr << "**** INVALID PARAMETER '" << argv[j][1] << "' **** " << endl << endl;
          PrintUsage (argv[0]);
          exit (EXIT_FAILURE);
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

  env_TDBSCAN_HOME = getenv ("TDBSCAN_HOME");

  if (env_TDBSCAN_HOME == NULL)
  {
    cerr << "ERROR: Missing environment variable 'TDBSCAN_HOME'" << endl;
    cerr << "Run 'source etc/sourceme.sh' in the TDBSCAN installation directory and try again." << endl;
    exit (EXIT_FAILURE);
  }

  TDBSCAN_HOME = string (env_TDBSCAN_HOME);

  /* Convert relative to absolute paths */
  string CWD = get_current_dir_name();

  if (ClusteringDefinitionXML[0] != '/')
  {
    ClusteringDefinitionXML = CWD + "/" + ClusteringDefinitionXML;
  }

  if (InputTraceName[0] != '/')
  {
    InputTraceName = CWD + "/" + InputTraceName;
  }

  if (OutputFileName[0] != '/')
  {
    OutputFileName = CWD + "/" + OutputFileName;
  }

  /* Check the files exist and are readable */
  std::ifstream fd_XML (ClusteringDefinitionXML.c_str() );

  if (!fd_XML.good() )
  {
    cerr << "Definition XML file '" << ClusteringDefinitionXML << "' does not exist!" << endl;
    exit (EXIT_FAILURE);
  }

  std::ifstream fd_Input (InputTraceName.c_str() );

  if (!fd_Input.good() )
  {
    cerr << "Input trace '" << InputTraceName << "' does not exist!" << endl;
    exit (EXIT_FAILURE);
  }

  return;
}


/**
 * Print a help message.
 */
void PrintUsage (char* ApplicationName)
{
  cout << "Usage: " << ApplicationName;
  cout << " [-sr] [-e <epsilon>] [-m <min_points>]";
  cout << " -d <clustering_def.xml> -i <input_trace> -o <output_trace>";
  cout << endl;
}

