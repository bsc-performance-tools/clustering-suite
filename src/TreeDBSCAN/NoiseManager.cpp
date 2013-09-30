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
#include <vector>
#include <stdlib.h>
#include "NoiseManager.h"
#include "TDBSCANTags.h"
#include "Utils.h"

using std::vector;
using std::cerr;
using std::cout;
using std::endl;


/*****************************************************************\
 *                  FRONT-END & FILTER INTERFACE                 *
\*****************************************************************/


/**
 * Front-end constructor. This instance of the manager is only meant
 * to receive the remaining noise points through the Unpack() call.
 */
NoiseManager::NoiseManager()
{
   libClustering = NULL;
}


/**
 * Filter constructor. Initializes a new instance of libDistributedClustering
 * to cluster noise points in the tree.
 * @param Epsilon
 * @param MinPoints
 */
NoiseManager::NoiseManager(double Epsilon, int MinPoints)
{
   libClustering = new libDistributedClustering(false, "NOISE"); /* true = Verbose */

   if (!libClustering->InitClustering(Epsilon, MinPoints))
   {
      cerr << "ERROR: NoiseManager::NoiseManager: Error initializing clustering: " << libClustering->GetErrorMessage() << endl;
      exit (EXIT_FAILURE);
   }
}


/**
 * Filter pushes the remaining noise points into the output packets array.
 * @param StreamID The ID of the stream where the packet will be sent through.
 * @param OutputPackets Filter's array of output packets.
 */
void NoiseManager::Serialize(int StreamID, std::vector< PacketPtr >& OutputPackets)
{
   int          NoiseCount           = 0;
   int          DimensionsCount      = 0;
   double      *SerialPoints         = NULL;
   unsigned int SerialPointsCount    = 0;
   long long   *SerialNoiseDurations = NULL;

   Serialize(NoiseCount, DimensionsCount, SerialPoints, SerialPointsCount, SerialNoiseDurations);

   PacketPtr new_packet1( new Packet( StreamID, TAG_NOISE, "%d %alf %ald", DimensionsCount, SerialPoints, SerialPointsCount, SerialNoiseDurations, NoiseCount) );
   if (SerialPoints != NULL)
   {
      new_packet1->set_DestroyData(true);
   }
   PacketPtr new_packet2( new Packet( StreamID, TAG_ALL_NOISE_SENT, "") );

   OutputPackets.push_back(new_packet1);
   OutputPackets.push_back(new_packet2);
}


/**
 * Called from the MRNet filter to cluster the noise points.
 * @param Points Noise points to cluster.
 * @param NoiseModel The resulting models for the clustered points.
 * @param CountRemainingNoise The number of remaining noise points after clustering.
 * @return true on success; false otherwise.
 */

bool NoiseManager::ClusterNoise(vector<const Point*>& Points, vector<long long>& Durations, vector<HullModel*>& NoiseModel, int &CountRemainingNoise)
{
   int rc = libClustering->ClusterAnalysis(Points, Durations, NoiseModel);

   CountRemainingNoise = 0;
   vector<const Point*> RemainingNoisePoints;
   vector<long long>    RemainingNoiseDurations;
   if (libClustering->GetNoisePoints(RemainingNoisePoints, RemainingNoiseDurations))
   {
      CountRemainingNoise = RemainingNoisePoints.size();
   }
   return rc;
}


/**
 * Unpacks an array of noise points from a MRNet packet and stores them
 * into the specified vector.
 * @param in_packet MRNet packet to extract the points from.
 * @param NoisePoints Array where the points are stored.
 * @return the number of noise points unpacked.
 */
int NoiseManager::Unpack(PACKET_PTR in_packet, vector<const Point *> &NoisePoints, vector<long long> &NoiseDurations)
{
   int        dimsCount      = 0, dimsValuesCount = 0, countPoints = 0, noiseCount = 0;
   double    *dimsValues     = NULL;
   long long *noiseDurations = NULL;

   if (in_packet == NULL) return 0;

   PACKET_unpack(in_packet, "%d %alf %ald", &dimsCount, &dimsValues, &dimsValuesCount, &noiseDurations, &noiseCount);

   /* Build clustering points from the dimensions values */
   for (unsigned int i=0; i<dimsValuesCount; i+=dimsCount)
   {
      countPoints ++;

      vector<double> PointDimensions;
      for (unsigned int j=0; j<dimsCount; j++)
      {
         PointDimensions.push_back( dimsValues[i+j] );
      }
      const Point *childPoint = new Point( PointDimensions );
      /* Accumulate this noise point in a vector */
      NoisePoints.push_back( childPoint );
      NoiseDurations.push_back( noiseDurations[countPoints - 1] );
   }

   xfree( dimsValues );
   xfree( noiseDurations );
   return countPoints;
}


/*****************************************************************\
 *                       BACK-END INTERFACE                      *
\*****************************************************************/


/**
 * Back-end constructor.
 * @param libClustering An instance of libDistributedClustering that performed a clustering analysis.
 */
NoiseManager::NoiseManager(libDistributedClustering *libClustering)
{
   this->libClustering = libClustering;
   if (libClustering == NULL)
   {
      cerr << "ERROR: NoiseManager::NoiseManager: Clustering library was not initialized?" << endl;
      exit (EXIT_FAILURE);
   }
}


/**
 * Back-end sends the remaining noise points through the specified stream.
 * @param OutputStream
 */
void NoiseManager::Serialize(Stream *OutputStream)
{
   int          NoiseCount           = 0;
   int          DimensionsCount      = 0;
   double      *SerialPoints         = NULL;
   unsigned int SerialPointsCount    = 0;
   long long   *SerialNoiseDurations = NULL;

   Serialize(NoiseCount, DimensionsCount, SerialPoints, SerialPointsCount, SerialNoiseDurations);

   STREAM_send(OutputStream, TAG_NOISE, "%d %alf %ald", DimensionsCount, SerialPoints, SerialPointsCount, SerialNoiseDurations, NoiseCount);
   xfree(SerialPoints);
   STREAM_send(OutputStream, TAG_ALL_NOISE_SENT, "");
}


/*****************************************************************\
 *                            PRIVATE                            *
\*****************************************************************/


/**
 * Retrieves the remaining noise points out of the libClustering instance and stores the information into
 * an array that can be sent through the MRNet.
 * @param DimensionsCount Number of clustering dimensions.
 * @param SerialPoints Array of points dimensions, in sequence.
 * @param SerialPointsCount Size of the points dimensions array.
 * @return the above 3 parameters by reference.
 */
void NoiseManager::Serialize(int &NoiseCount, int &DimensionsCount, double *&SerialPoints, unsigned int &SerialPointsCount, long long *&SerialNoiseDurations)
{
   vector<const Point*> NoisePoints;
   vector<long long>    NoiseDurations;

   DimensionsCount   = 0;
   SerialPoints      = NULL;
   SerialPointsCount = 0;

   /* Retrieve the remaining noise points */
   if (!libClustering->GetNoisePoints(NoisePoints, NoiseDurations))
   {
      cerr << "ERROR: NoiseManager::Serialize: Error retrieving noise points: " << libClustering->GetErrorMessage() << endl;
      NoisePoints.clear();
      NoiseDurations.clear();
   }

   /* DEBUG -- Fill with fake points
   vector<double> dim1, dim2;
   dim1.push_back(0.12);
   dim1.push_back(3.45);
   dim2.push_back(6.78);
   dim2.push_back(9.10);
   const Point* pt1 = new Point( dim1 );
   const Point* pt2 = new Point( dim2 );
   NoisePoints.push_back( pt1 );
   NoisePoints.push_back( pt2 ); */

   NoiseCount = NoisePoints.size();
   if (NoiseCount > 0)
   {
      DimensionsCount      = NoisePoints[0]->size();
      SerialPointsCount    = DimensionsCount * NoiseCount;
      SerialPoints         = (double *)malloc(SerialPointsCount * sizeof(double));
      SerialNoiseDurations = (long long *)malloc(NoiseCount * sizeof(long long));

      if (SerialPoints == NULL)
      {
         cerr << "ERROR: NoiseManager::Serialize: Not enough memory to serialize " << NoiseCount << " noise points" << endl;
         exit(EXIT_FAILURE);
      }

      /* Store the points dimensions in a linear array */
      for (unsigned int i=0; i<NoiseCount; i++)
      {
         for (unsigned int j=0; j<DimensionsCount; j++)
         {
            SerialPoints[ (i*DimensionsCount)+j ] = (*(NoisePoints[i]))[j];
         }
         SerialNoiseDurations[i] = NoiseDurations[i];
      }
   }
}

