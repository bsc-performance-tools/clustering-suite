#include <iostream>
#include <string>
#include <sstream>
#include <map>
#include "ClustersInfo.h"
#include "TDBSCANTags.h"
#include "SystemMessages.hpp"

using std::string;
using std::cout;
using std::cerr;
using std::endl;
using std::istringstream;
using std::ostringstream;
using std::map;

const char *ClustersInfoFormatString = "%s";

/* Back-end constructor */
ClustersInfo::ClustersInfo( vector<ClusterStatistics *> &Statistics )
{
  Reset();
  for (int i=0; i<Statistics.size(); i++)
  {
    Serialization << *Statistics[i];
    if (i < Statistics.size()-1) Serialization << DELIMITER_OBJECT;
  }
}

/* Back-end method to send the data to the filters */
void ClustersInfo::Serialize(STREAM *OutputStream)
{
  STREAM_send(OutputStream, TAG_CLUSTERS_INFO, ClustersInfoFormatString, Serialization.str().c_str());
}

/* FE & Filter constructor */
ClustersInfo::ClustersInfo()
{
  Reset();
}

void ClustersInfo::Reset()
{
  Serialization.str("");
  CombinedStatistics.clear();
}

void ClustersInfo::Unpack(PACKET_PTR InputPacket)
{
  char *ChildStatistics = NULL;

  PACKET_unpack(InputPacket, ClustersInfoFormatString, &ChildStatistics);

  CombineStatistics( ChildStatistics );
  
  if (ChildStatistics != NULL) free(ChildStatistics);
}

void ClustersInfo::CombineStatistics(char *ChildStatistics)
{
  string CurrentClusterStatisticsStr;

  istringstream is(ChildStatistics);

  while (getline(is, CurrentClusterStatisticsStr, DELIMITER_OBJECT))
  {
    ClusterStatistics *CurrentClusterStatistics = new ClusterStatistics( CurrentClusterStatisticsStr );

    if (CombinedStatistics.find( CurrentClusterStatistics->ID() ) != CombinedStatistics.end())
    {
      CombinedStatistics[ CurrentClusterStatistics->ID() ]->Aggregate( *CurrentClusterStatistics );
      delete CurrentClusterStatistics;
    }
    else
    {
      CombinedStatistics[ CurrentClusterStatistics->ID() ] = CurrentClusterStatistics;
    }
  }
}

char * ClustersInfo::Serialize()
{
  char *serial = NULL;

  map<int, ClusterStatistics *>::iterator it;
  int i = 0;
  for (it = CombinedStatistics.begin(); it != CombinedStatistics.end(); ++it)
  {
    ClusterStatistics *CurrentClusterStatistics = it->second;

    Serialization << *CurrentClusterStatistics;

    if (i < CombinedStatistics.size()-1) Serialization << DELIMITER_OBJECT;
    i++;
  }

  serial = strdup( Serialization.str().c_str() );
  return serial;
}

/* Filter method to send the data to upper levels of the tree */
void ClustersInfo::Serialize(int StreamID, vector<PacketPtr> &OutputPackets)
{
  char *serial = Serialize();

  PacketPtr new_packet( new Packet( StreamID, TAG_CLUSTERS_INFO, ClustersInfoFormatString, serial ) );
  new_packet->set_DestroyData(true);
  OutputPackets.push_back( new_packet );
}

void ClustersInfo::Print()
{
  char *serial = Serialize(); 
  
  fprintf(stderr, "[CLUSTERS_INFO] %s\n", serial);
  free(serial);
}

std::ostream& operator <<(std::ostream& out, const ClustersInfo& ci)
{
  int numClusters = 0;
  bool hasNoise = false;

  numClusters = ci.CombinedStatistics.size();
  if (ci.CombinedStatistics.find( NOISE_CLUSTER_ID ) != ci.CombinedStatistics.end())
  {
    hasNoise = true;
    numClusters--;
  }
  out << CLUSTER_NAME_LABEL << CSV_DELIMITER << NOISE_LABEL << CSV_DELIMITER;
  for (int i=0; i<numClusters; i++)
  {
    out << CLUSTER_LABEL << i+1;
    if (i < numClusters-1) out << CSV_DELIMITER;
  }
  out << endl;

  map<int, ClusterStatistics *>::const_iterator it;  

  out << DENSITY_LABEL; 
  if (!hasNoise) out << CSV_DELIMITER << 0;
  for (it = ci.CombinedStatistics.begin(); it != ci.CombinedStatistics.end(); ++it)
  {
    out << CSV_DELIMITER << it->second->Individuals();
  }
  out << endl;

  double AbsoluteTotalDuration = 0;

  out << TOTAL_DURATION_LABEL;
  if (!hasNoise) out << CSV_DELIMITER << 0;
  for (it = ci.CombinedStatistics.begin(); it != ci.CombinedStatistics.end(); ++it)
  {
    out << CSV_DELIMITER << it->second->TotalDuration();
    AbsoluteTotalDuration += it->second->TotalDuration();
  }
  out << endl;

  out << AVG_DURATION_LABEL;
  if (!hasNoise) out << CSV_DELIMITER << 0;
  for (it = ci.CombinedStatistics.begin(); it != ci.CombinedStatistics.end(); ++it)
  {
    out << CSV_DELIMITER << it->second->DurationMean();
  }
  out << endl;

  out << PCT_TOTAL_DURATION_LABEL;
  if (!hasNoise) out << CSV_DELIMITER << 0;
  for (it = ci.CombinedStatistics.begin(); it != ci.CombinedStatistics.end(); ++it)
  {
    out << CSV_DELIMITER << it->second->TotalDuration() / AbsoluteTotalDuration;
  }
  out << endl;

  int numMetrics = ci.CombinedStatistics.begin()->second->size();
  for (int i=0; i<numMetrics; i++)
  {
    string MetricLabel = (*ci.CombinedStatistics.begin()->second)[i].MetricID();
    out << MetricLabel;
    if (!hasNoise) out << CSV_DELIMITER << 0;
    for (it = ci.CombinedStatistics.begin(); it != ci.CombinedStatistics.end(); ++it)
    {
      out << CSV_DELIMITER << (*it->second)[i].Mean();
    }
    out << endl;
  }

  return out;
}

