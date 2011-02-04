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

  $URL:: https://svn.bsc.#$:  File
  $Rev:: 20               $:  Revision of last commit
  $Author:: jgonzale      $:  Author of last commit
  $Date:: 2010-03-09 17:1#$:  Date of last commit

\* -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=- */

#include "ConvexHullModel.hpp"
#include <CGAL/Polygon_2_algorithms.h>

ConvexHullModel::ConvexHullModel( vector< const Point * > cluster_points )
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

  CGAL::convex_hull_2( cgal_points.begin(), cgal_points.end(), std::back_inserter(HullPoints) );
}

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

ConvexHullModel::ConvexHullModel( int NumPoints, int NumDimensions, double * DimValues )
{
  Assemble( NumPoints, NumDimensions, DimValues);
  
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

void ConvexHullModel::Serialize( int & NumPoints, int & NumDimensions, double*& DimValues )
{
  CGALPoints::iterator it;

  NumPoints     = HullPoints.size();
  NumDimensions = Dimensions;
  DimValues     = (double*) malloc (sizeof(double)*(NumPoints*NumDimensions));

  for (size_t i = 0; i < HullPoints.size(); i++)
  {
    DimValues[i*NumDimensions]     = HullPoints[i].x();
    DimValues[(i*NumDimensions)+1] = HullPoints[i].y();
  }

  /*
  std::cout << "[Serialize] NumPoints=" << NumPoints << " NumDimensions=" << NumDimensions << " DimValues.size()=" << DimValues.size() << std::endl;
  */
}

CGALPoints ConvexHullModel::getHullPoints()
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
  Point_2 CGALPoint ((*QueryPoint)[0], (*QueryPoint)[1]);

  switch(CGAL::bounded_side_2(HullPoints.begin(), HullPoints.end(), CGALPoint))
  {
    case CGAL::ON_BOUNDED_SIDE:
      return true;
      break;
    case CGAL::ON_BOUNDARY:
      return true;
      break;
    case CGAL::ON_UNBOUNDED_SIDE:
      return false;
      break;
  }
}

ConvexHullModel * ConvexHullModel::Merge( ConvexHullModel * CHull2 )
{
  Polygon_2 P, Q;
  CGALPoints HullPoints2, JointPoints;
  ConvexHullModel * JointHull;

  CGALPoints::iterator it;
  for (it=HullPoints.begin(); it != HullPoints.end(); ++it)
  {
    P.push_back(*it);
    JointPoints.push_back(*it);
  }
  
  HullPoints2 = CHull2->getHullPoints();
  for (it=HullPoints2.begin(); it != HullPoints2.end(); ++it)
  {
    Q.push_back(*it);
    JointPoints.push_back(*it);
  }

  if ((CGAL::do_intersect (P, Q)))
  {
    /* DEBUG
    std::cout << "The two polygons intersect in their interior." << std::endl;
    */

    JointHull = new ConvexHullModel( P, Q );
    // JointHull->Print();
    return JointHull;
  }
  else
  {
    /*
    std::cout << "The two polygons do not intersect." << std::endl;
    */
    return NULL;
  }
}

void ConvexHullModel::Assemble(int NumPoints, int NumDimensions, double * DimValues)
{
  /* 2D */
  Dimensions = NumDimensions;

  for (int i = 0; i < NumPoints*NumDimensions; i +=2)
  {
    double x, y;

    x = DimValues[i];
    y = DimValues[i+1];

    HullPoints.push_back ( Point_2( x, y ) );
  }
}



