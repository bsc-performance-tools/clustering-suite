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

#include <iostream>
using std::cout;
using std::cerr;
using std::endl;
#include <sstream>
using std::ostringstream;

#include <Timer.hpp>
using cepba_tools::Timer;

#include <SystemMessages.hpp>
using cepba_tools::system_messages;

#include "TDBSCANWorkerOffline.h"
#include "TDBSCANTags.h"

/**
 * Initializes the clustering library for interactive use.
 * @param libClustering Instance of the clustering library.
 */
bool TDBSCANWorkerOffline::InitLibrary (void)
{

  /* Initialize the clustering library */
  if (!libClustering->InitClustering (ClusteringDefinitionXML,
                                      Epsilon,
                                      MinPoints,
                                      (Protocol::WhoAmI() == 0), // true == Root task
                                      Protocol::WhoAmI(),
                                      Protocol::NumBackEnds())) 
  {
    ostringstream Messages;
    Messages << "Error setting up clustering library: " << libClustering->GetErrorMessage() << endl;
    system_messages::information (Messages.str(), stderr);
    return false;
  }

  cepba_tools::system_messages::verbose                 = true;
  cepba_tools::system_messages::messages_from_all_ranks = false;
  cepba_tools::system_messages::print_timers            = true;
  return true;
}


/**
 * Feeds the clustering library with points from a Paraver trace.
 * @param libClustering Instance of the clustering library.
 * @return true on success; false otherwise.
 */
bool TDBSCANWorkerOffline::ExtractData (void)
{
  ostringstream Messages;

  cepba_tools::Timer t;
  t.begin();

  if (!libClustering->ExtractData (InputTraceName) )
  {
    Messages << "Error extracting data: " << libClustering->GetErrorMessage() << endl;
    system_messages::information (Messages.str(), stderr);
    return false;
  }

  return true;
}


/**
 * Reconstructs the trace if necessary.
 * @return true on success; false otherwise.
 */
bool TDBSCANWorkerOffline::ProcessResults (Support &GlobalSupport)
{
  ostringstream Messages;
  cepba_tools::Timer t;

  if (WhoAmI() == 0)
  {
    /* Reconstruct the trace */
    if (ReconstructTrace)
    {
      system_messages::information ("RECONSTRUCTING TRACE\n");

      t.begin();

      if (!libClustering->ReconstructInputTrace (OutputFileName) )
      {
        Messages.str ("");
        Messages << "Error writing output trace: " << libClustering->GetErrorMessage() << endl;
        system_messages::information (Messages.str(), stderr);
        return false;
      }

      system_messages::show_timer ("Trace reconstruction time", t.end() );
    }
  }

  return true;
}

