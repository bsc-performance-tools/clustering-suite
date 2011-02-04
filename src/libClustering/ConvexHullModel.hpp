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

#ifndef _CONVEX_HULL_MODEL_HPP_
#define _CONVEX_HULL_MODEL_HPP_

#include <vector>
using std::vector;
#include <iostream>
using std::ostream;
using std::endl;


#include <Point.hpp>

#include <CGAL/Exact_predicates_exact_constructions_kernel.h>
#include <CGAL/Exact_predicates_inexact_constructions_kernel.h>
#include <CGAL/Boolean_set_operations_2.h>
#include <CGAL/convex_hull_2.h>

typedef CGAL::Exact_predicates_inexact_constructions_kernel K;
typedef K::Point_2 Point_2;
typedef std::vector<Point_2> CGALPoints;
typedef CGAL::Polygon_2<K>                           Polygon_2;

class ConvexHullModel
{
  protected:

    int Dimensions;
    CGALPoints HullPoints;

  public:

    ConvexHullModel ( vector< const Point* > );
    ConvexHullModel ( Polygon_2 P, Polygon_2 Q );
    ConvexHullModel ( int NumPoints, int NumDimensions, double* DimValues );
    
    void Serialize  ( int & NumPoints, int & NumDimensions, double*& DimValues );
    void Flush( );
    ConvexHullModel * Merge( ConvexHullModel * CHull2 );
    CGALPoints getHullPoints();

    bool IsInside(const Point* QueryPoint);
    
    bool Flush(ostream&             str,
               cluster_id_t         id);

  private:
    void Assemble( int NumPoints, int NumDimensions, double * DimValues );
};

#endif // _CONVEX_HULL_MODEL_HPP_
