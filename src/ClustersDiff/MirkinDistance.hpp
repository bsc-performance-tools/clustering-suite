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

#ifndef _MIRKINDISTANCE_HPP_
#define _MIRKINDISTANCE_HPP_

#include <clustering_types.h>
#include <Error.hpp>
using cepba_tools::Error;

#include <string>
using std::string;

#include <fstream>
using std::ifstream;

#include <vector>
using std::vector;

#include <map>
using std::map;

#include <set>
using std::set;

class MirkinDistance: public Error
{
  string   ClustersFileName1;
  ifstream ClustersFile1;

  string   ClustersFileName2;
  ifstream ClustersFile2;

  public:

    bool GetMirkinDistance(string  ClustersFileName1,
                           string  ClustersFileName2,
                           bool&   UseNoise,
                           double& Distance);
  private:

    bool GenerateSets(map<cluster_id_t, vector<instance_t> >& ClustersSet1,
                      map<cluster_id_t, vector<instance_t> >& ClustersSet2);

    void PopulateRecord(vector<string> &Record,
                        const string   &Line,
                        char            Delimiter);

    bool CheckHeader(vector<string> &Record,
                     string         &CurrentFileName);

};



#endif
