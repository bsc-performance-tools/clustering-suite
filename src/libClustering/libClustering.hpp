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


#ifndef _LIBCLUSTERING_HPP_
#define _LIBCLUSTERING_HPP_

#include <string>
using std::string;
#include <map>
using std::map;

#include "clustering_types.h"
#include "Point.hpp"
#include "Partition.hpp"

class libClusteringImplementation;

class libClustering
{
  private:
    libClusteringImplementation* Implementation;
    
    bool   Error,        Warning;
    string ErrorMessage, WarningMessage;

  public:
    
    libClustering(void);

    bool InitClustering(string AlgorithmName, map<string, string> Parameters);

    bool ExecuteClustering(const vector<const Point*>& Data, Partition& DataPartition);

    bool UsingADistributedAlgorithm(void);

    string GetClusteringAlgorithmName(void);

    string GetErrorMessage(void);

    string GetWarningMessage(void);
    
  protected:

};

#endif // _LIBCLUSTERING_HPP_
