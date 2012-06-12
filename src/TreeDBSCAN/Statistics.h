#ifndef __STATISTICS_H__
#define __STATISTICS_H__

#include <vector>
#include <set>
#include <map>
#include <iostream>
#include <fstream>
#include "MRNet_wrappers.h"
#include <Timer.hpp>
#include <string>

using std::vector;
using std::set;
using std::map;
using std::ofstream;
using std::string;
using std::pair;
using cepba_tools::Timer;

class Statistics
{
  public:
    Statistics(int ID);

    void IncreaseInputHulls   (int num_hulls);
    void IncreaseOutputHulls  (int num_hulls);
    void IncreaseInputPoints  (int num_points);
    void IncreaseOutputPoints (int num_points);
    void IncreaseNumIntersects(bool valid);
    void IntersectTimeStart();
    void IntersectTimeStop();
    void ClusteringTimeStart();
    void ClusteringTimeStop();

    void to_str(string &NodeStats, string &EdgeStats);
    void Serialize(STREAM *OutputStream);
    void Serialize(int StreamID, vector<PacketPtr> &OutputPackets);
	void Unpack(PACKET_PTR InputPacket);

    void DumpAllStats(ofstream &Output);

    void Reset(void);

  private:
    int CurrentNodeID;

    int NumInputHulls;
    int NumOutputHulls;
    int NumInputPoints;
    int NumOutputPoints;
    int NumValidIntersects;
    int NumTotalIntersects;
    double ElapsedTimeClustering;
    double ElapsedTimeIntersecting;
    cepba_tools::Timer IntersectTimer;
    cepba_tools::Timer ClusteringTimer;

    map< int, pair<string, string> > NodeStats;
    
    void Serialize(
      int    &NumberOfNodes,
      int   *&IDsArray,
      char **&NodeStatsArray,
      char **&EdgeStatsArray);
};

#endif /* __STATISTICS_H__ */
