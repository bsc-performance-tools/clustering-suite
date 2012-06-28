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

  $Id:: BurstClustering.cpp 57 2011-12-13 14:29:3#$:  Id
  $Rev:: 57                                       $:  Revision of last commit
  $Author:: jgonzale                              $:  Author of last commit
  $Date:: 2011-12-13 15:29:33 +0100 (Tue, 13 Dec #$:  Date of last commit

\* -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=- */

#include <types.h>

#include "MirkinDistance.hpp"
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
  string ClustersFile1;
  bool   ClustersFile1Present;
  string ClustersFile2;
  bool   ClustersFile2Present;
  bool   UseNoise;
} globalArgs;

#define ABOUT \
"ClustersDiff version 1.0\n"\
"(c) CEPBA-Tools - Barcelona Supercomputing Center\n"\
"Mirkin distance of two different cluster analysis experiments with the same data\n"

#define HELP \
"Usage: ClustersDiff [options] -1 <clusters_csv_file1> -2 <clusters_csv_file2>\n"\
"  -n  : Consider points classified as noise to compute the distance\n"\
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
  globalArgs.verbosity            = false;
  globalArgs.ClustersFile1        = "";
  globalArgs.ClustersFile1Present = false;
  globalArgs.ClustersFile2        = "";
  globalArgs.ClustersFile2Present = false;
  globalArgs.UseNoise             = false;


  while( (opt = getopt( argc, argv, "1:2:nvh")) != -1 )
  {
    switch( opt )
    {
      case '1':
        globalArgs.ClustersFile1        = string(optarg);
        globalArgs.ClustersFile1Present = true;
        break;

      case '2':
        globalArgs.ClustersFile2        = string(optarg);
        globalArgs.ClustersFile2Present = true;
        break;

      case 'n':
        globalArgs.UseNoise = true;
        break;

      case 'v':
        globalArgs.verbosity = true;
        break;

      case 'h':   /* fall-through is intentional */
      case '?':
        PrintUsage();
        exit (EXIT_SUCCESS);

      default:
        cerr << "Wrong parameter!" << endl << endl;
        PrintUsage ();
        exit(EXIT_FAILURE);
        break;
    }
  }


  if (!globalArgs.ClustersFile1Present)
  {
    cerr << "You must specify first clusters CSV file ('-1')" << endl << endl;
    PrintUsage();
    exit(EXIT_FAILURE);
  }

  if (!globalArgs.ClustersFile2Present)
  {
    cerr << "You must specify second clusters CSV file ('-2')" << endl << endl;
    PrintUsage();
    exit(EXIT_FAILURE);
  }

  if (globalArgs.verbosity)
  {
    system_messages::verbose = true;
  }
}

int main(int argc, char *argv[])
{
  Timer          T;
  MirkinDistance DistanceCalculator;
  double         Distance;

  ReadArgs(argc, argv);


  if (!DistanceCalculator.GetMirkinDistance(globalArgs.ClustersFile1,
                                            globalArgs.ClustersFile2,
                                            globalArgs.UseNoise,
                                            Distance))
  {
    cerr << DistanceCalculator.GetLastError() << endl;
    exit(EXIT_FAILURE);
  }

  if (system_messages::verbose)
  {
    cout << "Mirkin Distance ";

    if (globalArgs.UseNoise)
      cout << "(with NOISE points) ";

    cout << "= " << Distance << endl;
  }
  else
  {
    cout << Distance << endl;
  }

  exit(EXIT_SUCCESS);
}
