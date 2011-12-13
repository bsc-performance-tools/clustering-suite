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

#ifndef _CLUSTERINGALGORITHM_HPP_
#define _CLUSTERINGALGORITHM_HPP_

#include <Error.hpp>
using cepba_tools::Error;

#include "clustering_types.h"

/* Forward declarations */
#include "Classifier.hpp"

class Point;
class Partition;

#include <sys/stat.h>

#include <cstdio>
#include <string>
#include <cerrno>
#include <cmath>
#include <cfloat>

#include <vector>
using std::vector;

#include <list>
using std::list;

#include <map>
using std::map;

#include <iostream>
using std::ostream;
using std::cout;
using std::endl;

class ClusteringAlgorithm: public Error
{
  protected:

    bool   ClusteringReady;
    bool   Distributed;

  public:

    virtual ~ClusteringAlgorithm(void) { };

    bool   IsClusteringReady(void) { return ClusteringReady; };

    virtual bool Run(const vector<const Point*>& Data,
                     Partition&                  DataPartition,
                     bool                        SimpleRun = false) = 0;

    // virtual Classifier* GetClassifier(void) {}; //

    virtual string GetClusteringAlgorithmName(void) const = 0;

    virtual string GetClusteringAlgorithmNameFile(void) const = 0;

    virtual bool IsDistributed(void) { return false; };

    virtual bool HasNoise(void)      { return false; };

    virtual bool ParametersApproximation(const vector<const Point*>& Data,
                                         map<string, string>&  Parameters,
                                         string                OutputFileNamePrefix)
    {
      SetErrorMessage("this algorithm does not have a parameter approximation defined");
      return false;
    };

    virtual bool Classify(const vector<const Point*> &Data,
                          Partition                  &DataPartition)
    {
      SetErrorMessage("this algorithm does not have support to generate data classifier");
      SetError(true);
      return false;
    }

    virtual bool Classify(const Point* Point, cluster_id_t& ID)
    {
      SetErrorMessage("this algorithm does not have support to generate data classifier");
      SetError(true);
      return false;
    }

};

#endif /* _CLUSTERINGALGORITHM_HPP_ */
