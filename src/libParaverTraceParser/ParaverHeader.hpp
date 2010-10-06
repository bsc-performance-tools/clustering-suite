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

#ifndef _PARAVERHEADER_H
#define _PARAVERHEADER_H

#include <types.h>

#include "ParaverApplicationDescription.hpp"
#include "Error.hpp"
using cepba_tools::Error;

#include <string>
using std::string;
#include <vector>
using std::vector;



#define MICROSECONDS 0
#define NANOSECONDS  1

class ParaverHeader: public Error
{
  private:
    char*  ASCIIHeader;
  
    UINT64 FinalTime;
    INT32  TimeUnits;
    
    INT32         ResourceNumber;
    bool          ResourceDescriptionPresent;
    vector<INT32> ResourceDescription;
  
    INT32  AppNumber;
    vector<ApplicationDescription_t> AppsDescription;
  
  public:
    ParaverHeader(char* ASCIIHeader, INT32 HeaderLength);
  
    vector<ApplicationDescription*> GetAppsDescription(void)
    {
      return AppsDescription;
    };
    
    UINT64 GetFinalTime(void) { return FinalTime; };
    INT32  GetTimeUnits(void) { return TimeUnits; };
    
    bool Flush(FILE* OutputFile);
  
  private:
    
    bool ProcessFinalTime       (char* ASCIIFinalTime);
    bool ProcessResourceList    (char* ASCIIRsrcList);
    bool ProcessApplicationList (char* ASCIIAppList);
};
typedef ParaverHeader* ParaverHeader_t;

#endif /* _PARAVERHEADER_H */
