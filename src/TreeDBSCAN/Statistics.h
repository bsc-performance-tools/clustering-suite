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
using std::ostream;
using std::string;
using std::pair;
using cepba_tools::Timer;

class Statistics
{
  public:
    Statistics(int ID, bool node_is_backend);

    void IncreaseInputHulls   (int num_hulls);
    void IncreaseOutputHulls  (int num_hulls);
    void IncreaseInputPoints  (int num_points);
    void IncreaseOutputPoints (int num_points);
    void IncreaseNumIntersects(bool valid);
    void ExtractionTimerStart();
    void ExtractionTimerStop();
    void ClusteringTimerStart();
    void ClusteringTimerStop();
    void MergeTimerStart();
    void MergeTimerStop();
    void ClassificationTimerStart();
    void ClassificationTimerStop();
    void ReconstructTimerStart();
    void ReconstructTimerStop();
    void TotalTimerStart();
    void TotalTimerStop();

    void to_str(string &NodeStats, string &EdgeStats);
    void Serialize(STREAM *OutputStream);
    void Serialize(int StreamID, vector<PacketPtr> &OutputPackets);
	void Unpack(PACKET_PTR InputPacket);

    void DumpAllStats(ostream &Output);

    void Reset(void);

  private:
    int  CurrentNodeID;
    bool CurrentNodeIsBackend;

    int NumInputHulls;
    int NumOutputHulls;
    int NumInputPoints;
    int NumOutputPoints;
    int NumValidIntersects;
    int NumTotalIntersects;

    double ElapsedTimeExtraction;
    double ElapsedTimeClustering;
    double ElapsedTimeIntersecting;
    double ElapsedTimeClassification;
    double ElapsedTimeReconstruct;
    double ElapsedTimeTotal;

    cepba_tools::Timer ExtractionTimer;
    cepba_tools::Timer ClusteringTimer;
    cepba_tools::Timer MergeTimer;
    cepba_tools::Timer ClassificationTimer;
    cepba_tools::Timer ReconstructTimer;
    cepba_tools::Timer TotalTimer;

    map< int, pair<string, string> > NodeStats;
    
    void Serialize(
      int    &NumberOfNodes,
      int   *&IDsArray,
      char **&NodeStatsArray,
      char **&EdgeStatsArray);
};

#endif /* __STATISTICS_H__ */
