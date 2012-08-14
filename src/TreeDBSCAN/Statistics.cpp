#include "ClusteringTags.h"
#include "Statistics.h"

#include <iostream>
#include <sstream>
#include <cstring>

using std::ostringstream;


using std::cout;
using std::cerr;
using std::endl;

const char *StatisticsFormatString = "%ad %as %as";

Statistics::Statistics(int ID)
{
  CurrentNodeID = ID;
  Reset();
}

void Statistics::IncreaseInputHulls(int num_hulls)
{
  NumInputHulls += num_hulls;
}

void Statistics::IncreaseOutputHulls(int num_hulls)
{
  NumOutputHulls += num_hulls;
}

void Statistics::IncreaseInputPoints(int num_points)
{
  NumInputPoints += num_points;
}

void Statistics::IncreaseOutputPoints(int num_points)
{
  NumOutputPoints += num_points;
}

void Statistics::IncreaseNumIntersects(bool valid)
{
  if (valid)
  {
    NumValidIntersects ++;
  }
  NumTotalIntersects ++;
}

void Statistics::IntersectTimeStart()
{
  IntersectTimer.begin();
}

void Statistics::IntersectTimeStop()
{
  ElapsedTimeIntersecting += IntersectTimer.end();
}

void Statistics::ClusteringTimeStart()
{
  ClusteringTimer.begin();
}

void Statistics::ClusteringTimeStop()
{
  ElapsedTimeClustering += ClusteringTimer.end();
}

#if defined(BACKEND)
void Statistics::to_str(string &InStats, string &OutStats)
{
  ostringstream Stats;

  InStats = "";
  OutStats = "";

  Stats.str("");
  Stats << "Clustering points=" << NumInputPoints << "\\n"
        << "Clustering time=" << ElapsedTimeClustering;
  InStats = Stats.str();

  Stats.str("");
  Stats << "Hulls=" << NumOutputHulls << "\\n"
        << "Noise=" << NumOutputPoints;
  OutStats = Stats.str();
}
#else
void Statistics::to_str(string &InStats, string &OutStats)
{
  ostringstream Stats;

  InStats = "";
  OutStats = "";

  Stats.str("");
  Stats << "Sum hulls=" << NumInputHulls << "\\n"
        << "Noise points=" << NumInputPoints << "\\n"
        << "Noise time=" << ElapsedTimeClustering << "\\n"
        << "Intersections=" << NumValidIntersects << "/" << NumTotalIntersects << " (" << (NumValidIntersects*100)/NumTotalIntersects << "%)";
    if (NumValidIntersects > 0)
    {
      Stats << "\\nIntersection time=" << ElapsedTimeIntersecting << " (Avg=" << ElapsedTimeIntersecting/NumTotalIntersects << ")";
    }
  InStats = Stats.str();

  Stats.str("");
  Stats << "Hulls=" << NumOutputHulls << "\\n"
        << "Noise=" << NumOutputPoints;
  OutStats = Stats.str();
}
#endif

void Statistics::Serialize(
  int    &NumberOfNodes,
  int   *&IDsArray,
  char **&InStatsArray,
  char **&OutStatsArray)
{
  pair <string, string> CurrentNodeStats;
  to_str(CurrentNodeStats.first, CurrentNodeStats.second);
  NodeStats[CurrentNodeID] = CurrentNodeStats;

  NumberOfNodes = NodeStats.size();
  IDsArray      = (int *)malloc(sizeof(int) * NumberOfNodes);
  InStatsArray  = (char **)malloc(sizeof(char *) * NumberOfNodes);
  OutStatsArray = (char **)malloc(sizeof(char *) * NumberOfNodes);

  int i = 0;
  map< int, pair<string, string> >::iterator it;
  for (it=NodeStats.begin(); it!=NodeStats.end(); ++it)
  {
    int    NodeID   = it->first;
    string InStats  = (it->second).first;
    string OutStats = (it->second).second;

    IDsArray[i]      = NodeID;
    InStatsArray[i]  = (char *)malloc(sizeof(char) * InStats.length()+1);
    strncpy(InStatsArray[i], InStats.c_str(), InStats.length()+1);
    OutStatsArray[i] = (char *)malloc(sizeof(char) * OutStats.length()+1);
    strncpy(OutStatsArray[i], OutStats.c_str(), OutStats.length()+1);

    i ++;
  }

  /* DEBUG
  cerr << "NumberOfNodes=" << NumberOfNodes << endl;
  cerr << "IDsArray = [ ";
  for (i=0; i<NumberOfNodes; i++)
  {
    cerr << IDsArray[i] << " ";
  }
  cerr << "]" << endl;
  cerr << "InStatsArray = [ ";
  for (i=0; i<NumberOfNodes; i++)
  {
    cerr << InStatsArray[i] << " ";
  }
  cerr << "]" << endl;
  cerr << "OutStatsArray = [ ";
  for (i=0; i<NumberOfNodes; i++)
  {
    cerr << OutStatsArray[i] << " ";
  }
  cerr << "]" << endl; */
}

void Statistics::Serialize(STREAM *OutputStream)
{
  int    NumberOfNodes   = 0;
  int   *IDsArray        = NULL;
  char **InStatsArray    = NULL;
  char **OutStatsArray   = NULL;

  Serialize (NumberOfNodes, IDsArray, InStatsArray, OutStatsArray);

  STREAM_send(OutputStream, TAG_STATISTICS, StatisticsFormatString,
    IDsArray,        NumberOfNodes,
    InStatsArray,    NumberOfNodes,
    OutStatsArray,   NumberOfNodes);

  free (IDsArray);
  for (int i=0; i<NumberOfNodes; i++)
  {
    free(InStatsArray[i]);
    free(OutStatsArray[i]);
  }
  free(InStatsArray);
  free(OutStatsArray);
}

void Statistics::Serialize(int StreamID, vector<PacketPtr> &OutputPackets)
{
  int    NumberOfNodes   = 0;
  int   *IDsArray        = NULL;
  char **InStatsArray    = NULL;
  char **OutStatsArray   = NULL;

  Serialize (NumberOfNodes, IDsArray, InStatsArray, OutStatsArray);

  PacketPtr new_packet( new Packet( StreamID, TAG_STATISTICS, StatisticsFormatString,
                                    IDsArray,        NumberOfNodes,
                                    InStatsArray,    NumberOfNodes,
                                    OutStatsArray,   NumberOfNodes ) );

  new_packet->set_DestroyData(true);
  OutputPackets.push_back( new_packet );
}

void Statistics::Unpack(PACKET_PTR InputPacket)
{
  int    NumberOfNodes   = 0;
  int   *IDsArray        = NULL;
  char **InStatsArray    = NULL;
  char **OutStatsArray   = NULL;

  PACKET_unpack(InputPacket, StatisticsFormatString,
    &IDsArray,        &NumberOfNodes,
    &InStatsArray,    &NumberOfNodes,
    &OutStatsArray,   &NumberOfNodes);

  for (int i=0; i<NumberOfNodes; i++)
  {
    NodeStats[ IDsArray[i] ] = make_pair(string(InStatsArray[i]), string(OutStatsArray[i]));
  }

  free (IDsArray);
  for (int i=0; i<NumberOfNodes; i++)
  {
    free(InStatsArray[i]);
    free(OutStatsArray[i]);
  }
  free(InStatsArray);
  free(OutStatsArray);
}

void Statistics::DumpAllStats(ofstream &Output)
{
  map< int, pair<string, string> >::iterator it;
  for (it=NodeStats.begin(); it!=NodeStats.end(); ++it)
  {
    int NodeID      = it->first;
    string InStats  = (it->second).first;
    string OutStats = (it->second).second;

    Output << NodeID << "," << InStats << "," << OutStats << endl;
  }
}

void Statistics::Reset(void)
{
  NodeStats.clear();
  NumInputHulls           = 0;
  NumOutputHulls          = 0;
  NumInputPoints          = 0;
  NumOutputPoints         = 0;
  NumValidIntersects      = 0;
  NumTotalIntersects      = 0;
  ElapsedTimeClustering   = 0;
  ElapsedTimeIntersecting = 0;
}

