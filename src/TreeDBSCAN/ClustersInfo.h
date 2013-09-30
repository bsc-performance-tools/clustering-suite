
#ifndef __CLUSTERS_INFO_H__
#define __CLUSTERS_INFO_H__

#include <sstream>
#include <vector>
#include <map>
#include <MRNet_wrappers.h>
#include "ClusterStatistics.hpp"

#define DELIMITER_OBJECT ';'

#define NOISE_CLUSTER_ID 0

#define CSV_DELIMITER ","
#define CLUSTER_NAME_LABEL "Cluster Name"
#define NOISE_LABEL "NOISE"
#define CLUSTER_LABEL "Cluster "
#define DENSITY_LABEL "Density"
#define TOTAL_DURATION_LABEL "Total duration"
#define AVG_DURATION_LABEL "Avg. duration"
#define PCT_TOTAL_DURATION_LABEL "% Total duration"

using std::stringstream;
using std::vector;
using std::map;

class ClustersInfo
{
  public:
    ClustersInfo(vector<ClusterStatistics *> &Statistics);
    ClustersInfo();
    void Reset();

    void Unpack(PACKET_PTR InputPacket);

    void Serialize(STREAM *OutputStream);
    void Serialize(int StreamID, vector<PacketPtr> &OutputPackets);

    void Print();

  private:
    stringstream                  Serialization;
    map<int, ClusterStatistics *> CombinedStatistics;

    void CombineStatistics(char *ChildStatistics);
    char * Serialize();

    friend std::ostream& operator <<(std::ostream& out, const ClustersInfo& ci);
};

#endif /* __CLUSTERS_INFO_H__ */
