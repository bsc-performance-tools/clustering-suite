#include "Support.h"
#include "TDBSCANTags.h"
#include "Utils.h"
#include <iostream>
#include <fstream>

using std::cerr;
using std::cout;
using std::endl;
using std::ofstream;

/* Front-end */
Support::Support(int NumberOfDimensions, double *MinGlobalDimensions, double *MaxGlobalDimensions)
{
  GridResolution = 0;
  Grid.clear();
  for (int i=0; i<NumberOfDimensions; i++)
  {
    MinDimensions.push_back( MinGlobalDimensions[i] );
    MaxDimensions.push_back( MaxGlobalDimensions[i] );
  }
}

/* Filter */
Support::Support()
{
  GridResolution = 0;
  Grid.clear();
}

void Support::Unpack(PACKET_PTR InputPacket)
{
  int NumberOfPoints = 0;
  int *Cells = NULL;
  int *Hits  = NULL;

  PACKET_unpack(InputPacket, "%d %ad %ad",
    &GridResolution,
    &Cells, &NumberOfPoints,
    &Hits, &NumberOfPoints);

  for (int i=0; i<NumberOfPoints; i++)
  {
    if (Grid.find( Cells[i] ) == Grid.end())
    {
      Grid[Cells[i]] = Hits[i];
    }
    else
    {
      Grid[Cells[i]] += Hits[i];
    }
  }
}

void Support::Serialize(int stream_id, std::vector< PacketPtr >& packets_out)
{
  int NumberOfPoints = 0;
  int *Cells = NULL;
  int *Hits  = NULL;

  Serialize(NumberOfPoints, Cells, Hits);

  PacketPtr new_packet( new Packet( stream_id, TAG_SUPPORT, "%d %ad %ad", 
    GridResolution,
    Cells, NumberOfPoints,
    Hits, NumberOfPoints) );

  new_packet->set_DestroyData(true);

  packets_out.push_back( new_packet );
}


/* Back-end */
Support::Support(libDistributedClustering *libClustering, int Resolution)
{
  vector<timestamp_t>  BeginTimes, EndTimes;
  vector<cluster_id_t> ClusterIDs;

  GridResolution = Resolution;
  Grid.clear();

  libClustering->GetFullBurstsInformation(Points, BeginTimes, EndTimes, ClusterIDs);

  libClustering->GetParameterRanges( MinDimensions, MaxDimensions );

  BuildGrid();
}

vector<Point *> Support::GetPoints()
{
  return Points;
}

vector<double> Support::GetMinDimensions()
{
  return MinDimensions;
}

vector<double> Support::GetMaxDimensions()
{
  return MaxDimensions;
}

int Support::GetResolution()
{
  return GridResolution;
}

Support::Support(Support &LocalSupport)
{
  Points         = LocalSupport.GetPoints(); 
  MinDimensions  = LocalSupport.GetMinDimensions();
  MaxDimensions  = LocalSupport.GetMaxDimensions();
  GridResolution = LocalSupport.GetResolution();
  Grid.clear();
}


void Support::BuildGrid()
{
  for (int i=0; i<Points.size(); i++)
  {
    Point *p = Points[i];
    int cell = Get_Cell( p );

    Grid[ cell ] = 1;
  }
}

int Support::Get_Cell(Point *p)
{
  double X = (*p)[0];
  double Y = (*p)[1];

  int i = (int) ((X - MinDimensions[0]) / ((MaxDimensions[0] - MinDimensions[0]) / GridResolution));
  int j = (int) ((Y - MinDimensions[1]) / ((MaxDimensions[1] - MinDimensions[1]) / GridResolution));
  
  if (i >= GridResolution) i--;
  if (j >= GridResolution) j--;

  int index = i * GridResolution + j;

  return index;
}

void Support::Serialize(int &NumberOfPoints, int *&Cells, int *&Hits)
{
  map<int, int>::iterator it;
  int  i     = 0;
  NumberOfPoints = Grid.size();

  Cells = (int *)malloc( sizeof(int) * Grid.size() );
  Hits  = (int *)malloc( sizeof(int) * Grid.size() );

  for (it = Grid.begin(); it != Grid.end(); ++it)
  {
    Cells[i] = it->first;
    Hits[i]  = it->second;
    i++;
  }
}

void Support::Serialize(Stream *stream)
{
  int NumberOfPoints = 0;
  int *Cells = NULL;
  int *Hits  = NULL;

  Serialize(NumberOfPoints, Cells, Hits);

  stream->send(TAG_SUPPORT, "%d %ad %ad", 
    GridResolution,
    Cells, Grid.size(),
    Hits, Grid.size());

  xfree(Cells);
  xfree(Hits);
}

void Support::GetSupport(vector<int> &BurstsSupport)
{
  for (int i=0; i<Points.size(); i++)
  {
    Point *p = Points[i];
    int cell = Get_Cell( p );

    BurstsSupport.push_back( Grid[ cell ] );
  }
}

void Support::dump()
{
   map<int, int>::iterator it;
   cerr << "Dumping support information:" << endl;

   for (it = Grid.begin(); it != Grid.end(); ++it)
   {
      cerr << "Cell: " << it->first << " Hits: " << it->second << endl;
   }
   cerr << endl;
}

void Support::Get_BBox( int cell, Box & b_box )
{

   int i = (int)cell / GridResolution;
   int j = cell - (i * GridResolution);

   double stepsX = ((MaxDimensions[0] - MinDimensions[0]) / GridResolution);
   double stepsY = ((MaxDimensions[1] - MinDimensions[1]) / GridResolution);

   double fromX = i*stepsX;
   double toX = (i+1)*stepsX;
   double fromY = j*stepsY;
   double toY = (j+1)*stepsY;

   /* cell 0 is not (0,0), there's an offset */
   fromX += MinDimensions[0];
   toX   += MinDimensions[0];
   fromY += MinDimensions[1];
   toY   += MinDimensions[1];

//   cout << "from (" << fromX << "," << fromY << ") to (" << toX << "," << toY << ")" << endl;

   b_box = Box(fromX, fromY, toX, toY);
}

void Support::plot(string outFile)
{
   ofstream myfile;
   myfile.open (outFile.c_str());

   map<int, int>::iterator it;
   for (it = Grid.begin(); it != Grid.end(); ++it)
   {
      Support::Box b_box;
      Get_BBox(it->first, b_box);

      double X = b_box.min[0] + ((b_box.max[0]-b_box.min[0])/2);
      double Y = b_box.min[1] + ((b_box.max[1]-b_box.min[1])/2);
      double Z = it->second;

      myfile << X << " " << Y << " " << Z << endl;
   }
   myfile.close();
}

void Support::plot2(string outFile)
{
   ofstream myfile;
   myfile.open (outFile.c_str());

   map<int, int>::iterator it;
   for (it = Grid.begin(); it != Grid.end(); ++it)
   {
      Support::Box b_box;
      Get_BBox(it->first, b_box);
      double X = b_box.min[0] + ((b_box.max[0]-b_box.min[0])/2);
      double Y = b_box.min[1] + ((b_box.max[1]-b_box.min[1])/2);
      int    Z = it->second;
      for (int i = 0; i < Z; i ++)
      {
        myfile << X << "," << Y << endl;
      }
   }
   myfile.close();
}


#if 0
int main()
{ 
   Point *p1, *p2, *p3, *p4, *p5;
   vector<double> d1;
   vector<double> d2;
   vector<double> d3;
   vector<double> d4;
   vector<double> d5;

   d1.push_back(1000);
   d1.push_back(1);
   d1.push_back(0.15);

   d2.push_back(2000);
   d2.push_back(2);
   d2.push_back(0.25);

   d3.push_back(3000);
   d3.push_back(3);
   d3.push_back(0.35);

   d4.push_back(4000);
   d4.push_back(4);
   d4.push_back(0.45);

   d5.push_back(5000);
   d5.push_back(5);
   d5.push_back(0.55);

   p1 = new Point(d1);
   p2 = new Point(d2);
   p3 = new Point(d3);
   p4 = new Point(d4);
   p5 = new Point(d5);

   cout << p1->size() << endl;
   cout << p2->size() << endl;
   cout << p3->size() << endl;
   cout << p4->size() << endl;
   cout << p5->size() << endl;
}
#endif
