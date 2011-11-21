#include <iostream>
#include <vector>
#include <mrnet/Packet.h>
#include <mrnet/NetworkTopology.h>
#include <ConvexHullModel.hpp>
#include "Clustering_Filter.h"
#include "NoiseManager.h"
#include "tags.h"
#include "utils.h"

using namespace MRN;
using namespace std;

extern "C" {

const char *filterClustering_format_string = "";

int  WaitForNoise = 0;
int  WaitForHulls = 0;
bool NeedsReset   = true;

vector<const Point *>   NoisePoints;
vector<ConvexHullModel> ClustersHulls;


/**
 * Initializes the filter.
 * @param top_info The TopologyLocalInfo object received by the filter.
 */
void Init(const TopologyLocalInfo & top_info)
{
   WaitForNoise = WaitForHulls = top_info.get_NumChildren();

   NoisePoints.clear();
   ClustersHulls.clear();

   NeedsReset = false;
}


/**
 * The clustering filter receives the noise points from all children,
 * clusters them and builds the model for the resulting new hulls, and
 * merges them with the hulls received from the children. The remaining
 * noise points and the merged hulls are sent to the next level of the tree.
 */
void filterClustering( std::vector< PacketPtr >& packets_in,
                       std::vector< PacketPtr >& packets_out,
                       std::vector< PacketPtr >& /* packets_out_reverse */,
                       void ** /* client data */,
                       PacketPtr& params,
                       const TopologyLocalInfo& top_info)
{
   int    tag = packets_in[0]->get_Tag();
   double Epsilon   = 0.0;
   int    MinPoints = 0;

   /* DEBUG - Bypass all messages
   for (unsigned int i=0; i<packets_in.size(); i++)
   {
      packets_out.push_back(packets_in[i]);
   }
   return; */

   /* Initialize the filter */
   if (NeedsReset)
   {
      Init(top_info);
   }

   /* Bypass the filter in the back-ends, there's nothing to merge at this level! */
   if (BOTTOM_FILTER(top_info))
   {
      for (unsigned int i=0; i<packets_in.size(); i++)
      {
         packets_out.push_back(packets_in[i]);
      }
      return;
   }

   /* Get filter parameters */
   params->unpack("%lf %d", &Epsilon, &MinPoints);

   /* Process the packet crossing the filter */
   switch(tag)
   {
#if defined(PROCESS_NOISE)
      case TAG_NOISE:
      {
         /* Accumulate all children noise points in vector NoisePoints */
         NoiseManager Noise = NoiseManager();
         Noise.Unpack( packets_in[0], NoisePoints );
         break;
      }
      case TAG_ALL_NOISE_SENT:
      {
         WaitForNoise --;
         /* Check whether all children sent their noise points */
         if (WaitForNoise <= 0)
         {
            NoiseManager Noise = NoiseManager(Epsilon, MinPoints);
            /* DEBUG -- Number of noise points
            cerr << "[DEBUG FILTER " << FILTER_ID(top_info) << "] NoisePoints.size()=" << NoisePoints.size() << endl; */

            /* Cluster all children noise points */
            vector<ConvexHullModel> NoiseModel;
            Noise.ClusterNoise( NoisePoints, NoiseModel );
            /* Send remaining noise points to the next tree level */
            Noise.Serialize(packets_in[0]->get_StreamId(), packets_out);
            /* Store the new noise hulls in the global array ClustersHulls */
            for (unsigned int i=0; i<NoiseModel.size(); i++)
            {
               ClustersHulls.push_back( NoiseModel[i] );
            }
         }
         break;
      }
#endif
      case TAG_HULLS:
      {
         /* Receive hull from one child */
         long long Density;
         int NumPoints, NumDimensions, DimValuesSize;
         long long *Instances, *NeighbourhoodSizes;
         double * DimValues;

         packets_in[0]->unpack("%ld %d %d %ald %ald %alf", &Density, &NumPoints, &NumDimensions, &Instances, &NumPoints, &NeighbourhoodSizes, &NumPoints, &DimValues, &DimValuesSize);


         /* Reconstruct the hull and store it in ClustersHulls vector */
         ConvexHullModel Hull(Density, NumPoints, NumDimensions, Instances, NeighbourhoodSizes, DimValues);
         ClustersHulls.push_back( Hull );

         xfree ( Instances );
         xfree ( NeighbourhoodSizes );
         xfree ( DimValues);
         break;
      }
      case TAG_ALL_HULLS_SENT:
      {
         WaitForHulls --;
         /* Check whether all children sent their hulls */
         if (WaitForHulls <= 0)
         {
            vector<ConvexHullModel> MergedModel;
            vector<ConvexHullModel>::iterator it;

            /* Merge all children and noise hulls */
            MergeAlltoAll ( ClustersHulls, MergedModel, Epsilon, MinPoints );

            /* Send the joint hulls */
            for (size_t i = 0; i < MergedModel.size(); i++)
            {
               ConvexHullModel &Hull  = MergedModel[i];
               long long totalDensity = 0;
               int       totalPoints  = 0, dimsCount = 0;
               long long *Instances, *NeighbourhoodSizes;
               double    *dimsValues   = NULL;

               /* if (Hull->GetDensity() >= WeightedMinPoints) // Now using fixed MinPoints at all levels */
               {
                  Hull.Serialize (totalDensity, totalPoints, dimsCount, Instances, NeighbourhoodSizes, dimsValues);

                  /* DEBUG -- Dump this hull
                  cerr << "*** HULL " << i << " *** << endl;
                  Hull->Print(); */

                  PacketPtr new_packet( new Packet( packets_in[0]->get_StreamId(), TAG_HULLS, "%ld %d %d %ald %ald %alf",
                                                    totalDensity,
                                                    totalPoints,
                                                    dimsCount,
                                                    Instances, totalPoints,
                                                    NeighbourhoodSizes, totalPoints,
                                                    dimsValues, totalPoints*dimsCount) );

                  new_packet->set_DestroyData(true);

                  packets_out.push_back( new_packet );
               }
            }

            /* Notify all hulls were sent */
            PacketPtr new_packet( new Packet( packets_in[0]->get_StreamId(), TAG_ALL_HULLS_SENT, "") );
            packets_out.push_back( new_packet );

            /* Reset the filter next time it triggers */
            NeedsReset = true;
         }
         break;
      }
      default:
      {
         cerr << "[FILTER " << FILTER_ID(top_info) << "] WARNING: Unknown message tag '" << tag << "'" << endl;
         break;
      }
   }
}


/**
 * Takes all hulls in the input array ClustersHulls and checks whether they intersect
 * with each other. When two hulls intersect, a new hull is build that embraces both of them,
 * otherwise they're kept separate.
 * @param ClustersHulls Input array of hulls to merge. It's modified inside this function!!!
 * @param MergedModel Output array of merged hulls.
 * @param Epsilon Determines the minimum distance to consider whether two hulls intersect.
 * @param MinPoints This parameter is not really used at the moment, but it used to limit the
 *                  minimum density of the merged hull.
 */
void MergeAlltoAll(vector<ConvexHullModel> &ClustersHulls,
                   vector<ConvexHullModel> &MergedModel,
                   double Epsilon, int MinPoints)
{
   vector<ConvexHullModel>::iterator it;
   int idx = 0, idx2 = 0;
   vector<bool> TakeThis (ClustersHulls.size(), true);

   /* DEBUG -- Show the total number of hulls (from children + noise) we're going to merge
   cerr << "[DEBUG] MergeAlltoAll: ClustersHulls.size()=" << ClustersHulls.size() << endl;

   for (it = ClustersHulls.begin(); it != ClustersHulls.end(); ++ it)
   {
      TakeThis.push_back(true);
   }
   */

   for (idx = 0; idx < ClustersHulls.size(); ++ idx)
   {


      if (!TakeThis[idx]) continue;

      for (idx2 = idx+1; idx2 < ClustersHulls.size(); ++ idx2)
      {
         ConvexHullModel *newHull;

         /* DEBUG
         cerr << "Checking Hull #" << idx << " vs. #" << idx2 << endl; */


         if (!TakeThis[idx2])
         {
           /* DEBUG
           cerr << "#" << idx2 << " already USED!" << endl; */
           continue;
         }

         if ((newHull = ClustersHulls[idx].Merge(&ClustersHulls[idx2],
                                                  Epsilon,
                                                  MinPoints)) != NULL)
         {
            /* Hulls idx and idx2 intersect */

            TakeThis[idx]  = false;
            TakeThis[idx2] = false;

            ClustersHulls.push_back(*newHull);
            TakeThis.push_back(true);

            /* DEBUG
            cerr << "[DEBUG FILTER] MergeAlltoAll: Hulls " << idx << " and " << idx2 << " intersect. New Hull #" << TakeThis.size() << " inserted" << endl; */

            delete newHull;
            break;
         }
         else
         {
            /* These don't intersect */
            /* DEBUG
            cerr << "[DEBUG FILTER] MergeAlltoAll: Hulls " << idx << " and " << idx2 << " DO NOT intersect" << endl; */
         }
      }
   }

   for (idx = 0; idx < ClustersHulls.size(); idx ++)
   {
      if (TakeThis[idx])
      {
        MergedModel.push_back( ClustersHulls[idx] );
      }
   }
   ClustersHulls.clear(); /* It is also cleared in Init, but just to make
                             sure we don't use it after this function, as
                             it's been reused to store the intermediate merged hulls */
}


} /* extern "C" */

