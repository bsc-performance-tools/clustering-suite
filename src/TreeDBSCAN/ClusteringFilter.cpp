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

#include <iostream>
#include <vector>
#include <mrnet/Packet.h>
#include <mrnet/NetworkTopology.h>
#include <HullModel.hpp>
#include "ClusteringFilter.h"
#include "NoiseManager.h"
#include "HullManager.h"
#include "ClusteringTags.h"
#include "Utils.h"

using namespace MRN;
using namespace std;

extern "C" {

const char *filterTreeDBSCAN_format_string = "";

int  WaitForNoise = 0;
int  WaitForHulls = 0;
bool NeedsReset   = true;

vector<const Point *> NoisePoints;
vector<HullModel*>    ClustersHulls;


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
void filterTreeDBSCAN( std::vector< PacketPtr >& packets_in,
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
   unsigned int NumSiblings = top_info.get_NumSiblings() + 1;
   int WeightedMinPoints = MinPoints / NumSiblings;
   if (WeightedMinPoints < 3) WeightedMinPoints = 3;

   /* DEBUG
   cerr << "[FILTER " << FILTER_ID(top_info) << "] NumSiblings=" << NumSiblings << " WeightedMinPoints=" << WeightedMinPoints << endl; */

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
            NoiseManager Noise = NoiseManager(Epsilon, WeightedMinPoints);
            /* DEBUG -- Number of noise points
            cerr << "[DEBUG FILTER " << FILTER_ID(top_info) << "] NoisePoints.size()=" << NoisePoints.size() << endl; */

            /* Cluster all children noise points */
            vector<HullModel*> NoiseModel;
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
      case TAG_HULL:
      {
         /* Receive hull from one child */
         HullModel *Hull = NULL;
         HullManager HM  = HullManager();

         Hull = HM.Unpack(packets_in[0]);
         ClustersHulls.push_back(Hull);

         break;
      }
      case TAG_ALL_HULLS_SENT:
      {
         WaitForHulls --;
         /* Check whether all children sent their hulls */
         if (WaitForHulls <= 0)
         {
            vector<HullModel*> MergedModel;

            /* Merge all children and noise hulls */
            MergeAlltoAll ( ClustersHulls, MergedModel, Epsilon, WeightedMinPoints );

            /* Send the joint hulls */
            HullManager HM = HullManager();
            HM.Serialize(packets_in[0]->get_StreamId(), packets_out, MergedModel);

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
void MergeAlltoAll(vector<HullModel*> &ClustersHulls,
                   vector<HullModel*> &MergedModel,
                   double              Epsilon,
                   int                 MinPoints)
{
  int idx = 0, idx2 = 0;

  vector<bool> TakeThis (ClustersHulls.size(), true);

  /* DEBUG -- Show the total number of hulls (from children + noise) we're going to merge
  cerr << "[DEBUG FILTER] MergeAlltoAll: ClustersHulls.size()=" << ClustersHulls.size() << endl; */

   for (idx = 0; idx < ClustersHulls.size(); ++ idx)
   {
      if (!TakeThis[idx]) continue;

      for (idx2 = idx+1; idx2 < ClustersHulls.size(); ++ idx2)
      {
         HullModel *newHull;

         if (!TakeThis[idx2]) continue;

         /* DEBUG
         cout << "*** HULL IDX " << idx << " (density=" << ClustersHulls[idx]->Density() << ")" << endl;
         ClustersHulls[idx]->Flush();
         cout << "*** HULL IDX " << idx2 << " (density=" << ClustersHulls[idx2]->Density() << ")" << endl;
         ClustersHulls[idx2]->Flush(); 
         cout << "[DEBUG FILTER ] Trying to merge hulls " << idx << " (size=" << ClustersHulls[idx]->Size() << ") and " << idx2 << " (size=" << ClustersHulls[idx2]->Size() << "). Intersect? "; */

         if ((newHull = ClustersHulls[idx]->Merge(ClustersHulls[idx2], Epsilon, MinPoints)) != NULL)
         {
            /* Hulls idx and idx2 intersect 
            cout << "YES (new_size=" << newHull->Size() << ")" << endl; */

            TakeThis[idx]  = false;
            TakeThis[idx2] = false;

            ClustersHulls.push_back(newHull);
            TakeThis.push_back(true);

            break;
         }
         else
         {
            /* DEBUG 
            cout << "NO" << endl; */
            /* These don't intersect */
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

