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

  $URL::                                          $:  File
  $Rev::                                          $:  Revision of last commit
  $Author::                                       $:  Author of last commit
  $Date::                                         $:  Date of last commit

\* -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=- */

#include "FileNameManipulator.hpp"
using cepba_tools::FileNameManipulator;

FileNameManipulator::FileNameManipulator(string OriginalFileName,
                                         string Extension)
{
  this->OriginalFileName = OriginalFileName;
  this->Extension        = Extension;
  
  string::size_type SubstrPosition;

  SubstrPosition = OriginalFileName.rfind("."+Extension);

  if (SubstrPosition == string::npos)
  {
    ChoppedFileName = OriginalFileName;
  }
  else
  {
    ChoppedFileName = OriginalFileName.substr(0, SubstrPosition);
  }
}

string FileNameManipulator::AppendString(string Append)
{
  return (ChoppedFileName+"."+Append+"."+Extension);
}

string FileNameManipulator::AppendStringAndExtension(string Append, string Extension)
{
  return (ChoppedFileName+"."+Append+"."+Extension);
}

string FileNameManipulator::GetChoppedFileName(void)
{
  return this->ChoppedFileName;
}

string FileNameManipulator::GetExtension(string FileName)
{
  size_t LastPointPosition;

  LastPointPosition = FileName.find_last_of(".");

  if (LastPointPosition == string::npos)
  {
    return string("");
  }
  else
  {
    return FileName.substr(LastPointPosition+1);
  }
}
