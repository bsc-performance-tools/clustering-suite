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

#include "ClusteringBackEndOffline.h"
#include "ClusteringTags.h"

/**
 * Initializes the clustering library for interactive use.
 * @param libClustering Instance of the clustering library.
 */
bool ClusteringBackEndOffline::InitLibrary (void)
{

  /* Initialize the clustering library */
  if (!libClustering->InitClustering (ClusteringDefinitionXML,
                                      Epsilon,
                                      MinPoints,
                                      (Protocol::WhoAmI() == 0),
                                      Protocol::WhoAmI(),
                                      Protocol::NumBackEnds())) // true == Root Task
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
bool ClusteringBackEndOffline::ExtractData (void)
{
  ostringstream Messages;

  int tag;
  PACKET_new (p);

  cepba_tools::Timer t;
  t.begin();

  if (!libClustering->ExtractData (InputTraceName) )
  {
    Messages << "Error extracting data: " << libClustering->GetErrorMessage() << endl;
    system_messages::information (Messages.str(), stderr);
    return false;
  }


  /* In the offline version there's no need to reduce the dimensions, because all back-ends get them from
   * the trace they're parsing. We have to send something though, because the front-end is waiting this
   * message, as it makes no distinction between online/offline back-ends. I like this way more, just in
   * case the back-ends parse different data files in the future and may need to use this.
   */
  MRN_STREAM_SEND (stXchangeDims, TAG_XCHANGE_DIMENSIONS, "%alf %alf", NULL, 0, NULL, 0);
  MRN_STREAM_RECV (stXchangeDims, &tag, p, TAG_XCHANGE_DIMENSIONS);

  PACKET_delete (p);
  return true;
}


/**
 * Analyzes the data extracted from the Paraver trace.
 * @return true on success; false otherwise.
 */
bool ClusteringBackEndOffline::AnalyzeData (void)
{
  cepba_tools::Timer t;
  t.begin();

  if (!libClustering->ClusterAnalysis (LocalModel) )
  {
    ostringstream Messages;
    Messages << "Error clustering data: " << libClustering->GetErrorMessage() << endl;
    system_messages::information (Messages.str(), stderr);
    return false;
  }

  system_messages::show_timer ("Local clustering time", t.end() );
  return true;
}


/**
 * Prints the output plots and reconstructs the trace if necessary.
 * @return true on success; false otherwise.
 */
bool ClusteringBackEndOffline::ProcessResults (void)
{
  /* Print the local model */
  ostringstream ModelTitle, Messages;
  cepba_tools::Timer t;

  system_messages::information ("Printing local model\n");

  ModelTitle << "Local Hull Models BE " << WhoAmI() << " MinPoints = " << MinPoints << " Eps = " << Epsilon << "\\n";
  ModelTitle << "Trace \'" << InputTraceName << "\'";

  if (!libClustering->PrintModels (LocalModel,
                                   LocalModelDataFileName,
                                   LocalModelPlotFileNamePrefix,
                                   ModelTitle.str() ) )
  {
    Messages.str ("");
    Messages << "Error printing local model scripts: " << libClustering->GetErrorMessage() << endl;
    system_messages::information (Messages.str(), stderr);
    return false;
  }


  if (WhoAmI() == 0)
  {
    /* Print the global model */
    Messages.str ("");
    system_messages::information ("Printing global model script\n");

    ModelTitle.str ("");
    ModelTitle << "Global Model MinPoints = " << MinPoints << " Eps = " << Epsilon << "\\n";
    ModelTitle << "Trace \'" << InputTraceName << "\'";

    if (!libClustering->PrintModels (GlobalModel,
                                     GlobalModelDataFileName,
                                     GlobalModelPlotFileNamePrefix,
                                     ModelTitle.str() ) )
    {
      Messages.str ("");
      Messages << "Error printing global model script: " << libClustering->GetErrorMessage() << endl;
      system_messages::information (Messages.str(), stderr);
      return false;
    }

    /* Print the data plots */
    system_messages::information ("Printing global data plot script\n");

    if (!libClustering->PrintPlotScripts (OutputDataFileName, "", false) ) // false = Global Classification
    {
      Messages.str ("");
      Messages << "Error printing global data plot scripts: " << libClustering->GetErrorMessage() << endl;
      system_messages::information (Messages.str(), stderr);
      return false;
    }

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

    if (!libClustering->FlushClustersInformation (ClustersInformationFileName) )
    {
      Messages.str ("");
      Messages << "Error writing clusters information file: " << libClustering->GetErrorMessage() << endl;
      system_messages::information (Messages.str(), stderr);
      return false;
    }
  }

  /* Print local clustering plots */

  system_messages::information ("Printing local data plot script\n");

  if (!libClustering->PrintPlotScripts (OutputLocalClusteringFileName, "", true) ) // true = Local partition
  {
    Messages.str ("");
    Messages << "Error printing local data plot scripts: " << libClustering->GetErrorMessage() << endl;
    system_messages::information (Messages.str(), stderr);
    return false;
  }

  return true;
}

