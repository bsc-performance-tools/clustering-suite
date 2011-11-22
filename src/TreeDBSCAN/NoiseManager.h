#ifndef __NOISE_MANAGER_H__
#define __NOISE_MANAGER_H__

#include "libDistributedClustering.hpp"
#include "MRNet_wrappers.h"

#define PROCESS_NOISE

using MRN::Stream;
using MRN::Packet;
using MRN::PacketPtr;

/** 
 * This class manages the serialization and clustering of the noise data through the MRNet.
 */ 
class NoiseManager
{
   public:
#if defined(FRONTEND) || defined(FILTER)
      /* Front-end/Filter interface */
      NoiseManager();
      NoiseManager(double Epsilon, int MinPoints);

      bool ClusterNoise(vector<const Point*>& Points, vector<ConvexHullModel>& NoiseModel);
      void Serialize(int StreamID, std::vector< PacketPtr >& OutputPackets);
      int  Unpack(PACKET_PTR in_packet, vector<const Point *> &NoisePoints);
#endif /* FRONTEND or FILTER */

#if defined(BACKEND)
      /* Back-end interface */
      NoiseManager(libDistributedClustering *libClustering);

      void Serialize(Stream *OutputStream);
#endif /* BACKEND */

   private:
      libDistributedClustering *libClustering;

      void Serialize(int &DimensionsCount, double *&SerialPoints, unsigned int &SerialPointsCount);
};

#endif /* __NOISE_MANAGER_H__ */
