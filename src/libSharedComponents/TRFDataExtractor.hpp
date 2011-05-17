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

#ifndef TRFDATAEXTRACTOR_H
#define TRFDATAEXTRACTOR_H

#include <trace_clustering_types.h>

#include "DataExtractor.hpp"
#include "TraceData.hpp"

#include <string>
#include <cmath>
#include <cstdio>


class TRFDataExtractor: public DataExtractor
{
  private:
    off_t  InputTraceFileSize;
  
  public:
    TRFDataExtractor(string InputTraceName);

    ~TRFDataExtractor();

    bool SetEventsToDealWith(set<event_type_t>& EventsToDealWith);
    
    bool ExtractData(TraceData* TraceDataSet);

    input_file_t GetFileType(void) { return DimemasTrace; };
    
  private:

    bool NormalizeData(void);
    
    INT32 GetInputTraceFilePercentage(void)
    {
      off_t CurrentPosition = ftello(InputTraceFile);
      return (INT32) (lround (100.0*CurrentPosition/InputTraceFileSize));
    };
};

typedef TRFDataExtractor* TRFDataExtractor_t;

#endif /* TRACEDATAEXTRACTOR_H */
