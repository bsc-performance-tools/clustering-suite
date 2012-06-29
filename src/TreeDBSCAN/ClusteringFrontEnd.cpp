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
#include "ClusteringFrontEnd.h"
#include "NoiseManager.h"
#include "HullManager.h"
#include "ClusteringTags.h"
#include "Utils.h"

/**
 * Constructor sets the clustering configuration parameters.
 */
ClusteringFrontEnd::ClusteringFrontEnd(
   double Eps,
   int    MinPts,
   string ClusteringDefinitionXML,
   string InputTraceName,
   string OutputFileName,
   bool   Verbose,
   bool   ReconstructTrace)
{
   this->Epsilon                 = Eps;
   this->MinPoints               = MinPts;
   this->ClusteringDefinitionXML = ClusteringDefinitionXML;
   this->InputTraceName          = InputTraceName;
   this->OutputFileName          = OutputFileName;
   this->Verbose                 = Verbose;
   this->ReconstructTrace        = ReconstructTrace;
}


/**
 * Register the streams and load the filters used in the TreeDBSCAN protocol.
 */
void ClusteringFrontEnd::Setup()
{
   stClustering = Register_Stream("TreeDBSCAN", SFILTER_DONTWAIT);
   stClustering->set_FilterParameters(FILTER_UPSTREAM_TRANS, "%lf %d", Epsilon, MinPoints);
   stXchangeDims = Register_Stream("XchangeDimensions", SFILTER_WAITFORALL);
}


/**
 * Front-end side of the TreeDBSCAN clustering algorithm.
 */
int ClusteringFrontEnd::Run()
{
   int       countGlobalHulls = 0;
   int       tag;
   PacketPtr p;

   cout << "[FE] Sending clustering configuration:"                       << endl;
   cout << "[FE] + Epsilon     = " << Epsilon                             << endl;
   cout << "[FE] + Min Points  = " << MinPoints                           << endl;
   cout << "[FE] + XML         = " << ClusteringDefinitionXML             << endl;
   
   if (InputTraceName != "")
   {
      cout << "[FE] + Input       = " << InputTraceName                      << endl;
   }
   cout << "[FE] + Output      = " << OutputFileName                      << endl;
   cout << "[FE] + Verbose     = " << ( Verbose          ? "yes" : "no" ) << endl;
   cout << "[FE] + Reconstruct = " << ( ReconstructTrace ? "yes" : "no" ) << endl;
   cout << endl;

   /* Send the clustering configuration to the back-ends */
   Send_Configuration();

   cout << "[FE] Computing global dimensions..." << endl;
   /* Receive and broadcast back the global dimensions */
   MRN_STREAM_RECV(stXchangeDims, &tag, p, TAG_XCHANGE_DIMENSIONS);
   stXchangeDims->send(p);

   cout << "[FE] Computing global hulls..." << endl;
   do
   {
      MRN_STREAM_RECV(stClustering, &tag, p, TAG_ANY);
#if 0
      /* Receive the resulting global hulls one by one */
      if (tag == TAG_HULL)
      {
         HullModel *GlobalHull = NULL;
         HullManager HM        = HullManager();

         GlobalHull = HM.Unpack(p);
         GlobalModel.push_back( GlobalHull );

         /* Broadcast back the global hull */
         stClustering->send(p);
         countGlobalHulls ++;

         /* DEBUG
         std::cout << "********** [FE] BROADCASTING HULL " << countGlobalHulls << std::endl;
         std::cout << "Current Hull size = " << GlobalHull->Size() << " density = " << GlobalHull->Density() << std::endl;
         GlobalHull->Flush();
         std::cout << "********** [FE] END BROADCASTING HULL " << countGlobalHulls << std::endl; */
      }
#endif
      /* Receive 1 packet with all resulting global hulls at once */
      if (tag == TAG_ALL_HULLS)
      {
         HullManager HM = HullManager();
         HM.Unpack(p, GlobalModel);
         vector<HullModel *> FilteredGlobalModel;
         for (unsigned int i = 0; i<GlobalModel.size(); i++)
         {
            if (GlobalModel[i]->Density() >= MinPoints)
            {
               FilteredGlobalModel.push_back( GlobalModel[i] );
            }
         }
         /* Broadcast back the global model */
         HM.SerializeAll(stClustering, FilteredGlobalModel);
         countGlobalHulls = FilteredGlobalModel.size();

      }

#if defined(PROCESS_NOISE)
      /* Count the remaining noise points */
      else if (tag == TAG_NOISE)
      {
         vector<const Point *> NoisePoints;
         NoiseManager Noise = NoiseManager();
         Noise.Unpack(p, NoisePoints);
         cout << "[FE] Remaining noise points = " << NoisePoints.size() << endl;
      }
#endif
   } while (tag != TAG_ALL_HULLS_SENT);

   stClustering->send(TAG_ALL_HULLS_SENT, "");
   cout << "[FE] Broadcasted " << countGlobalHulls << " global hulls!" << endl;

   /* Receive the statistics from all nodes */
   Statistics ClusteringStats(WhoAmI());
   MRN_STREAM_RECV(stClustering, &tag, p, TAG_STATISTICS);
   ClusteringStats.Unpack(p);
   PrintGraphStats(ClusteringStats); 

   return countGlobalHulls;
}

void ClusteringFrontEnd::PrintGraphStats(Statistics &ClusteringStats)
{
  string TreeLayoutFileName = "MRNetStats.layout";
  GetNetwork()->get_NetworkTopology()->print_DOTGraph( TreeLayoutFileName.c_str() );

  string StatsFileName = "MRNetStats.data";
  ofstream StatsFile;
  StatsFile.open (StatsFileName.c_str());
  ClusteringStats.DumpAllStats(StatsFile);
  StatsFile.close();

  string OutputDOTName = "MRNetStats.dot";
  string CMD = string(getenv("TREE_DBSCAN_HOME")) + "/bin/draw_stats " + TreeLayoutFileName + " " + StatsFileName + " " + OutputDOTName;

  cout << "[FE] Generating debug statistics... ";
  system(CMD.c_str());
  cout << "done!" << endl;

  unlink(TreeLayoutFileName.c_str());
  unlink(StatsFileName.c_str());
}

