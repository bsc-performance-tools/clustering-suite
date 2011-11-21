#ifndef __NOISE_MANAGER_H__
#define __NOISE_MANAGER_H__

#include "MRNet_wrappers.h"
#include "libDistributedClustering.hpp"

#define PROCESS_NOISE

using MRN::Stream;
using MRN::Packet;
using MRN::PacketPtr;

class NoiseManager
{
   public:
      /* Front-end & Filter interface */
#if defined(FRONTEND) || defined(FILTER)
      NoiseManager();
      NoiseManager(double Epsilon, int MinPoints);

      bool ClusterNoise(vector<const Point*>& Points, vector<ConvexHullModel>& NoiseModel);
      void Serialize(int StreamID, std::vector< PacketPtr >& OutputPackets);
      int  Unpack(PACKET_PTR in_packet, vector<const Point *> &NoisePoints);
#endif /* FRONTEND || FILTER */

      /* Back-end interface */
#if defined(BACKEND)
      NoiseManager(libDistributedClustering *libClustering);

      void Serialize(Stream *OutputStream);
#endif /* BACKEND */

   private:
      libDistributedClustering *libClustering;

      void Serialize(int &DimensionsCount, double *&SerialPoints, unsigned int &SerialPointsCount);
};

#endif /* __NOISE_MANAGER_H__ */
