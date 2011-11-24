#include <iostream>
#include <vector>
#include <stdlib.h>
#include "NoiseManager.h"
#include "ClusteringTags.h"
#include "Utils.h"

using std::vector;
using std::cerr;
using std::cout;
using std::endl;


#if defined(FRONTEND) || defined(FILTER)
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
   libClustering = new libDistributedClustering(false); /* true = Verbose */

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
   int          DimensionsCount   = 0;
   double      *SerialPoints      = NULL;
   unsigned int SerialPointsCount = 0;

   Serialize(DimensionsCount, SerialPoints, SerialPointsCount);

   PacketPtr new_packet1( new Packet( StreamID, TAG_NOISE, "%d %alf", DimensionsCount, SerialPoints, SerialPointsCount) );
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
 * @return true on success; false otherwise.
 */
bool NoiseManager::ClusterNoise(vector<const Point*>& Points, vector<HullModel*>& NoiseModel)
{
   return libClustering->ClusterAnalysis(Points, NoiseModel);
}


/**
 * Unpacks an array of noise points from a MRNet packet and stores them
 * into the specified vector.
 * @param in_packet MRNet packet to extract the points from.
 * @param NoisePoints Array where the points are stored.
 * @return the number of noise points unpacked.
 */
int NoiseManager::Unpack(PACKET_PTR in_packet, vector<const Point *> &NoisePoints)
{
   int     dimsCount  = 0, dimsValuesCount = 0, countPoints = 0;
   double *dimsValues = NULL;

   if (in_packet == NULL) return 0;

   PACKET_unpack(in_packet, "%d %alf", &dimsCount, &dimsValues, &dimsValuesCount);

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
   }
   xfree( dimsValues );
   return countPoints;
}

#endif /* FRONTEND || FILTER */


#if defined(BACKEND)
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
   int          DimensionsCount   = 0;
   double      *SerialPoints      = NULL;
   unsigned int SerialPointsCount = 0;

   Serialize(DimensionsCount, SerialPoints, SerialPointsCount);

   STREAM_send(OutputStream, TAG_NOISE, "%d %alf", DimensionsCount, SerialPoints, SerialPointsCount);
   xfree(SerialPoints);
   STREAM_send(OutputStream, TAG_ALL_NOISE_SENT, "");
}

#endif /* BACKEND */


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
void NoiseManager::Serialize(int &DimensionsCount, double *&SerialPoints, unsigned int &SerialPointsCount)
{
   vector<const Point*> NoisePoints;

   DimensionsCount   = 0;
   SerialPoints      = NULL;
   SerialPointsCount = 0;

   /* Retrieve the remaining noise points */
   if (!libClustering->GetNoisePoints(NoisePoints))
   {
      cerr << "ERROR: NoiseManager::Serialize: Error retrieving noise points: " << libClustering->GetErrorMessage() << endl;
      NoisePoints.clear();
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

   unsigned int NoisePointsCount = NoisePoints.size();
   if (NoisePointsCount > 0)
   {
      DimensionsCount   = NoisePoints[0]->size();
      SerialPointsCount = DimensionsCount * NoisePointsCount;
      SerialPoints      = (double *)malloc(SerialPointsCount * sizeof(double));

      if (SerialPoints == NULL)
      {
         cerr << "ERROR: NoiseManager::Serialize: Not enough memory to serialize " << NoisePointsCount << " noise points" << endl;
         exit(EXIT_FAILURE);
      }

      /* Store the points dimensions in a linear array */
      for (unsigned int i=0; i<NoisePointsCount; i++)
      {
         for (unsigned int j=0; j<DimensionsCount; j++)
         {
            SerialPoints[ (i*DimensionsCount)+j ] = (*(NoisePoints[i]))[j];
         }
      }
   }
}

