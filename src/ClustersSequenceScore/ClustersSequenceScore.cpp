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

#include <Utilities.hpp>
#include <SystemMessages.hpp>
using cepba_tools::system_messages;
#include <FileNameManipulator.hpp>
using cepba_tools::FileNameManipulator;
#include <Timer.hpp>
using cepba_tools::Timer;


#include <cstring>
#include <cstdlib>
#include <cstdio>
#include <cerrno>

#include <unistd.h>
#include <getopt.h>

#include <iostream>
using std::cout;
using std::cerr;
using std::endl;

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
  bool   UseNoise;
} globalArgs;

#define ABOUT \
"ClustersSequenceScore version 1.0\n"\
"(c) CEPBA-Tools - Barcelona Supercomputing Center\n"\
"Cluster Sequence Score a cluster analysis results\n"

#define HELP \
"Usage: ClustersSequenceScore [options] <clusters_data_csv>\n"\
"  -n  : Do not consider noise points to compute the score (Default: Yes)\n"\
"  -v  : Verbose mode\n"\
"  -h  : Print this help\n"

void PrintUsage(void)
{
  cout << ABOUT;
  cout << HELP;
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
  globalArgs.verbosity           = false;
  globalArgs.ClustersFile        = "";
  globalArgs.UseNoise            = true;


  while( (opt = getopt( argc, argv, "nvh")) != -1 )
  {
    switch( opt )
    {
      case 'n':
        globalArgs.UseNoise = false;
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

  /* Las parameter is the clusters csv file name */
  globalArgs.ClustersFile = argv[optind];

  return;
}

int main(int argc, char *argv[])
{
  Timer          T;

  ReadArgs(argc, argv);

  std::cout << "Hello Clusters!" << endl;

  exit(EXIT_SUCCESS);
}
