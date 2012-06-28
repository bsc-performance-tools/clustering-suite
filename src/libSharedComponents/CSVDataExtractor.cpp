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

  $Id:: CSVDataExtractor.cpp 25 2011-07-13 #$:  Id
  $Rev:: 25                                       $:  Revision of last commit
  $Author:: jgonzale                              $:  Author of last commit
  $Date:: 2011-07-13 14:59:54 +0200 (Wed, 13 Jul #$:  Date of last commit

\* -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=- */

#include <SystemMessages.hpp>
using cepba_tools::system_messages;

#include "CSVDataExtractor.hpp"
#include "ParaverTraceParser.hpp"

#include <cstring>
#include <cerrno>

#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

#include <iostream>
using std::cout;
using std::cerr;
using std::endl;

#include <sstream>
using std::ostringstream;

#define DEBUG_PARAVER_INPUT 1

CSVDataExtractor::CSVDataExtractor(string InputTraceName)
:DataExtractor(InputTraceName)
{
  if (GetError())
    return;

  return;
}


CSVDataExtractor::~CSVDataExtractor()
{
  // unlink(TraceDataFileName.c_str());
}


bool CSVDataExtractor::ExtractData(TraceData* TraceDataSet)
{
  return true;
}
