/*****************************************************************************\
 *                        ANALYSIS PERFORMANCE TOOLS                         *
 *                             ClusteringSuite                               *
 *   Infrastructure and tools to apply clustering analysis to Paraver and    *
 *                              Dimemas traces                               *
 *                                                                           *
 *****************************************************************************
 *     ___     This library is free software; you can redistribute it and/or *
 *    /  __         modify it under the terms of the GNU LGPL as published   *
 *   /  /  _____    by the Free Software Foundation; either version 2.1      *
 *  /  /  /     \   of the License, or (at your option) any later version.   *
 * (  (  ( B S C )                                                           *
 *  \  \  \_____/   This library is distributed in hope that it will be      *
 *   \  \__         useful but WITHOUT ANY WARRANTY; without even the        *
 *    \___          implied warranty of MERCHANTABILITY or FITNESS FOR A     *
 *                  PARTICULAR PURPOSE. See the GNU LGPL for more details.   *
 *                                                                           *
 * You should have received a copy of the GNU Lesser General Public License  *
 * along with this library; if not, write to the Free Software Foundation,   *
 * Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA          *
 * The GNU LEsser General Public License is contained in the file COPYING.   *
 *                                 ---------                                 *
 *   Barcelona Supercomputing Center - Centro Nacional de Supercomputacion   *
\*****************************************************************************/

/* -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=- *\

  $Id::                                           $:  Id
  $Rev::                                          $:  Revision of last commit
  $Author::                                       $:  Author of last commit
  $Date::                                         $:  Date of last commit

\* -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=- */

#include "ConvexHullModel.hpp"
#include <CGAL/Polygon_2_algorithms.h>
#include <cassert>

#include <SystemMessages.hpp>
using cepba_tools::system_messages;

#include <sstream>
using std::ostringstream;

int ConvexHullModel::MIN_HULL_POINTS = 3;

ConvexHullModel::ConvexHullModel(void)
{
  Dimensions     = 0;
  Density        = 0;
  TotalTime      = 0;
  HullPoints.clear();
}

ConvexHullModel::ConvexHullModel( vector<const Point*> cluster_points, long long TotalTime )
{
  vector<MyPoint_2> InternalPoints;

  Dimensions = 2;

  for (size_t i = 0; i < cluster_points.size(); i++)
  {
    MyPoint_2 NewPoint((*cluster_points[i])[0], (*cluster_points[i])[1]);
    NewPoint.Instance()          = (long long) (*cluster_points[i]).GetInstance();
    NewPoint.NeighbourhoodSize() = (long long) (*cluster_points[i]).GetNeighbourhoodSize();
    InternalPoints.push_back(NewPoint);
  }

  this->Density = cluster_points.size();
  this->TotalTime = TotalTime;

  /* To compute a hull we need at least three points */
  if (Density >= ConvexHullModel::MIN_HULL_POINTS)
  {
    CGAL::convex_hull_2(InternalPoints.begin(),
                        InternalPoints.end(),
                        std::back_inserter(HullPoints) );
  }
  else
  {
    /* All points are "hull" points */
    HullPoints = InternalPoints;
  }
}

ConvexHullModel::ConvexHullModel(vector<MyPoint_2> HullPoints, long long Density, long long TotalTime)
{
  this->Density    = Density;
  this->TotalTime  = TotalTime;
  this->Dimensions = 2;

  if (HullPoints.size() >= 3)
  {
    CGAL::convex_hull_2( HullPoints.begin(), HullPoints.end(), std::back_inserter(this->HullPoints) );
  }
  else
  {
    this->HullPoints = HullPoints;
  }
}

/*
ConvexHullModel::ConvexHullModel( Polygon_2 P, Polygon_2 Q )
{
  CGALPoints JointPoints;
  Polygon_2::Vertex_iterator it;

  Dimensions = 2;

  for (it = P.vertices_begin(); it != P.vertices_end(); ++it)
  {
    JointPoints.push_back ( *it );
  }

  for (it = Q.vertices_begin(); it != Q.vertices_end(); ++it)
  {
    JointPoints.push_back ( *it );
  }

  CGAL::convex_hull_2( JointPoints.begin(), JointPoints.end(), std::back_inserter(HullPoints) );

}
*/

ConvexHullModel::ConvexHullModel(long long  Density,
                                 long long  TotalTime,
                                 int        NumPoints,
                                 int        NumDimensions,
                                 long long *Instances,
                                 long long *NeighbourhoodSizes,
                                 double    *DimValues)
{
  Assemble(Density,
           TotalTime,
           NumPoints,
           NumDimensions,
           Instances,
           NeighbourhoodSizes,
           DimValues);
}

int ConvexHullModel::size()
{
  return this->HullPoints.size();
}

long long ConvexHullModel::GetDensity()
{
  return this->Density;
}

long long ConvexHullModel::GetTotalTime()
{
  return this->TotalTime;
}

void ConvexHullModel::Flush( )
{
  vector<MyPoint_2>::iterator it;

  std::cout << HullPoints.size() << " points on the convex hull" << std::endl;

  for (it=HullPoints.begin(); it != HullPoints.end(); ++it)
  {
    std::cout << "(" << it->x() << "," << it->y() << ")" << std::endl;
  }
}

string ConvexHullModel::GetPlotLine(string DataFileName, cluster_id_t ID)
{
  ostringstream Result;

  /* WE DO NOT TAKE INTO ACCOUNT THE DIMENSION ORDERS... */
  Result << "'DataFileName' using 2:1";
}

void ConvexHullModel::Serialize(long long  &Density,
                                long long  &TotalTime,
                                int        &NumPoints,
                                int        &NumDimensions,
                                long long *&Instances,
                                long long *&NeighbourhoodSizes,
                                double    *&DimValues )
{
  vector<MyPoint_2>::iterator it;

  Density            = this->Density;
  TotalTime          = this->TotalTime;
  NumPoints          = HullPoints.size();
  NumDimensions      = Dimensions;
  Instances          = (long long*) malloc (sizeof(long long)*(NumPoints));
  NeighbourhoodSizes = (long long*) malloc (sizeof(long long)*(NumPoints));
  DimValues          = (double*)    malloc (sizeof(double)*(NumPoints*NumDimensions));


  for (size_t i = 0; i < HullPoints.size(); i++)
  {
    Instances[i]          = (long long) HullPoints[i].Instance();
    NeighbourhoodSizes[i] = (long long) HullPoints[i].NeighbourhoodSize();

    DimValues[i*NumDimensions]     = CGAL::to_double(HullPoints[i].x());
    DimValues[(i*NumDimensions)+1] = CGAL::to_double(HullPoints[i].y());
  }

  /*
  std::cout << "[Serialize] NumPoints=" << NumPoints << " NumDimensions=" << NumDimensions << " DimValues.size()=" << DimValues.size() << std::endl;
  */
}

vector<MyPoint_2>& ConvexHullModel::getHullPoints()
{
  return HullPoints;
}

bool ConvexHullModel::Flush(ostream& str, cluster_id_t id)
{
  vector<MyPoint_2>::iterator HullPointsIterator;

  if (HullPoints.size() == 0)
  {
    return false;
  }

  for (HullPointsIterator  = HullPoints.begin();
       HullPointsIterator != HullPoints.end();
       HullPointsIterator++)
  {
    str.precision(9);
    str << (*HullPointsIterator).x() << ", " << (*HullPointsIterator).y() << ", " << id << endl;
  }

  return true;
}

bool ConvexHullModel::IsInside(const Point* QueryPoint)
{
  MyPoint_2 InternalPoint((*QueryPoint)[0], (*QueryPoint)[1]);

  InternalPoint.Instance()          = QueryPoint->GetInstance();
  InternalPoint.NeighbourhoodSize() = QueryPoint->GetNeighbourhoodSize();

  return IsInside(InternalPoint);
}
bool ConvexHullModel::IsInside(const MyPoint_2& QueryPoint)
{
  ostringstream Message;

  switch(CGAL::bounded_side_2(HullPoints.begin(), HullPoints.end(), QueryPoint))
  {
    case CGAL::ON_BOUNDED_SIDE:
      return true;
      break;
    case CGAL::ON_BOUNDARY:
      return true;
      break;
    case CGAL::ON_UNBOUNDED_SIDE:
      break;
  }

  return false;
}

bool ConvexHullModel::IsNear(const Point* QueryPoint, double Epsilon, int MinPoints)
{
  vector<MyPoint_2>::iterator HullPointsIt;
  MyPoint_2 InternalQueryPoint((*QueryPoint)[0], (*QueryPoint)[1]);

  InternalQueryPoint.Instance()          = QueryPoint->GetInstance();
  InternalQueryPoint.NeighbourhoodSize() = QueryPoint->GetNeighbourhoodSize();

  // PolytopePoint_2 CGALPoint_Dist ((*QueryPoint)[0], (*QueryPoint)[1]);
  // vector<PolytopePoint_2> R, S;

  /*
  Message << " (Using distance query!) ";
  system_messages::information(Message.str()); */

  for (HullPointsIt = HullPoints.begin(); HullPointsIt != HullPoints.end(); ++HullPointsIt)
  {
    MyPoint_2 CurrentHullPoint = *HullPointsIt;

    double sqrDistance = CGAL::to_double(squared_distance(CurrentHullPoint, InternalQueryPoint));

    if ((sqrDistance <= pow(Epsilon, 2.0)) &&
        ((CurrentHullPoint.NeighbourhoodSize()+(InternalQueryPoint.NeighbourhoodSize()/2)+1) >= MinPoints))
    {
      return true;
    }
  }

  return false;
}

void ConvexHullModel::GetDistanceAndDensity(const Point* QueryPoint,
                                            double      &SqDistance,
                                            int         &Density)
{
  vector<MyPoint_2>::iterator HullPointsIt;

  MyPoint_2 InternalQueryPoint((*QueryPoint)[0], (*QueryPoint)[1]);

  SqDistance = MAX_DOUBLE;
  Density    = 0;

  for (HullPointsIt = HullPoints.begin(); HullPointsIt != HullPoints.end(); ++HullPointsIt)
  {
    MyPoint_2 CurrentHullPoint = *HullPointsIt;

    double CurrentDistance = CGAL::to_double(squared_distance(CurrentHullPoint, InternalQueryPoint));

    if (CurrentDistance < SqDistance)
    {
      SqDistance = CurrentDistance;
      Density    = CurrentHullPoint.NeighbourhoodSize();
    }
  }

  return;
}

ConvexHullModel * ConvexHullModel::Merge( ConvexHullModel * CHull2, double Epsilon, int MinPoints )
{
  Polygon_2         P, Q;
  bool              doMerge = false;
  vector<MyPoint_2> Hull2Points, JointPoints;
  ConvexHullModel  *JointHull;
  long long         JointDensity   = this->GetDensity() + CHull2->GetDensity();
  long long         JointTotalTime = this->GetTotalTime() + CHull2->GetTotalTime();               

  Hull2Points = CHull2->getHullPoints();

  /* DEBUG
  std::cerr << "MinPoints = " << MinPoints << " Hull1.size()=" << HullPoints.size() << " Hull1.density()=" << this->GetDensity() << " Hull2.size()=" << Hull2Points.size() << " Hull2.density()=" << CHull2->GetDensity() << std::endl;
  std::cerr.flush();
  */

  if (this->size() < MIN_HULL_POINTS || CHull2->size() < MIN_HULL_POINTS)
  {
    /* Check if the points in the hulls are close enough */
    doMerge = this->IsNear(CHull2, Epsilon, MinPoints);
  }
  else
  {
    for (size_t i = 0; i < HullPoints.size(); i++)
    {
      P.push_back(HullPoints[i]);
    }

    for (size_t i = 0; i < Hull2Points.size(); i++)
    {
      Q.push_back(Hull2Points[i]);
    }

    /* DEBUG
    std::cout << "Polygon P characteristics: ";
    std::cout << "Is " <<
    (P.is_simple() ? "" : "not ") << "simple. ";
    std::cout << "Is " <<
    (P.is_convex() ? "" : "not ") << "convex." << std::endl;
    */

    /* DEBUG
    std::cout << "Polygon Q characteristics: ";
    std::cout << "Is " <<
    (Q.is_simple() ? "" : "not ") << "simple. ";
    std::cout << "Is " <<
    (Q.is_convex() ? "" : "not ") << "convex." << std::endl; */

    /* Check polygon intersection */

    /* DEBUG
    if (P.size() < 3)
    {
      std::cerr << "[DEBUG] P.size()=" << P.size() << endl;
    }
    if (Q.size() < 3)
    {
      std::cerr << "[DEBUG] Q.size()=" << Q.size() << endl;
    }
    Polygon_2::Vertex_iterator it;
    for (it = P.vertices_begin(); it != P.vertices_end(); ++it)
    {
      std::cerr << "[DEBUG] P x=" << it->x() << " " << it->y() << endl;
    }
    for (it = Q.vertices_begin(); it != Q.vertices_end(); ++it)
    {
      std::cerr << "[DEBUG] Q x=" << it->x() << " " << it->y() << endl;
    }
    */

    if (CGAL::do_intersect (P, Q))
    {
      doMerge = true;
    }
    else if (Epsilon > 0)
    {
      /* Check if the hulls are close enough */
      doMerge = this->IsNear(CHull2, Epsilon, MinPoints);
    }
  }

  if (doMerge)
  {
    for (size_t i = 0; i < HullPoints.size(); i++)
    {
      JointPoints.push_back(HullPoints[i]);
    }

    for (size_t i = 0; i < Hull2Points.size(); i++)
    {
      JointPoints.push_back(Hull2Points[i]);
    }

    JointHull = new ConvexHullModel( JointPoints, JointDensity, JointTotalTime );
    /* DEBUG
    std::cout << "The two hulls are merged." << std::endl;
    JointHull->Print(); */
    return JointHull;
  }
  else
  {
    /* DEBUG
    std::cout << "The two hulls are NOT merged." << std::endl; */
    return NULL;
  }
}

bool ConvexHullModel::IsNear (ConvexHullModel *Hull2, double Epsilon, int MinPoints)
{
  K::Compute_squared_distance_2 squared_distance;

  vector<MyPoint_2> HullPoints2;
  vector<MyPoint_2>::iterator it1, it2;

  MyPoint_2 Cand1, Cand2;

  HullPoints2 = Hull2->getHullPoints();
  for (it1 = HullPoints.begin(); it1 != HullPoints.end(); ++it1)
  {
    Cand1 = *it1;
    for (it2 = HullPoints2.begin(); it2 != HullPoints2.end(); ++it2)
    {
      double sqrDistance;

      Cand2 = *it2;

      sqrDistance = CGAL::to_double(squared_distance(Cand1, Cand2));

      if ( sqrDistance <= pow(Epsilon, 2.0) &&
          (((Cand1.NeighbourhoodSize() + Cand2.NeighbourhoodSize())/2)+1) >= MinPoints)
      {
        // std::cout << "Cand1.NeighbourhoodSize = " << Cand1.NeighbourhoodSize() << " Cand2.NeighbourhoodSize = " << Cand2.NeighbourhoodSize() << std::endl;
        return true;
      }
    }
  }
  return false;
}


void ConvexHullModel::Assemble(long long  Density,
                               long long  TotalTime,
                               int        NumPoints,
                               int        NumDimensions,
                               long long *Instances,
                               long long *NeighbourhoodSizes,
                               double    *DimValues)
{
  /* 2D */
  this->Density   = Density;
  this->TotalTime = TotalTime;
  Dimensions      = NumDimensions;

  for (size_t i = 0; i < NumPoints; i++)
  {
    MyPoint_2 NewPoint(DimValues[i*NumDimensions], DimValues[i*NumDimensions+1]);

    // NewPoint.x()                 = DimValues[i*NumDimensions];
    // NewPoint.y()                 = DimValues[i*NumDimensions+1];
    NewPoint.Instance()          = Instances[i];
    NewPoint.NeighbourhoodSize() = NeighbourhoodSizes[i];

    HullPoints.push_back(NewPoint);
  }

  /*
  for (int i = 0; i < NumPoints*NumDimensions; i +=2)
  {
    double x, y;

    x = DimValues[i];
    y = DimValues[i+1];

    HullPoints.push_back ( Point_2( x, y ) );
  }
  */
}

