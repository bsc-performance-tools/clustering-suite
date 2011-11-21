#include <iostream>
#include "ClusteringOffline_FE.h"
#include "NoiseManager.h"
#include "tags.h"
#include "utils.h"

Clustering::Clustering(
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

void Clustering::set_Epsilon(double Eps)
{
   Epsilon = Eps;
}

void Clustering::set_MinPoints(int MinPts)
{
   MinPoints = MinPts;
}

void Clustering::Setup()
{
   stClustering = Register_Stream("Clustering", SFILTER_DONTWAIT);
   stClustering->set_FilterParameters(FILTER_UPSTREAM_TRANS, "%lf %d", Epsilon, MinPoints);
}

int Clustering::Run()
{
   int       countGlobalHulls = 0;
   int       tag;
   PacketPtr p;

   cout << "[FE] Sending clustering configuration:"                       << endl;
   cout << "[FE] + Epsilon     = " << Epsilon                             << endl;
   cout << "[FE] + Min Points  = " << MinPoints                           << endl;
   cout << "[FE] + XML         = " << ClusteringDefinitionXML             << endl;
   cout << "[FE] + Input       = " << InputTraceName                      << endl;
   cout << "[FE] + Output      = " << OutputFileName                      << endl;
   cout << "[FE] + Verbose     = " << ( Verbose          ? "yes" : "no" ) << endl;
   cout << "[FE] + Reconstruct = " << ( ReconstructTrace ? "yes" : "no" ) << endl;
   cout << endl;

   stClustering->send(TAG_CLUSTERING_CONFIG, "%lf %d %s %s %s %d %d",
      Epsilon,
      MinPoints,
      ClusteringDefinitionXML.c_str(),
      InputTraceName.c_str(),
      OutputFileName.c_str(),
      ((int)Verbose),
      ((int)ReconstructTrace));

   do
   {
      /* Receive the resulting global hulls */
      stClustering->recv(&tag, p);
      if (tag == TAG_HULLS)
      {
         long long Density;
         int NumPoints, NumDimensions, DimValuesSize;
         long long *Instances, *NeighbourhoodSizes;
         double * DimValues;

         p->unpack("%ld %d %d %ald %ald %alf", &Density, &NumPoints, &NumDimensions, &Instances, &NumPoints, &NeighbourhoodSizes, &NumPoints, &DimValues, &DimValuesSize);

         /* Broadcast the global model */
         /* if (Density >= MinPoints) //Now this is sure to be true */
         {
            //stClustering->send(TAG_HULLS, "%ld %d %d %alf", Density, NumPoints, NumDimensions, DimValues, DimValuesSize);
            stClustering->send(p);
            countGlobalHulls ++;
         }

         /* DEBUG
         std::cout << "********** [FE] BROADCASTING HULL " << countGlobalHulls << std::endl;
         CHull.Print();
         std::cout << "********** [FE] END BROADCASTING HULL " << countGlobalHulls << std::endl;
         */

         ConvexHullModel GlobalHull(Density, NumPoints, NumDimensions, Instances, NeighbourhoodSizes, DimValues);
         GlobalModel.push_back( GlobalHull );

         xfree(Instances);
         xfree(NeighbourhoodSizes);
         xfree(DimValues);
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

   return 0;
}

