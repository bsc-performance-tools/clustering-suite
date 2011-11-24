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
    HullModel* Unpack(PACKET_PTR InputPacket);

    /* Back-end API */
    void Serialize(STREAM *OutputStream, vector<HullModel*> &HullsList);

    /* Filter API */
    void Serialize(int StreamID, vector<PacketPtr> &OutputPackets, vector<HullModel*> &HullsList);

  private:
    /* Back-end API */
    void SerializeOne(STREAM *OutputStream, HullModel *Hull);
    void SerializeDone(STREAM *OutputStream);

    /* Filter API */
    void SerializeOne(int StreamID, vector<PacketPtr> &OutputPackets, HullModel *Hull);
    void SerializeDone(int StreamID, vector<PacketPtr> &OutputPackets);
};

#endif /* __HULL_MANAGER_H__ */

