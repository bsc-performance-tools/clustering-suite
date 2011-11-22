#ifndef __HULL_MANAGER_H__
#define __HULL_MANAGER_H__

#include <vector>
#include "libDistributedClustering.hpp"
#include "MRNet_wrappers.h"

using namespace std;

/**
 * This class manages the serialization of convex hulls through the MRNet.
 */
class HullManager 
{
  public:
    HullManager();

    /* Common API */
    ConvexHullModel * Unpack(PACKET_PTR InputPacket);

    /* Back-end API */
    void Serialize(STREAM *OutputStream, vector<ConvexHullModel> &HullsList);

    /* Filter API */
    void Serialize(int StreamID, vector<PacketPtr> &OutputPackets, vector<ConvexHullModel> &HullsList);

  private:
    /* Back-end API */
    void SerializeOne(STREAM *OutputStream, ConvexHullModel &Hull);
    void SerializeDone(STREAM *OutputStream);

    /* Filter API */
    void SerializeOne(int StreamID, vector<PacketPtr> &OutputPackets, ConvexHullModel &Hull);
    void SerializeDone(int StreamID, vector<PacketPtr> &OutputPackets);
};

#endif /* __HULL_MANAGER_H__ */

