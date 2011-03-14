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

  $URL::                                                                   $:

  $Rev::                            $:  Revision of last commit
  $Author::                         $:  Author of last commit
  $Date::                           $:  Date of last commit

\* -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=- */

#ifndef _POINT_HPP_
#define _POINT_HPP_

#include <vector>
using std::vector;

#include "clustering_types.h"

class Point
{
  protected:
    vector<double> Dimensions;

    bool Normalized;
    
  public:


#if defined (HAVE_MUSTER) && defined (HAVE_MPI)
    static size_t PointDimensions;
#endif

    Point() {};
    
    Point(size_t Dimensions);

    Point(vector<double>& _Dimensions);

    void   RangeNormalization(const vector<double>& MaxValues,
                              const vector<double>& MinValues,
                              const vector<double>& Factors);

    void   ScaleDimensions(const vector<double>& Factors);
    
    double EuclideanDistance(const Point& OtherPoint) const;
    
    double NormalizedEuclideanDistance(Point& OtherPoint) const;

    bool   IsNormalized(void) const { return Normalized; };
    void   SetNormalized(bool Normalized) { this->Normalized = Normalized; };

    void   clear(void);
    
    size_t size(void) const;

    double&      operator [] (int i);
    const double operator [] (int i) const;
    Point        operator +  (const Point& other);
    Point        operator /  (const size_t scalar);
    bool         operator != (const Point& other) const;
    Point&       operator =  (const Point& other);
    
    void check_const(void) { Normalized = true;};
    
    void PrintPoint(void);
    
  private:

};

/*
Point* operator+ (Point* left, Point* right)
{
  Point* result;
  
  if (left == NULL && right == NULL)
  {
    return NULL;
  }
  else if (left == NULL && right != NULL)
  {
    result = new Point(right->size());

    for (size_t i = 0; i < right->size(); i++)
    {
      Point->[i] = right->[i];
    }
    
    return result;
  }
  else if (left != NULL && right == NULL)
  {
    result = new Point(left->size());

    for (size_t i = 0; i < left->size(); i++)
    {
      Point->[i] = left->[i];
    }
    
    return result;
  }

  if (left->size() != right->size())
  {
    return NULL;
  }
  
  Point* result = new Point(left->size());

  for (size_t i = 0; i < left->size(); i++)
  {
    result->[i] = left->[i] + right->[i];
  }

  return result
}
*/

/* Distance functor to use with 'muster' library */
struct PointEuclideanDistance
{
  double operator()(const Point* left, const Point* right) const
  {
      return left->EuclideanDistance((*right));
  }

  /*
  double operator()(const Point left, const Point right) const
  {
    return left.EuclideanDistance (right);
  }
  */

  double operator()(const Point& left, const Point& right) const
  {
    return left.EuclideanDistance (right);
  }
};


#endif // _POINT_HPP_
