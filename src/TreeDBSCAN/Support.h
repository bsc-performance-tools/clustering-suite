
#ifndef __SUPPORT_H__
#define __SUPPORT_H__

#include <vector>
#include <map>

using std::vector;
using std::map;

#include <libDistributedClustering.hpp>
#include "MRNet_wrappers.h"

class Support
{
  public:

      struct Box
      {
         Box() { }
         Box(double minX, double minY, double maxX, double maxY)
         {
            min[0] = minX;
            min[1] = minY;
            max[0] = maxX;
            max[1] = maxY;
         }
         double min[2];
         double max[2];
      };

    /* Front-end */
    Support(int NumberOfDimensions, double *MinGlobalDimensions, double *MaxGlobalDimensions);


    /* Filter */
    Support();
    void Unpack(PACKET_PTR InputPacket);

    void Serialize(int stream_id, vector< PacketPtr >& packets_out);

    /* Back-end */
    Support(libDistributedClustering *libClustering, int Resolution);
    Support(Support &LocalSupport);

    vector<Point *> GetPoints();
    vector<double>  GetMinDimensions();
    vector<double>  GetMaxDimensions();
    int             GetResolution();

    void BuildGrid();
    int Get_Cell(Point *p);
    void Serialize(Stream *stream);

    void GetSupport(vector<int> &BurstsSupport);

    void dump();
    void plot(string outFile);
    void plot2(string outFile);
    void Get_BBox(int cell, Box & b_box);



  private:
    vector<Point *> Points;
    vector<double>  MinDimensions;
    vector<double>  MaxDimensions;
    int             GridResolution;
    map<int, int>   Grid;

    void Serialize(int &NumberOfPoints, int *&Cells, int *&Hits);

};

#endif /* __SUPPORT_H__ */

