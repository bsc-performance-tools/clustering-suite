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

  $Id:: libDistributedClustering.cpp 51 2011-11-2#$:  Id
  $Rev:: 51                                       $:  Revision of last commit
  $Author:: jgonzale                              $:  Author of last commit
  $Date:: 2011-11-24 15:47:29 +0100 (Thu, 24 Nov #$:  Date of last commit

\* -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=- */

#include <Timer.hpp>
using cepba_tools::Timer;

#include "ClusteringBackEndOffline.h"
#include "ClusteringTags.h"

#include <iostream>
using std::cout;
using std::cerr;
using std::endl;
#include <sstream>
using std::ostringstream;

/**
 * Initializes the clustering library for interactive use.
 * @param libClustering Instance of the clustering library.
 */
bool ClusteringBackEndOffline::InitLibrary(void)
{
   /* Initialize the clustering library */
   if (!libClustering->InitClustering(ClusteringDefinitionXML, Epsilon, MinPoints, (Protocol::WhoAmI() == 0), Protocol::WhoAmI(), Protocol::NumBackEnds())) // true == Root Task
   {
      cerr << "[BE " << Protocol::WhoAmI() << "] Error setting up clustering library: " << libClustering->GetErrorMessage() << endl;
      return false;
   }
   return true;
}


/**
 * Feeds the clustering library with points from a Paraver trace.
 * @param libClustering Instance of the clustering library.
 * @return true on success; false otherwise.
 */
bool ClusteringBackEndOffline::ExtractData(void)
{
   int tag;
   PACKET_new(p);

   if (!libClustering->ExtractData(InputTraceName))
   {
      cerr << "[BE " << Protocol::WhoAmI() << "] Error extracting data: " << libClustering->GetErrorMessage() << endl;
      return false;
   }
   cout << "[BE " << WhoAmI() << "] Bursts to analyze: " << libClustering->GetNumberOfPoints() << endl;

   /* In the offline version there's no need to reduce the dimensions, because all back-ends get them from
    * the trace they're parsing. We have to send something though, because the front-end is waiting this
    * message, as it makes no distinction between online/offline back-ends. I like this way more, just in
    * case the back-ends parse different data files in the future and may need to use this.
    */
   MRN_STREAM_SEND(stXchangeDims, TAG_XCHANGE_DIMENSIONS, "%alf %alf", NULL, 0, NULL, 0);
   MRN_STREAM_RECV(stXchangeDims, &tag, p, TAG_XCHANGE_DIMENSIONS);

   PACKET_delete(p);
   return true;
}


/**
 * Analyzes the data extracted from the Paraver trace.
 * @return true on success; false otherwise.
 */
bool ClusteringBackEndOffline::AnalyzeData(void)
{
   if (!libClustering->ClusterAnalysis(LocalModel))
   {
      cerr << "[BE " << WhoAmI() << "] Error clustering data: " << libClustering->GetErrorMessage() << endl;
      return false;
   }
   return true;
}


/**
 * Prints the output plots and reconstructs the trace if necessary.
 * @return true on success; false otherwise.
 */
bool ClusteringBackEndOffline::ProcessResults(void)
{
   /* Print the local model */
   ostringstream ModelTitle;
   Timer t;

   cout << "[BE " << WhoAmI() << "] Printing local model" << endl;

   ModelTitle << "Local Hull Models BE " << WhoAmI() << " MinPoints = " << MinPoints << " Eps = " << Epsilon << "\\n";
   ModelTitle << "Trace \'" << InputTraceName << "\'";

   if (!libClustering->PrintModels(LocalModel,
                                   LocalModelDataFileName,
                                   LocalModelPlotFileName,
                                   ModelTitle.str()))
   {
      cerr << "[BE " << WhoAmI() << "] Error printing local model scripts: " << libClustering->GetErrorMessage() << endl;
      return false;
   }


   if (WhoAmI() == 0)
   {
      /* Print the global model */
      cout << "[BE " << WhoAmI() << "] Printing global model script" << endl;

      ModelTitle.str("");
      ModelTitle << "Global Model MinPoints = " << MinPoints << " Eps = " << Epsilon << "\\n";
      ModelTitle << "Trace \'" << InputTraceName << "\'";

      if (!libClustering->PrintModels(GlobalModel,
                                      GlobalModelDataFileName,
                                      GlobalModelPlotFileName,
                                      ModelTitle.str()))
      {
        cerr << "[BE " << WhoAmI() << "] Error printing global model script: " << libClustering->GetErrorMessage() << endl;
        return false;
      }

      /* Print the data plots */
      cout << "[BE " << WhoAmI() << "] Printing global data plot script" << endl;
      if (!libClustering->PrintPlotScripts(OutputDataFileName, "", false)) // false = Global Classification
      {
        cerr << "[BE " << WhoAmI() << "] Error printing global data plot scripts: " << libClustering->GetErrorMessage() << endl;
        return false;
      }

      /* Reconstruct the trace */
      if (ReconstructTrace)
      {
        if (Verbose) cout << "[BE " << WhoAmI() << "] RECONSTRUCTING TRACE" << endl;

        t.begin();
        if (!libClustering->ReconstructInputTrace(OutputFileName))
        {
          cerr << "Error writing output trace: " << libClustering->GetErrorMessage() << endl;
          return false;
        }
        cout << "[BE " << WhoAmI() << "] Trace reconstruction time: " << t.end() << endl;
      }

      if (!libClustering->FlushClustersInformation(ClustersInformationFileName))
      {
        cerr << "Error writing clusters information file: " << libClustering->GetErrorMessage() << endl;
        return false;
      }
   }

   /* Print local clustering plots */
   cout << "[BE " << WhoAmI() << "] Printing local data plot script" << endl;
   if (!libClustering->PrintPlotScripts(OutputLocalClusteringFileName, "", true)) // true = Local partition
   {
      cerr << "[BE " << WhoAmI() << "] Error printing local data plot scripts: " << libClustering->GetErrorMessage() << endl;
      return false;
   }
   return true;
}

