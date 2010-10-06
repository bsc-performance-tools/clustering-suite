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

#include "Point.hpp"

#include <cmath>
#include <cassert>
#include <limits>

#include <iostream>
using std::cout;
using std::endl;

Point::Point(size_t Dimensions)
{
  this->Dimensions = vector<double> (Dimensions, 0.0);
}

Point::Point(vector<double>& _Dimensions)
{
  Dimensions = _Dimensions;
  Normalized = false;

  cout << "New point! Dimensions size = " << _Dimensions.size() << endl;
}

void
Point::RangeNormalization(const vector<double>& MaxValues,
                          const vector<double>& MinValues)
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
  
    Dimensions[i] =
      (BaseValue - MinValues[i]) / (MaxValues[i] - MinValues[i]);
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

double
Point::EuclideanDistance(Point& OtherPoint) const
{
  double Result = std::numeric_limits<double>::max();;

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

double&
Point::operator[](int i)
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

void
Point::PrintPoint(void)
{
  for (size_t i = 0; i < Dimensions.size(); i++)
  {
    cout << " [" << i << "] = " << Dimensions[i];
  }
  cout << endl;
}
