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

#include "Point.hpp"

#include <cmath>
#include <cassert>
#include <limits>

#include <iostream>
using std::cout;
using std::endl;

#if defined (HAVE_MUSTER) && defined (HAVE_MPI)
size_t Point::PointDimensions = 0;
#endif

Point::Point(size_t Dimensions)
{
  this->Dimensions = vector<double> (Dimensions, 0.0);

#if defined (HAVE_MUSTER) && defined (HAVE_MPI)
  if (Point::PointDimensions == 0)
  {
    Point::PointDimensions = Dimensions;
  }
#endif
}

Point::Point(vector<double>& _Dimensions)
{
  Dimensions = _Dimensions;
  Normalized = false;

#if defined (HAVE_MUSTER) && defined (HAVE_MPI)
  if (Point::PointDimensions == 0)
  {
    Point::PointDimensions = _Dimensions.size();
  }
#endif
  
  /* DEBUG
  cout << "New point! Dimensions size = " << _Dimensions.size() << endl;
  */
}

void
Point::RangeNormalization(const vector<double>& MaxValues,
                          const vector<double>& MinValues,
                          const vector<double>& Factors)
{
  double BaseValue;
  
  /* DEBUG
  cout << "Original Dimensions = {";
  for (size_t i = 0; i < Dimensions.size(); i++)
  {
    cout << Dimensions[i] << " ";
  }
  cout << "}" << endl;
  */

  
  for (size_t i = 0; i < Dimensions.size(); i++)
  {
    BaseValue = Dimensions[i];
  
    Dimensions[i] = Factors[i]*((BaseValue - MinValues[i]) / (MaxValues[i] - MinValues[i]));
  }

  Normalized = true;
  
  /* DEBUG
  cout << "Resulting Dimensions = {";
  for (size_t i = 0; i < Dimensions.size(); i++)
  {
    cout << Dimensions[i] << " ";
  }
  cout << "}" << endl;
  */
}

double Point::EuclideanDistance(const Point& OtherPoint) const
{
  double Result = std::numeric_limits<double>::max();

  if (Dimensions.size() != OtherPoint.size())
  {
    return Result;
  }
  else
  {
    Result = 0.0;
    
    for (size_t i = 0; i < Dimensions.size(); i++)
    {
      
      Result += pow(Dimensions[i] - OtherPoint.Dimensions[i], 2.0);
    }
    
    return sqrt(Result);
  }
}

size_t Point::size(void) const
{
  return Dimensions.size();
}

void Point::clear(void)
{
  Dimensions.clear();
  Normalized = false;
}

double& Point::operator[](int i)
{
  assert(i >= 0 && i < Dimensions.size());

  return Dimensions[i];
}

const double
Point::operator[](int i) const
{
  assert(i >= 0 && i < Dimensions.size());

  return Dimensions[i];
}

Point Point::operator +  (const Point& other)
{
  Point Result((*this).size());

  if ((*this).size() != other.size())
  {
    return Point(0);
  }

  for (size_t i = 0; i < (*this).size(); i++)
  {
    Result[i] = (*this)[i] + other[i];
  }

  return Result;
}

Point Point::operator /  (const size_t scalar)
{
  Point Result((*this).size());

  for (size_t i = 0; i < (*this).size(); i++)
  {
    Result[i] = (*this)[i] / scalar;
  }

  return Result;
}

bool Point::operator != (const Point& other) const
{
  if (this->Normalized != other.IsNormalized())
    return true;

  if (this->size() != other.size())
    return true;

  for (size_t i = 0; i < Dimensions.size(); i++)
  {
    if (this[i] != other[i])
    {
      return true;
    }
  }

  return false;
}

Point& Point::operator =  (const Point& other)
{
  if ((*this) != other)
  {
    Dimensions.clear();
    
    for (size_t i = 0; i < other.size(); i++)
    {
      Dimensions.push_back(other[i]);
    }
  }

  return *this;
}

void Point::PrintPoint(void)
{
  for (size_t i = 0; i < Dimensions.size(); i++)
  {
    cout << " [" << i << "] = " << Dimensions[i];
  }
  cout << endl;
}
