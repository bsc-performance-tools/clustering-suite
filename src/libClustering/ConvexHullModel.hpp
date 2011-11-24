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

#ifndef _CONVEX_HULL_MODEL_HPP_
#define _CONVEX_HULL_MODEL_HPP_

#include <vector>
using std::vector;
#include <iostream>
using std::ostream;
using std::endl;

#include <Point.hpp>

#include <CGAL/basic.h>
#include <CGAL/Filtered_kernel.h>
#include <CGAL/Boolean_set_operations_2.h>
#include <CGAL/convex_hull_2.h>

#include "CGAL_Kernel/MyKernel.h"

typedef MyKernel<float>                  K;
//typedef CGAL::Filtered_kernel_adaptor<MK> K;
typedef K::Point_2                        MyPoint_2;
typedef CGAL::Polygon_2<K>                Polygon_2;

class ConvexHullModel
{
  protected:

    int               Dimensions;
    long long         Density;
    vector<MyPoint_2> HullPoints;

  public:

    static int MIN_HULL_POINTS;

    ConvexHullModel(void);

    ConvexHullModel ( vector< const Point* > );

    ConvexHullModel(vector<MyPoint_2> HullPoints, long long Density);
    /* ConvexHullModel ( Polygon_2 P, Polygon_2 Q ); */
    ConvexHullModel(long long  Density,
                    int        NumPoints,
                    int        NumDimensions,
                    long long *Instances,
                    long long *NeighbourhoodSizes,
                    double    *DimValues);

    int size(void);
    long long GetDensity(void);

    void Serialize(long long  &Density,
                   int        &NumPoints,
                   int        &NumDimensions,
                   long long *&Instances,
                   long long *&NeighbourhoodSizes,
                   double    *&DimValues );
    void Flush( );
    ConvexHullModel * Merge( ConvexHullModel * CHull2, double Epsilon = 0, int MinPoints = 1);

    vector<MyPoint_2>& getHullPoints();

    bool IsInside(const Point* QueryPoint);
    bool IsInside(const MyPoint_2& QueryPoint);
    bool IsNear(const Point* QueryPoint, double Epsilon, int MinPoints);
    bool IsNear (ConvexHullModel *Hull2, double Epsilon, int MinPoints);

    void GetDistanceAndDensity(const Point* QueryPoint, double &SqDistance, int&Density);

    bool    Flush(ostream&             str,
                  cluster_id_t         id);

    string  GetPlotLine(string DataFileName, cluster_id_t ID);

  private:

    void   Assemble(long long  Density,
                    int        NumPoints,
                    int        NumDimensions,
                    long long *Instances,
                    long long *NeighbourhoodSizes,
                    double    *DimValues );

};

#endif // _CONVEX_HULL_MODEL_HPP_
