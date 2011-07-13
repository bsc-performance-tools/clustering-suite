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

#ifndef _CLUSTEREDTRACEGENERATOR_H
#define _CLUSTEREDTRACEGENERATOR_H

#include "trace_clustering_types.h"
#include <Error.hpp>
using cepba_tools::Error;

#include <CPUBurst.hpp>

#include <set>
using std::set;

class ClusteredTraceGenerator: public Error
{
  protected:
    string InputTraceName;
    FILE*  InputTraceFile;
    string OutputTraceName;
    FILE*  OutputTraceFile;
    bool   DestroyClusteredFile;
  
  public:
    ClusteredTraceGenerator(string  InputTraceName,
                            string  OutputTraceName);
  
    virtual bool Run(vector<CPUBurst*>&    Bursts,
                     vector<cluster_id_t>& IDs,
                     set<cluster_id_t>&    DifferentIDs,
                     bool                  MinimizeInformation = false) = 0;

    virtual bool SetEventsToDealWith(set<event_type_t>& EventsToDealWith) = 0;
};

#endif
