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
   /* Receive the resulting global hulls */
   do
   {
      MRN_STREAM_RECV(stClustering, &tag, p, TAG_ANY);
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
         GlobalHull->Flush();
         std::cout << "********** [FE] END BROADCASTING HULL " << countGlobalHulls << std::endl; */
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

   return countGlobalHulls;
}

