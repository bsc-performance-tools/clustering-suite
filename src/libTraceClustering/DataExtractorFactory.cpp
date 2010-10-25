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

#include "DataExtractorFactory.hpp"
#include "PRVDataExtractor.hpp"
#include "TRFDataExtractor.hpp"

#include <sstream>
using std::ostringstream;

#include <cstring>
#include <cerrno>

DataExtractorFactory* DataExtractorFactory::Instance = NULL;

DataExtractorFactory::DataExtractorFactory(void)
{
  this->FileType = UndefinedInputFile;
}

DataExtractorFactory* DataExtractorFactory::GetInstance(void)
{
  if (DataExtractorFactory::Instance == NULL)
  {
    DataExtractorFactory::Instance = new DataExtractorFactory();
  }
  
  return (DataExtractorFactory::Instance);
}

bool DataExtractorFactory::GetExtractor(string          InputFileName,
                                        DataExtractor *&DataExtractorObject)
{
  if (!CheckFileType(InputFileName))
  {
    return false;
  }

  switch(FileType)
  {
    case ParaverTrace:
      DataExtractorObject = new PRVDataExtractor(InputFileName);
      break;
    case DimemasTrace:
      DataExtractorObject = new TRFDataExtractor(InputFileName);
      break;
    case ClusteringCSV:
      SetError(true);
      SetErrorMessage("CSV input files not supported yet");
      return false;
    default:
      SetError(true);
      SetErrorMessage("undefined error while manipulating input file");
      return false;
  }

  return true;
}

bool DataExtractorFactory::CheckFileType(string InputTraceName)
{
  FILE* InputTraceFile;
  char  Magic[10];
  
  if ( (InputTraceFile = fopen(InputTraceName.c_str(), "r")) == NULL)
  {
    ostringstream ErrorMessage;
    
    ErrorMessage << "unable to open input file \"" << InputTraceName << "\"";
    SetErrorMessage(ErrorMessage.str());

    return false;
  }
  
  if (fread(Magic, sizeof(char), 8, InputTraceFile) != 8)
  {
    ostringstream ErrorMessage;
    
    ErrorMessage << "error reading input file (" << strerror(errno) << ")";
    SetErrorMessage(ErrorMessage.str());

    return false;
  }
  
  if (strncmp(Magic, "#Paraver", 8) == 0)
  {
    FileType = ParaverTrace;
    return true;
  }
  
  if (strncmp(Magic, "\nSDDFA;;", 8) == 0 ||
      strncmp(Magic, "SDDFA;;", 7) == 0)
  {
    FileType = DimemasTrace;
    return true;
  }

  if (strncmp(Magic, "#ClusteringCSV", 14) == 0)
  {
    FileType = ClusteringCSV;
    return true;
  }
  
  SetErrorMessage("unable to detect input file type");
  return false;
}
