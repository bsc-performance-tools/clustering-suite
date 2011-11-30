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

  $Id:: ConvexHullModel.hpp 36 2011-11-21 11:00:1#$:  Id
  $Rev:: 36                                       $:  Revision of last commit
  $Author:: jgonzale                              $:  Author of last commit
  $Date:: 2011-11-21 12:00:12 +0100 (Mon, 21 Nov #$:  Date of last commit

\* -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=- */

#ifndef _HULLMODEL_HPP_
#define _HULLMODEL_HPP_

#include "clustering_types.h"
#include <iostream>
using std::ostream;

class ConvexHullModel;

class HullModel
{
  private:

    ConvexHullModel* _Model;

  public:

    HullModel(void);

    HullModel(ConvexHullModel* Model);

    HullModel(long long  Density,
              int        NumPoints,
              int        NumDimensions,
              long long *Instances,
              long long *NeighbourhoodSizes,
              double    *DimValues);

    ~HullModel(void);

    void Serialize(long long  &Density,
                   int        &NumPoints,
                   int        &NumDimensions,
                   long long *&Instances,
                   long long *&NeighbourhoodSizes,
                   double    *&DimValues );

    HullModel* Merge (HullModel* Other, double Epsilon, int MinPoints);

    ConvexHullModel* const Model(void);

    int  Density(void);

    int  Size(void);

    void Flush(void);

    bool Flush(ostream&     str,
               cluster_id_t id);
};

#endif

