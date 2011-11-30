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

#include "HullModel.hpp"
#include <ConvexHullModel.hpp>

HullModel::HullModel(void)
{
  _Model = NULL;
}

HullModel::HullModel(ConvexHullModel* Model)
{
  _Model = Model;
}

HullModel::HullModel(long long  Density,
                     int        NumPoints,
                     int        NumDimensions,
                     long long *Instances,
                     long long *NeighbourhoodSizes,
                     double    *DimValues)
{
  _Model = new ConvexHullModel(Density,
                               NumPoints,
                               NumDimensions,
                               Instances,
                               NeighbourhoodSizes,
                               DimValues);
}

HullModel::~HullModel(void)
{
  if (_Model != NULL)
  {
    delete _Model;
  }
}

void HullModel::Serialize(long long  &Density,
                          int        &NumPoints,
                          int        &NumDimensions,
                          long long *&Instances,
                          long long *&NeighbourhoodSizes,
                          double    *&DimValues )
{
  if (_Model != NULL)
  {
    _Model->Serialize(Density,
                      NumPoints,
                      NumDimensions,
                      Instances,
                      NeighbourhoodSizes,
                      DimValues);
  }
}

HullModel* HullModel::Merge (HullModel* Other, double Epsilon, int MinPoints)
{
  ConvexHullModel *ThisModel, *OtherModel, *MergedModel;

  if (_Model == NULL)
    return NULL;

  ThisModel = _Model;

  OtherModel = Other->Model();

  if (Other->Model() == NULL)
    return NULL;

  MergedModel = ThisModel->Merge(OtherModel, Epsilon, MinPoints);

  if (MergedModel == NULL)
    return NULL;

  return new HullModel(MergedModel);
}

ConvexHullModel* const HullModel::Model(void)
{
  return _Model;
}

int  HullModel::Size(void)
{
  if (_Model == NULL)
    return -1;
  else
    return _Model->size();
}

int  HullModel::Density(void)
{
  if (_Model == NULL)
    return -1;
  else
    return _Model->GetDensity();
}

void HullModel::Flush(void)
{

  if (_Model != NULL)
  {
    _Model->Flush();
  }
}

bool HullModel::Flush(ostream&             str,
                      cluster_id_t         id)
{
  if (_Model == NULL)
  {
    return false;
  }
  else
  {
    return _Model->Flush(str, id);
  }
}

