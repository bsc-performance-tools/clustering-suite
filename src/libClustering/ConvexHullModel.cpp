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

ConvexHullModel::ConvexHullModel( vector<const Point*> cluster_points )
{
  vector<Point *>::const_iterator it;
  CGALPoints cgal_points;

  Dimensions = 2;

  for (size_t i = 0; i < cluster_points.size(); i++)
  {
    double x, y;
    x = (*cluster_points[i])[0];
    y = (*cluster_points[i])[1];

    cgal_points.push_back(Point_2(x, y));
  }

  Density = cluster_points.size();
  /* To compute a hull we need at least three points */
  if (Density >= ConvexHullModel::MIN_HULL_POINTS)
  {
    CGAL::convex_hull_2( cgal_points.begin(), cgal_points.end(), std::back_inserter(HullPoints) );
  }
  else
  {
    /* All points are "hull" points */
    HullPoints = cgal_points;
  }
}

ConvexHullModel::ConvexHullModel(CGALPoints HullPoints, long long Density)
{
  this->Density    = Density;
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

ConvexHullModel::ConvexHullModel( long long Density, int NumPoints, int NumDimensions, double * DimValues )
{
  Assemble( Density, NumPoints, NumDimensions, DimValues);
  
}

int ConvexHullModel::size()
{
  return this->HullPoints.size();
}

long long ConvexHullModel::GetDensity()
{
  return this->Density;
}

void ConvexHullModel::Flush( )
{
  CGALPoints::iterator it;

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

void ConvexHullModel::Serialize( long long& Density, int & NumPoints, int & NumDimensions, double*& DimValues )
{
  CGALPoints::iterator it;

  Density       = this->Density;
  NumPoints     = HullPoints.size();
  NumDimensions = Dimensions;
  DimValues     = (double*) malloc (sizeof(double)*(NumPoints*NumDimensions));

  for (size_t i = 0; i < HullPoints.size(); i++)
  {
    DimValues[i*NumDimensions]     = CGAL::to_double(HullPoints[i].x());
    DimValues[(i*NumDimensions)+1] = CGAL::to_double(HullPoints[i].y());
  }

  /*
  std::cout << "[Serialize] NumPoints=" << NumPoints << " NumDimensions=" << NumDimensions << " DimValues.size()=" << DimValues.size() << std::endl;
  */
}

CGALPoints& ConvexHullModel::getHullPoints()
{
  return HullPoints;
}

bool ConvexHullModel::Flush(ostream& str, cluster_id_t id)
{
  CGALPoints::iterator HullPointsIterator;

  for (HullPointsIterator  = HullPoints.begin();
       HullPointsIterator != HullPoints.end();
       HullPointsIterator++)
  {
    str << (*HullPointsIterator).x() << ", " << (*HullPointsIterator).y() << ", " << id << endl;
  }
  
  return true;
}

bool ConvexHullModel::IsInside(const Point* QueryPoint)
{
  Point_2 CGALPoint((*QueryPoint)[0], (*QueryPoint)[1]);
  
  return IsInside(CGALPoint);
}
bool ConvexHullModel::IsInside(const Point_2& QueryPoint)
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

bool ConvexHullModel::IsNear(const Point* QueryPoint, double Epsilon)
{
  CGALPoints::iterator it;
  PolytopePoint_2 CGALPoint_Dist ((*QueryPoint)[0], (*QueryPoint)[1]);
  vector<PolytopePoint_2> R, S;

  /*
  Message << " (Using distance query!) ";
  system_messages::information(Message.str()); */
  
  for (it=HullPoints.begin(); it != HullPoints.end(); ++it)
  {
    R.push_back(PolytopePoint_2(CGAL::to_double(it->x()), CGAL::to_double(it->y())));
  }

  S.push_back(CGALPoint_Dist);

  if (GenericDistance(R, S) <= pow(Epsilon, 2.0))
  {
    return true;
  }
  
  return false;
}

ConvexHullModel * ConvexHullModel::Merge( ConvexHullModel * CHull2, double Epsilon, int MinPoints )
{
  Polygon_2            P, Q;
  bool                 doMerge = false;
  CGALPoints           Hull2Points, JointPoints;
  ConvexHullModel      *JointHull;
  int                  TotalDensity = this->GetDensity() + CHull2->GetDensity();

  Hull2Points = CHull2->getHullPoints();

  /* DEBUG 
  std::cerr << "Hull1.size()=" << HullPoints.size() << " Hull1.density()=" << this->GetDensity() << " Hull2.size()=" << Hull2Points.size() << " Hull2.density()=" << CHull2->GetDensity() << std::endl; */

  for (size_t i = 0; i < HullPoints.size(); i++)
  {
    P.push_back(HullPoints[i]);
  }

  for (size_t i = 0; i < Hull2Points.size(); i++)
  {
    Q.push_back(Hull2Points[i]);
  }

  /* Check polygon intersection */
  if (CGAL::do_intersect (P, Q))
  {
    doMerge = true;
  } 
  else if (Epsilon > 0)
  {
    /* Check polytope distances */
    double sqrDistance = this->DistanceTo (CHull2);

    if (sqrDistance <= pow(Epsilon, 2.0))
    {
      /* The two hulls are close below Epsilon */
      doMerge = true;
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

    JointHull = new ConvexHullModel( JointPoints, TotalDensity );
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

#if 0
ConvexHullModel * ConvexHullModel::Merge( ConvexHullModel * CHull2, double Epsilon, int MinPoints )
{
  Polygon_2            P, Q;
  CGALPoints           Hull2Points, JointPoints;
  CGALPoints::iterator it;
  ConvexHullModel      *JointHull;
  bool                 doMerge = false;
  int                  TotalDensity;
  
  ostringstream        Message;
  
  Hull2Points = CHull2->getHullPoints();
  
  TotalDensity = this->GetDensity() + CHull2->GetDensity();
  
  /*  
  if (TotalDensity < MinPoints)
  {
    return NULL;
  }
  */

  if (this->GetDensity() < ConvexHullModel::MIN_HULL_POINTS && 
      CHull2->GetDensity() < ConvexHullModel::MIN_HULL_POINTS)
  { /* Check point by point the distances */
    
    /* DEBUG 
    Message << "Checking two small hulls" << std::endl;
    system_messages::information(Message.str()); */
  
    for (size_t i = 0; i < HullPoints.size(); i++)
    {
      for (size_t j = 0; j < Hull2Points.size(); j++)
      {
        if (squared_distance(HullPoints[i], Hull2Points[j]) <= pow(Epsilon, 2.0))
        {
          doMerge = true;
          break;
        }
      }

      if (doMerge)
      {
        break;
      }
      
    }
  }
  else 
  if (this->GetDensity() < ConvexHullModel::MIN_HULL_POINTS ||
      CHull2->GetDensity() < ConvexHullModel::MIN_HULL_POINTS)
  {
    /* DEBUG
    Message << "Checking one small hull vs. one big hull" << std::endl;
    system_messages::information(Message.str());
    */
    
    if (this->DistanceTo(CHull2) <= pow(Epsilon, 2.0))
    {
      doMerge = true;
    }
    else
    { /* Intersection criteria */
      ConvexHullModel *BigHull,       *SmallHull;
      if (this->GetDensity() >= ConvexHullModel::MIN_HULL_POINTS)
      {
        BigHull   = this;
        SmallHull = CHull2;
      }
      else
      {
        BigHull   = CHull2;
        SmallHull = this;
      }
      CGALPoints &BigHullPoints   = BigHull->getHullPoints();
      CGALPoints &SmallHullPoints = SmallHull->getHullPoints();
      
      for (size_t i = 0; i < SmallHullPoints.size(); i++)
      {
        if (BigHull->IsInside(SmallHullPoints[i]))
        {
          doMerge = true;
          break;
        }
      }
    }
  }
  else 
  if (this->GetDensity() >= ConvexHullModel::MIN_HULL_POINTS && 
           CHull2->GetDensity() >= ConvexHullModel::MIN_HULL_POINTS)
  {
    /* DEBUG
    Message << "Checking two big hulls" << std::endl;
    system_messages::information(Message.str()); */

    for (size_t i = 0; i < HullPoints.size(); i++)
    {
      P.push_back(HullPoints[i]);
    }
  
    for (size_t i = 0; i < Hull2Points.size(); i++)
    {
      Q.push_back(Hull2Points[i]);
    }

    /* Check polygon intersection */
    if (CGAL::do_intersect (P, Q))
    {
      doMerge = true;
    }
    else if (Epsilon > 0)
    {
      /* Check polytope distances */
      double sqrDistance = this->DistanceTo (CHull2);

      if (sqrDistance <= pow(Epsilon, 2.0))
      {
        /* The two hulls are close below Epsilon */
        doMerge = true;
      }
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
    
    JointHull = new ConvexHullModel( JointPoints, TotalDensity );
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
#endif

double ConvexHullModel::DistanceTo ( ConvexHullModel * CHull2 )
{
  CGALPoints HullPoints2;
  CGALPoints::iterator it;
  vector<PolytopePoint_2> R, S;

  for (it=HullPoints.begin(); it != HullPoints.end(); ++it)
  {
    R.push_back(PolytopePoint_2(CGAL::to_double(it->x()), CGAL::to_double(it->y())));
  }

  HullPoints2 = CHull2->getHullPoints();
  for (it=HullPoints2.begin(); it != HullPoints2.end(); ++it)
  {
    S.push_back(PolytopePoint_2(CGAL::to_double(it->x()), CGAL::to_double(it->y())));
  }

  return GenericDistance (R, S);
}



void ConvexHullModel::Assemble(long long Density, int NumPoints, int NumDimensions, double * DimValues)
{
  /* 2D */
  this->Density = Density;
  Dimensions    = NumDimensions;

  for (int i = 0; i < NumPoints*NumDimensions; i +=2)
  {
    double x, y;

    x = DimValues[i];
    y = DimValues[i+1];

    HullPoints.push_back ( Point_2( x, y ) );
  }
}

double ConvexHullModel::GenericDistance(vector<PolytopePoint_2> P, vector<PolytopePoint_2> Q)
{
  Polytope_distance pd(P.begin(), P.end(), Q.begin(), Q.end());

  assert (pd.is_valid());

  double sqrDistance = CGAL::to_double (pd.squared_distance_numerator()) /
                       CGAL::to_double (pd.squared_distance_denominator());

  /* std::cout << "[ConvexHullModel::DistanceTo] Squared distance=" << sqrDistance << std::endl; */

  return sqrDistance;
}
