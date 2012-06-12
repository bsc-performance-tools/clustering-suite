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
#include "FileNameManipulator.hpp"
#include "ClusteringBackEnd.h"
#include "NoiseManager.h"
#include "HullManager.h"
#include "ClusteringTags.h"
#include "Utils.h"
#include "Statistics.h"

using namespace cepba_tools;

#include <iostream>
using std::cout;
using std::cerr;
using std::endl;

#include <sstream>
using std::ostringstream;


/**
 * TreeDBSCAN back-end protocol constructor.
 */
ClusteringBackEnd::ClusteringBackEnd()
{
   libClustering = NULL;
}


/**
 * Register the streams used in this protocol.
 */
void ClusteringBackEnd::Setup()
{
   Register_Stream(stClustering);
   Register_Stream(stXchangeDims);
}


/**
 * Implements the back-end side of the TreeDBSCAN clustering algorithm.
 * @return 0 on success; -1 otherwise.
 */
int ClusteringBackEnd::Run()
{
   int tag;
   PACKET_new(p);
   vector<HullModel*>::iterator it;
   cepba_tools::Timer t;
   Statistics ClusteringStats(WhoAmI());

   /* Delete any previous clustering */
   if (libClustering != NULL) delete libClustering;
   libClustering = new libDistributedClustering(Verbose);

   /* Receive clustering configuration from the front-end */
   Recv_Configuration();
   /* Prepare all outputs file names */
   CheckOutputFile();

   if (!InitLibrary())
   {
      cerr << "[BE " << WhoAmI() << "] Error initializing clustering. Exiting..." << endl;
      exit (EXIT_FAILURE);
   }

   if (Verbose) cout << "[BE " << WhoAmI() << "] EXTRACTING DATA" << endl;

   t.begin();
   if (!ExtractData())
   {
      cerr << "[BE " << WhoAmI() << "] Error extracting clustering data. Exiting..." << endl;
      exit (EXIT_FAILURE);
   }
   if (WhoAmI() == 0) cout << "[BE " << WhoAmI() << "] Data extraction time: " << t.end() << endl;

   ClusteringStats.IncreaseInputPoints( libClustering->GetNumberOfPoints() );

   /* Start the clustering analysis */
   t.begin();
   ClusteringStats.ClusteringTimeStart();
   if (!AnalyzeData())
   {
      cerr << "[BE " << WhoAmI() << "] Error analyzing data. Exiting..." << endl;
      exit (EXIT_FAILURE);
   }
   ClusteringStats.ClusteringTimeStop();
   
#if defined(PROCESS_NOISE)
   vector<const Point *> NoisePoints;
   libClustering->GetNoisePoints(NoisePoints);
   ClusteringStats.IncreaseOutputPoints( NoisePoints.size() );

   /* DEBUG -- count remaining noise points
   if (Verbose) cerr << "[BE " << WhoAmI() << "] Number of noise points = " << NoisePoints.size() << endl; */

   NoiseManager Noise = NoiseManager(libClustering);
   Noise.Serialize(stClustering);
#endif



   /* Send the local hulls */
   if (Verbose) cout << "[BE " << WhoAmI() << "] Sending " << LocalModel.size() << " local hulls" << endl;
   ClusteringStats.IncreaseOutputHulls( LocalModel.size() );

   HullManager HM = HullManager();
//   HM.Serialize(stClustering, LocalModel);
   HM.SerializeAll(stClustering, LocalModel);

#if 0
   /* Receive the resulting global hulls one by one */
   do
   {
      STREAM_recv(stClustering, &tag, p, TAG_HULL);
      if (tag == TAG_HULL)
      {
         HullModel *Hull = NULL;
         HullManager HM  = HullManager();
         Hull            = HM.Unpack(p);

         /* DEBUG
         std::cout << "[BE " << WhoAmI() << "] Received global Hull (size = " << Hull->Size() << " density = " << Hull->Density() << ")" << std::endl;
         Hull->Flush(); */

         GlobalModel.push_back(Hull);
      }
   } while (tag != TAG_ALL_HULLS_SENT);
#endif
   /* Receive 1 packet with all the resulting global hulls */
   do
   {
      STREAM_recv(stClustering, &tag, p, TAG_ANY);
      if (tag == TAG_ALL_HULLS)
      {
         HullManager HM  = HullManager();
         HM.Unpack(p, GlobalModel);
      }
   } while (tag != TAG_ALL_HULLS_SENT);

   /* Once the clustering is over, send the statistics to the root */
   ClusteringStats.Serialize(stClustering);

   if (Verbose) cout << "[BE " << WhoAmI() << "] Received " << GlobalModel.size() << " global hulls." << endl;
   // cout << "[BE " << WhoAmI() << "] >> Clustering time: " << t.end() << "[" << NumBackEnds() << " BEs]" << endl;

   /* All back-ends classify their local data */
   if (Verbose) cout << "[BE " << WhoAmI() << "] START CLASSIFYING WITH " << GlobalModel.size() << " GLOBAL HULLS." << endl;
   
   // t.begin();
   if (!libClustering->ClassifyData(GlobalModel))
   {
      cerr << "[BE " << WhoAmI() << "] Error classifying data: " << libClustering->GetErrorMessage() << endl;
      exit (EXIT_FAILURE);
   }
   if (WhoAmI() == 0) cout << "[BE " << WhoAmI() << "] Clustering time: " << t.end() << endl;

   /* Process the results and generate the output files */
   if (!ProcessResults())
   {
      cerr << "[BE " << WhoAmI() << "] Error processing clustering results. Exiting..." << endl;
      exit (EXIT_FAILURE);
   }

   PACKET_delete(p);
   return 0;
}


/**
 * Prepares the outputs files names.
 */
void ClusteringBackEnd::CheckOutputFile()
{
   string OutputFileExtension;
   OutputFileExtension = FileNameManipulator::GetExtension(OutputFileName);

   FileNameManipulator NameManipulator(OutputFileName, OutputFileExtension);

   ostringstream ModelExtension;
   ModelExtension.str("");
   ModelExtension << "LOCAL_MODEL_" << WhoAmI();

   LocalModelDataFileName = NameManipulator.AppendStringAndExtension (ModelExtension.str(), "csv");
   LocalModelPlotFileName = NameManipulator.AppendStringAndExtension (ModelExtension.str(), "gnuplot");

   ModelExtension.str("");
   ModelExtension << "LOCAL_CLUSTERING_" << WhoAmI();

   OutputLocalClusteringFileName = NameManipulator.AppendStringAndExtension (ModelExtension.str(), "csv");

   GlobalModelDataFileName     = NameManipulator.AppendStringAndExtension ("JOINT_MODEL", "csv");
   GlobalModelPlotFileName     = NameManipulator.AppendStringAndExtension ("JOINT_MODEL", "gnuplot");
   OutputDataFileName          = NameManipulator.AppendStringAndExtension ("CLUSTERED", "csv");
   ClustersInformationFileName = NameManipulator.AppendStringAndExtension ("clusters_info", "csv");
}

