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

  $Id:: ParaverApplicationDescription.cpp 23 2011#$:  Id
  $Rev:: 23                                       $:  Revision of last commit
  $Author:: jgonzale                              $:  Author of last commit
  $Date:: 2011-05-17 11:47:12 +0200 (Tue, 17 May #$:  Date of last commit

\* -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=- */

#include "ParaverMetadataManager.hpp"
#include "Utilities.hpp"

#include <sstream>
using std::ostringstream;
using std::istringstream;
using std::stringstream;
#include <algorithm>
using std::sort;

int Metadata::FIELD_COUNT = 4;

Metadata::Metadata(string Date,
                   string Action,
                   string Application,
                   string OriginalTrace):
Date(Date), Action(Action), Application(Application), OriginalTrace(OriginalTrace)
{
}

void Metadata::Write(ostream& os) const
{
  os << "#" << Date << ":" << Action << ":" << Application << ":" << OriginalTrace << ":";

  FlushSpecificFields(os);
}

ostream& operator<< (ostream& os, const Metadata& MetadataRecord)
{
  MetadataRecord.Write(os);
}

int    CutterMetadata::FIELD_COUNT = Metadata::FIELD_COUNT + 3;
string CutterMetadata::ACTION_ID   = "CUTTER";
string CutterMetadata::RUNAPP_APPLICATION_ID = "RUNAPP";
string CutterMetadata::ORIGINAL_APPLICATION_ID = "ORIGINAL";


CutterMetadata::CutterMetadata (vector<string>& CutterMetadataFields)
{
  istringstream Converter;

  if (CutterMetadataFields.size() != CutterMetadata::FIELD_COUNT)
  {
    ostringstream ErrorMessageSStr;
    ErrorMessageSStr <<  "wrong number of fields value in cutter metadata record (";
    ErrorMessageSStr <<  CutterMetadataFields.size() << " read, ";
    ErrorMessageSStr << CutterMetadata::FIELD_COUNT << " expected)";

    Error = true;
    ErrorMessage = ErrorMessageSStr.str();
  }

  this->Date          = CutterMetadataFields[0];
  this->Action        = CutterMetadataFields[1];
  this->Application   = CutterMetadataFields[2];
  this->OriginalTrace = CutterMetadataFields[3];


  /* FIELD 5: Offset */
  Converter.clear();
  Converter.str(CutterMetadataFields[4]);
  if (!(Converter >> this->Offset))
  {
    ostringstream ErrorMessageSStr;
    ErrorMessageSStr <<  "wrong offset value in cutter metadata record (";
    ErrorMessageSStr <<  CutterMetadataFields[4] << ")";

    Error = true;
    ErrorMessage = ErrorMessageSStr.str();

    return;
  }


  /* FIELD 6: BeginTime */
  Converter.clear();
  Converter.str(CutterMetadataFields[5]);
  if (!(Converter >> this->BeginTime))
  {
    ostringstream ErrorMessageSStr;
    ErrorMessageSStr <<  "wrong cut begin time value in cutter metadata record (";
    ErrorMessageSStr <<  CutterMetadataFields[5] << ")";

    Error = true;
    ErrorMessage = ErrorMessageSStr.str();

    return;
  }

  /* FIELD 7: EndTime */
  Converter.clear();
  Converter.str(CutterMetadataFields[6]);
  if (!(Converter >> this->EndTime))
  {
    ostringstream ErrorMessageSStr;
    ErrorMessageSStr <<  "wrong cut end time value in cutter metadata record (";
    ErrorMessageSStr <<  CutterMetadataFields[6] << ")";

    Error = true;
    ErrorMessage = ErrorMessageSStr.str();

    return;
  }
}

/*
CutterMetadata::CutterMetadata (string Date,
                                string Application,
                                string OriginalTrace,
                                UINT64 Offset,
                                UINT64 BeginTime,
                                UINT64 EndTime)
:Metadata(Date, CutterMetadata::ACTION_ID, Application, OriginalTrace)
{
  this->Offset    = Offset;
  this->BeginTime = BeginTime;
  this->EndTime   = EndTime;
}
*/



void CutterMetadata::FlushSpecificFields(ostream& os) const
{
  os << Offset << ":" << BeginTime << ":" << EndTime;
}

bool ParaverMetadataManager::NewMetadata(string MetadataStr)
{
  vector<string> MetadataFields;

  PopulateRecord(MetadataFields, MetadataStr, ':');

  if (MetadataFields.size() < Metadata::FIELD_COUNT)
  {
    ostringstream ErrorMessageSStr;
    Error = true;

    ErrorMessageSStr << "wrong metadata field (" << MetadataFields.size() << " ";
    ErrorMessageSStr << "fields read, " << Metadata::FIELD_COUNT << " minimum)";
    ErrorMessage = ErrorMessageSStr.str();
    return false;
  }

  /*
   * Metadata Common Fields
   * FIELD 0: Date
   * FIELD 1: Action
   * FIELD 2: Application
   * FIELD 3: Original Trace
   */

  if (MetadataFields[1].compare(CutterMetadata::ACTION_ID) == 0)
  {
    CutterMetadata* NewCutterMetadata = new CutterMetadata(MetadataFields);

    if (NewCutterMetadata->GetError())
    {
      Error = true;
      ErrorMessage = NewCutterMetadata->GetErrorMessage();

      delete NewCutterMetadata;

      return false;
    }

    TraceMetadataStorage.push_back(NewCutterMetadata);
    CutterMetadataStorage.push_back(NewCutterMetadata);
  }

  return true;
}

vector<Metadata*>& ParaverMetadataManager::GetMetadata(void)
{
  sort(TraceMetadataStorage.rbegin(), TraceMetadataStorage.rend());
  return TraceMetadataStorage;
}

vector<CutterMetadata*>& ParaverMetadataManager::GetCutterMetadata(void)
{
  sort(CutterMetadataStorage.rbegin(), CutterMetadataStorage.rend());
  return CutterMetadataStorage;
}

string ParaverMetadataManager::GetCurrentDate()
{
  stringstream currentDate;

  time_t t = time(0);   // get time now
  struct tm * now = localtime( & t );
  currentDate << (now->tm_year + 1900)
              << (now->tm_mon + 1)
              << now->tm_mday
              << now->tm_hour
              << now->tm_min
              << now->tm_sec;


  return currentDate.str();
}

void ParaverMetadataManager::PopulateRecord(vector<string> &Record,
                                            const string   &Line,
                                            char            Delimiter)
{
  int linepos  = 0;
  int inquotes = false;
  char c;
  int i;
  int linemax = Line.length();
  string curstring;
  Record.clear();

  while(Line[linepos]!=0 && linepos < linemax)
  {
    c = Line[linepos];

    if (!inquotes && curstring.length()==0 && c=='"')
    {
      //beginquotechar
      inquotes=true;
    }
    else if (inquotes && c=='"')
    {
      //quotechar
      if ( (linepos+1 <linemax) && (Line[linepos+1]=='"') )
      {
        //encountered 2 double quotes in a row (resolves to 1 double quote)
        curstring.push_back(c);
        linepos++;
      }
      else
      {
        //endquotechar
        inquotes = false;
      }
    }
    else if (!inquotes && c == Delimiter)
    {
      //end of field

      Record.push_back( cepba_tools::trim(curstring) );
      curstring="";
    }
    else if (!inquotes && (c=='\r' || c=='\n') )
    {
      Record.push_back( cepba_tools::trim(curstring) );
      return;
    }
    else
    {
      curstring.push_back(c);
    }
    linepos++;
  }

  Record.push_back( cepba_tools::trim(curstring) );
  return;
}
