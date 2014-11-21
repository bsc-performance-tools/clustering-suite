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
using std::cerr;
using std::endl;

#include <sstream>
using std::ostringstream;

#include <SystemMessages.hpp>
using cepba_tools::system_messages;

#include <Timer.hpp>
#include <FileNameManipulator.hpp>
using cepba_tools::FileNameManipulator;

#include "TDBSCANWorker.h"
#include "NoiseManager.h"
#include "HullManager.h"
#include "TDBSCANTags.h"
#include "Utils.h"
#include "Statistics.h"
#include "Support.h"


/**
 * TDBSCAN back-end protocol constructor.
 */
TDBSCANWorker::TDBSCANWorker()
{
  libClustering = NULL;
}


/**
 * Register the streams used in this protocol.
 */
void TDBSCANWorker::Setup()
{
  Register_Stream (stClustering);
  Register_Stream (stXchangeDims);
  Register_Stream (stSupport);
}


/**
 * Implements the back-end side of the TDBSCAN clustering algorithm.
 * @return 0 on success; -1 otherwise.
 */
int TDBSCANWorker::Run()
{
  ostringstream Messages;

  int tag;
  PACKET_new (p);
  vector<HullModel*>::iterator it;
  cepba_tools::Timer t;
  Statistics ClusteringStats (WhoAmI(true), true);

  /* Delete any previous clustering */
  if (libClustering != NULL)
  {
    delete libClustering;
  }

  libClustering = new libDistributedClustering (Verbose, "BE");

  /* Receive clustering configuration from the front-end */
  Recv_Configuration();

  /* Prepare all outputs file names */
  CheckOutputFile();

  if (!InitLibrary() )
  {
    Messages << "Error initializing clustering. Exiting..." << endl;
    system_messages::information (Messages.str(), stderr);
    exit (EXIT_FAILURE);
  }

  t.begin();

  if (!ExtractData() )
  {
    exit (EXIT_FAILURE);
  }

  system_messages::show_timer ("Data extraction time", t.end() );

  ClusteringStats.IncreaseInputPoints ( libClustering->GetNumberOfPoints() );

  Messages.str ("");
  Messages << "Bursts to analyze: " << libClustering->GetNumberOfPoints() << endl;
  system_messages::information (Messages.str() );

  /* Start the clustering analysis */
  t.begin();
  ClusteringStats.ClusteringTimeStart();

  if (!AnalyzeData() )
  {
    exit (EXIT_FAILURE);
  }

  ClusteringStats.ClusteringTimeStop();

#if defined(PROCESS_NOISE)
  vector<const Point *> NoisePoints;
  vector<long long>     NoiseDurations;
  libClustering->GetNoisePoints (NoisePoints, NoiseDurations );
  ClusteringStats.IncreaseOutputPoints ( NoisePoints.size() );

  /* DEBUG -- count remaining noise points
  if (Verbose) cerr << "[BE " << WhoAmI() << "] Number of noise points = " << NoisePoints.size() << endl; */

  NoiseManager Noise = NoiseManager (libClustering);
  Noise.Serialize (stClustering);
#endif

  /* Send the local hulls */
  Messages.str ("");
  Messages << "Sending " << LocalModel.size() << " local hulls" << endl;
  system_messages::information (Messages.str() );
  ClusteringStats.IncreaseOutputHulls ( LocalModel.size() );

  HullManager HM = HullManager();
//   HM.Serialize(stClustering, LocalModel);
  HM.SerializeAll (stClustering, LocalModel);

#if 0

  /* Receive the resulting global hulls one by one */
  do
  {
    STREAM_recv (stClustering, &tag, p, TAG_HULL);

    if (tag == TAG_HULL)
    {
      HullModel *Hull = NULL;
      HullManager HM  = HullManager();
      Hull            = HM.Unpack (p);

      /* DEBUG
      std::cout << "[BE " << WhoAmI() << "] Received global Hull (size = " << Hull->Size() << " density = " << Hull->Density() << ")" << std::endl;
      Hull->Flush(); */

      GlobalModel.push_back (Hull);
    }
  }
  while (tag != TAG_ALL_HULLS_SENT);

#endif

  /* Receive 1 packet with all the resulting global hulls */
  do
  {
    STREAM_recv (stClustering, &tag, p, TAG_ANY);

    if (tag == TAG_ALL_HULLS)
    {
      HullManager HM  = HullManager();
      HM.Unpack (p, GlobalModel);
    }
  }
  while (tag != TAG_ALL_HULLS_SENT);

  /* Once the clustering is over, send the statistics to the root */
  ClusteringStats.Serialize (stClustering);

  Messages.str ("");
  Messages << "Received " << GlobalModel.size() << " global hulls." << endl;
  system_messages::information (Messages.str() );
  // cout << "[BE " << WhoAmI() << "] >> Clustering time: " << t.end() << "[" << NumBackEnds() << " BEs]" << endl;

  /* All back-ends classify their local data */
  Messages.str ("");
  Messages << "START CLASSIFYING WITH " << GlobalModel.size() << " GLOBAL HULLS." << endl;
  system_messages::information (Messages.str() );

  // t.begin();
  if (!libClustering->ClassifyData (GlobalModel) )
  {
    Messages.str ("");
    Messages << "Error classifying data: " << libClustering->GetErrorMessage() << endl;
    system_messages::information (Messages.str(), stderr);
    exit (EXIT_FAILURE);
  }

  system_messages::show_timer ("Clustering time", t.end() );

#if 0
  Support BackendSupport(libClustering, 50);
  BackendSupport.Serialize(stSupport);

  Support GlobalSupport(BackendSupport);
  MRN_STREAM_RECV (stSupport, &tag, p, TAG_SUPPORT);
  GlobalSupport.Unpack(p);
#endif

  /* Process the results and generate the output files */

  /* WARNING: this 'Support' variable should be removed from here
   * when reactivating the support management in the very previous
   * '#if 0' */
  Support GlobalSupport;

  if (!ProcessResults(GlobalSupport) )
  {
    exit (EXIT_FAILURE);
  }

  PACKET_delete (p);
  return 0;
}


/**
 * Prepares the outputs files names.
 */
void TDBSCANWorker::CheckOutputFile()
{
  string OutputPrefix;
  string OutputFileExtension;
  ostringstream ModelExtension;

  if (OutputFileName == "") OutputFileName = "TDBSCAN";

  OutputFileExtension = FileNameManipulator::GetExtension (OutputFileName);
  FileNameManipulator NameManipulator (OutputFileName, OutputFileExtension);
  OutputPrefix = NameManipulator.GetChoppedFileName();

  /* Names for the global model data and plot */
  GlobalModelDataFileName       = NameManipulator.AppendStringAndExtension ("GLOBAL_MODEL", "csv");
  GlobalModelPlotFileNamePrefix = OutputPrefix + ".GLOBAL_MODEL";

  /* Names for the local models data and plots */
  ModelExtension.str ("");
  ModelExtension << "LOCAL_MODEL_" << WhoAmI();
  LocalModelDataFileName       = NameManipulator.AppendStringAndExtension (ModelExtension.str(), "csv");
  LocalModelPlotFileNamePrefix = OutputPrefix + "." + ModelExtension.str();

  /* Names for the local clustering on each back-end */
  ModelExtension.str ("");
  ModelExtension << "LOCAL_CLUSTERING_" << WhoAmI();
  OutputLocalClusteringFileName = NameManipulator.AppendStringAndExtension (ModelExtension.str(), "csv");

  /* Names for the global clustering on each back-end (classification using the global model) */
  ModelExtension.str ("");
  ModelExtension << "GLOBAL_CLUSTERING_" << WhoAmI();
  OutputGlobalClusteringFileName = NameManipulator.AppendStringAndExtension (ModelExtension.str(), "csv");

  /* Names for the final global clustering (all back-ends data merged) */
  FinalClusteringFileName          = NameManipulator.AppendStringAndExtension ("DATA", "csv");
  FinalClustersInformationFileName = NameManipulator.AppendStringAndExtension ("clusters_info", "csv");
}

