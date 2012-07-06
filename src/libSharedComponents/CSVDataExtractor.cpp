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

  $Id::                                     $:  Id
  $Rev::                                          $:  Revision of last commit
  $Author::                                       $:  Author of last commit
  $Date::                                         $:  Date of last commit

\* -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=- */

#include <SystemMessages.hpp>
using cepba_tools::system_messages;

#include <Utilities.hpp>

#include "CSVDataExtractor.hpp"
#include <CPUBurst.hpp>
#include <TraceData.hpp>

#include <cstring>
#include <cerrno>

#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

#include <iostream>
using std::cout;
using std::cerr;
using std::endl;

#include <sstream>
using std::ostringstream;
using std::istringstream;

CSVDataExtractor::CSVDataExtractor(string CSVFileName)
:DataExtractor(CSVFileName)
{
  ostringstream  ErrorMessage;
  vector<string> Header;
  string         Line;

  this->CSVFileName = CSVFileName;

  if (GetError())
    return;

  CSVFile.open(CSVFileName.c_str(), ifstream::in);

  if (CSVFile.fail())
  {
    ErrorMessage << "Unable to open file " << CSVFileName;
    SetError(true);
    SetErrorMessage(ErrorMessage.str().c_str(),
                    strerror(errno));
    return;
  }

  getline(CSVFile, Line);
  if (!CSVFile.good())
  {
    ErrorMessage << "Error reading header of file " << CSVFileName;
    SetError(true);
    SetErrorMessage(ErrorMessage.str().c_str(),
                    strerror(errno));
    return;
  }

  PopulateRecord(Header, Line, ',');
  if (!ParseHeader(Header))
    return;

  FirstPos = CSVFile.tellg();
  CSVFile.seekg(0, ifstream::end);
  EndPos = CSVFile.tellg();
  CSVFile.seekg(FirstPos);

  return;
}


CSVDataExtractor::~CSVDataExtractor()
{
  // unlink(TraceDataFileName.c_str());
}

bool CSVDataExtractor::SetEventsToDealWith(set<event_type_t>& EventsToDealWith)
{
  SetError(true);
  SetErrorMessage("CSV files do not permit parsing based on events");

  return false;
}


bool CSVDataExtractor::ExtractData(TraceData* TraceDataSet)
{
  ostringstream  ErrorMessage;
  vector<string> Record;
  string         Line;

  system_messages::show_percentage_progress("Loading file "+CSVFileName,
                                            0);

  CurrentLine = 1;
  while(getline(CSVFile, Line)  && CSVFile.good())
  {
    Record.clear();

    PopulateRecord (Record, Line, ',');
    ParseRecord (Record, TraceDataSet);

    int Percentage = static_cast<int>(CSVFile.tellg()) * 100 / EndPos;

    system_messages::show_percentage_progress("Loading file '"+CSVFileName+"'",
                                              Percentage);

    CurrentLine++;
  }

  if (!CSVFile.eof())
  {
    ErrorMessage << "Error while loading " << CSVFileName;
    SetError(true);
    SetErrorMessage(ErrorMessage.str().c_str(),
                    strerror(errno));
    return false;
  }

  system_messages::show_percentage_end("Loading file '"+CSVFileName+"'");

  return true;
}

bool CSVDataExtractor::GetPartition(Partition& ReadPartition)
{
  /* Input file doesn't contain cluster information at all */
  if (IDs.size() == 0)
  {
    return false;
  }

  ReadPartition.clear();

  ReadPartition.SetAssignmentVector(AssignmentVector);
  ReadPartition.SetIDs(IDs);

  return true;
}

/**
 * Loads into a string vector 'Record' the contents of the CSV string 'Line'
 * separated by the token 'Delimiter'
 *
 * \param Record    Output vector where the tokens of the CSV line will be
 *                  stored
 * \param Line      The line that constains a CSV row
 * \param Delimiter Character that separates the fields in the line
 *
 */
void CSVDataExtractor::PopulateRecord(vector<string> &Record,
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

/**
 * Parses the 'Record' that contains the header fields read from the CSV file.
 * Fills the different attributes with the names of the clustering/extrapolation
 * parameters
 *
 * \param  Record Vector containing the strings present in the CSV file header
 *
 * \result True if the header is correct, false otherwise
 *
 */
bool CSVDataExtractor::ParseHeader(vector<string> &Record)
{
  ostringstream ErrorMessage;
  bool Clustering, Normalized;

  if (Record.size() < 7)
  {
    ErrorMessage << "wrong number of fields in the file";

    SetError(true);
    SetErrorMessage(ErrorMessage.str());
    return false;
  }

  if (Record[0].compare(INSTANCE_CSV) != 0)
  {
    ErrorMessage << "wrong field '" << Record[0] << "' (Expected " << INSTANCE_CSV << ")";

    SetError(true);
    SetErrorMessage(ErrorMessage.str());
    return false;
  }

  if (Record[1].compare(TASKID_CSV) != 0)
  {
    ErrorMessage << "wrong field '" << Record[1] << "' (Expected " << TASKID_CSV << ")";

    SetError(true);
    SetErrorMessage(ErrorMessage.str());
    return false;
  }

  if (Record[2].compare(THREADID_CSV) != 0)
  {
    ErrorMessage << "wrong field '" << Record[2] << "' (Expected " << THREADID_CSV << ")";

    SetError(true);
    SetErrorMessage(ErrorMessage.str());
    return false;
  }

  if (Record[3].compare(BEGINTIME_CSV) != 0)
  {
    ErrorMessage << "wrong field '" << Record[3] << "' (Expected " << BEGINTIME_CSV << ")";

    SetError(true);
    SetErrorMessage(ErrorMessage.str());
    return false;
  }

  if (Record[4].compare(ENDTIME_CSV) != 0)
  {
    ErrorMessage << "wrong field '" << Record[4] << "' (Expected " << ENDTIME_CSV << ")";

    SetError(true);
    SetErrorMessage(ErrorMessage.str());
    return false;
  }

  if (Record[5].compare(DURATION_CSV) != 0)
  {
    ErrorMessage << "wrong field '" << Record[5] << "' (Expected " << DURATION_CSV << ")";

    SetError(true);
    SetErrorMessage(ErrorMessage.str());
    return false;
  }

  if (Record[6].compare(LINE_CSV) != 0)
  {
    ErrorMessage << "wrong field '" << Record[6] << "' (Expected " << LINE_CSV << ")";

    SetError(true);
    SetErrorMessage(ErrorMessage.str());
    return false;
  }

  if (Record[Record.size()-1].compare(CLUSTERID_CSV) != 0)
  {
    ErrorMessage << "wrong field '" << Record[Record.size()-1];
    ErrorMessage << "' (Expected " << CLUSTERID_CSV << ")";

    SetError(true);
    SetErrorMessage(ErrorMessage.str());
    return false;
  }

  Clustering = true;
  Normalized = false;
  for (vector<string>::size_type i = 7;
       i < (Record.size()-1);
       i++)
  {
    if (Clustering)
    {
      if (Record[i].find("_Norm") != string::npos)
      {
        Normalized = true;
        Clustering = false;
        NormalizedParameters.push_back(Record[i]);
      }
      else
      {
        ClusteringParameters.push_back(Record[i]);
      }
    }
    else if (Normalized)
    {
      if (Record[i].find("_Norm") != string::npos)
      {
        NormalizedParameters.push_back(Record[i]);
      }
      else
      {
        Normalized = false;
        ExtrapolationParameters.push_back(Record[i]);
      }
    }
    else
    {
      ExtrapolationParameters.push_back(Record[i]);
    }
  }

  /* DEBUG
  cout << "**** Clustering Parameters ****" << endl;
  for (vector<string>::size_type i = 0;
       i < ClusteringParameters.size();
       i++)
  {
    cout << ClusteringParameters[i] << " ";
  }
  cout << endl;

  cout << "**** Clustering Parameters (Normalized) ****" << endl;
  for (vector<string>::size_type i = 0;
       i < NormalizedParameters.size();
       i++)
  {
    cout << NormalizedParameters[i] << " ";
  }
  cout << endl;

  cout << "**** Extrapolation Parameters ****" << endl;
  for (vector<string>::size_type i = 0;
       i < ExtrapolationParameters.size();
       i++)
  {
    cout << ExtrapolationParameters[i] << " ";
  }
  cout << endl;
  */

  RecordSize = Record.size();

  return true;
}

/**
 * Parses a 'Record' that contains a burst information from the CSV file.
 * If the information is correct, fills inserts the new burst information in
 * the data set
 *
 * \param  Record       Vector containing the strings present in the CSV file
 * \param  TraceDataSet Container of the 'CPUBurst' collection
 *
 * \result True if the records has been added to the data set corrrectly, false
 *         otherwise
 *
 */
bool CSVDataExtractor::ParseRecord(vector<string> &Record,
                                   TraceData* TraceDataSet)
{
  ostringstream       ErrorMessage;
  instance_t          Instance;
  task_id_t           TaskId;
  thread_id_t         ThreadId;
  line_t              Line;
  timestamp_t         BeginTime;
  timestamp_t         EndTime;
  duration_t          BurstDuration;
  vector<double>      ClusteringRawData;
  vector<double>      ClusteringProcessedData;
  map<size_t, double> ExtrapolationData;
  burst_type_t        BurstType;
  cluster_id_t        ClusterId;

  if (Record.size() != RecordSize)
  {
    ErrorMessage << "wrong number of fields (" << Record.size();
    ErrorMessage << " found, " << RecordSize << "expected) in record on line ";
    ErrorMessage << CurrentLine;

    SetError(true);
    SetErrorMessage(ErrorMessage.str());
    return false;
  }

  istringstream(Record[0]) >> Instance;
  istringstream(Record[1]) >> TaskId;
  istringstream(Record[2]) >> ThreadId;
  istringstream(Record[3]) >> BeginTime;
  istringstream(Record[4]) >> EndTime;
  istringstream(Record[5]) >> BurstDuration;
  istringstream(Record[6]) >> Line;

  for (vector<string>::size_type i = 0; i < ClusteringParameters.size(); i++)
  {
    double CurrentValue;

    vector<string>::size_type Offset = CSV_HEADING_FIELDS;


    istringstream(Record[Offset+i]) >> CurrentValue;

    ClusteringRawData.push_back(CurrentValue);
  }

  for (vector<string>::size_type i = 0; i < NormalizedParameters.size(); i++)
  {
    double CurrentValue;

    vector<string>::size_type Offset =
      CSV_HEADING_FIELDS + ClusteringParameters.size();

    istringstream(Record[Offset+i]) >> CurrentValue;

    ClusteringProcessedData.push_back(CurrentValue);
  }

  for (vector<string>::size_type i = 0; i < ExtrapolationParameters.size(); i++)
  {
    double CurrentValue;

    vector<string>::size_type Offset =
      CSV_HEADING_FIELDS + ClusteringParameters.size() + NormalizedParameters.size();

    if (Record[Offset+i].compare ("nan") != 0)
    {
      istringstream(Record[Offset+i]) >> CurrentValue;

      ExtrapolationData[i] = CurrentValue;
    }
  }

  istringstream(Record[Record.size()-1]) >> ClusterId;

  ClusterId -= PARAVER_OFFSET;

  switch(ClusterId)
  {
    case MISSING_DATA_CLUSTERID:
      BurstType = MissingDataBurst;
      break;

    case DURATION_FILTERED_CLUSTERID:
      BurstType = DurationFilteredBurst;
      break;

    case RANGE_FILTERED_CLUSTERID:
      BurstType = RangeFilteredBurst;
      break;

    case THRESHOLD_FILTERED_CLUSTERID:
      BurstType = RangeFilteredBurst;
      break;

    case UNCLASSIFIED:
      BurstType = CompleteBurst;
      break;

    default:
      BurstType = CompleteBurst;
      AssignmentVector.push_back(ClusterId);
      IDs.insert(ClusterId);
      break;
  }

  /* DEBUG
  cout << "Line = " << CurrentLine << " Instance = " << Instance;
  cout << " BurstType = " << CPUBurst::BurstTypeStr(BurstType);
  cout << " ClusterID = " << ClusterId << endl;
  */

  if (!TraceDataSet->NewBurst(Instance,
                              TaskId,
                              ThreadId,
                              Line,
                              BeginTime,
                              EndTime,
                              BurstDuration,
                              ClusteringRawData,
                              ClusteringProcessedData,
                              ExtrapolationData,
                              BurstType))
  {
    SetError(true);
    SetErrorMessage(TraceDataSet->GetErrorMessage());
    return false;
  }
  return true;
}
