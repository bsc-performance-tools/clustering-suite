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

  $Id:: PRVStatesDataExtractor.cpp 87 2013-05-08 #$:  Id
  $Rev:: 87                                       $:  Revision of last commit
  $Author:: jgonzale                              $:  Author of last commit
  $Date:: 2013-05-08 18:30:09 +0200 (mié, 08 may#$:  Date of last commit

\* -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=- */

#include <SystemMessages.hpp>
using cepba_tools::system_messages;

#include "PRVSemanticGuidedDataExtractor.hpp"
#include "ParaverTraceParser.hpp"

#include <Utilities.hpp>

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

// #define DEBUG_PARAVER_INPUT 1

/******************************************************************************
 * Class 'BurstContainter'
 ******************************************************************************/
string PRVSemanticGuidedDataExtractor::BurstContainer::toString(void)
{
  ostringstream Result;

  Result << "[" << TaskId << "|" << ThreadId << "] (" << Line << ") ";
  Result << "Begin: " << BeginTime << " End: " << EndTime << " (Dur:";
  Result << Duration << ")" << endl;

  Result << "*** EventsData *** " << endl;
  for (map<event_type_t, event_value_t>::iterator it  = EventsData.begin();
                                                  it != EventsData.end();
                                                ++it)
  {
    Result << "[ " << it->first << " ] = " << it->second << endl;
  }

  Result << "*** BurstEndEvents ***" << endl;
  Result << "[ ";
  for (set<event_type_t>::iterator it  = BurstEndEvents.begin();
                                   it != BurstEndEvents.end();
                                 ++it)
  {
    Result << (*it) <<  " ";
  }
  Result << "]";

  return Result.str();
}


/******************************************************************************
 * Class 'PRVSemanticGuidedDataExtractor'
 ******************************************************************************/
PRVSemanticGuidedDataExtractor::PRVSemanticGuidedDataExtractor(string InputTraceName,
                                                               string InputSemanticCSV)
:DataExtractor(InputTraceName)
{
  if (GetError())
    return;

  this->InputSemanticCSV = InputSemanticCSV;

  /* Check Semantic CSV accesibility */
  SemanticCSVStream.open(InputSemanticCSV.c_str(), ifstream::in);
  if (SemanticCSVStream.fail())
  {
    ostringstream ErrorMessage;
    ErrorMessage << "Unable to open semnantic CSV file " << InputSemanticCSV;
    SetError(true);
    SetErrorMessage(ErrorMessage.str().c_str(),
                    strerror(errno));
    return;
  }

  TraceParser = new ParaverTraceParser(InputTraceName, InputTraceFile);

  if (!TraceParser->InitTraceParsing())
  {
    SetError(true);
    SetErrorMessage("error init tracing", TraceParser->GetLastError());
    return;
  }

  if (TraceParser->GetTimeUnits() == MICROSECONDS)
    TimeFactor = 1e3;
  else
    TimeFactor = 1;

  OneThreadPerTask = true;

  return;
}

PRVSemanticGuidedDataExtractor::~PRVSemanticGuidedDataExtractor()
{
  for (map<string, list<BurstContainer*> >::iterator It = BurstsToLoad.begin();
       It != BurstsToLoad.end();
       ++It)
  {
    while (!It->second.empty())
    {
      BurstContainer *Burst = It->second.front();
      It->second.pop_front();
      delete Burst;
    }
  }
}

bool PRVSemanticGuidedDataExtractor::SetEventsToDealWith (set<event_type_t>& EventsToDealWith,
                                                          bool               ConsecutiveEvts)
{
  SetError(true);
  SetErrorMessage("trying to use a state guided parser to an event guided parsing");

  return false;
}

bool PRVSemanticGuidedDataExtractor::ExtractData(TraceData* TraceDataSet)
{
  vector<ApplicationDescription_t> AppsDescription;
  vector<TaskDescription_t>        TaskInfo;
  ParaverMetadataManager*          MetadataManager;
  vector<CutterMetadata*>          CutterMetadataRecords;
  timestamp_t                      CutOffset    = 0;
  size_t                           TraceObjects = 0;

  // ParaverRecord *CurrentRecord;
  // State         *CurrentState;
  Event         *CurrentEvent;

  INT32  CurrentPercentage = 0;


  /*
  if (InputDataManager == NULL)
  {
    SetError(true);
    SetErrorMessage("data manager must be defined to extract data");
    return false;
  }
  */

  /* Create the structure to manage the different bursts foreach task/thread */
  AppsDescription = TraceParser->GetApplicationsDescription();

  if (TraceParser->GetError())
  {
    SetError(true);
    SetErrorMessage("unable to start data extraction",
                    TraceParser->GetLastError());
    return false;
  }

  if (AppsDescription.size() != 1)
  {
    /* DEBUG
    cout << "Application description size is " << AppsDescription.size() << endl;
    */

    SetError(true);
    SetErrorMessage("unable to apply cluster analysis on a trace with more than one application");
    return false;
  }

  TaskInfo = AppsDescription[0]->GetTaskInfo();

  /* Set the number of tasks on trace data, to perform the possible data distribution */
  TraceDataSet->SetNumberOfTasks(TaskInfo.size());

  for (INT32 i = 0; i < TaskInfo.size(); i++)
  {
    INT32 Threads;

    if ( (Threads = TaskInfo[i]->GetThreadCount()) > 1)
    {
      OneThreadPerTask = false;
    }

    TraceObjects += Threads;
  }

  TraceDataSet->SetTraceObjects(TraceObjects);

/* DEBUG
  cout << "Ready to process Semantic CSV!" << endl;
  */
  MetadataManager       = TraceParser->GetMetadata();
  CutterMetadataRecords = MetadataManager->GetCutterMetadata();

  if (CutterMetadataRecords.size() != 0)
  {
    CutterMetadata* FirstCutterMetadata = CutterMetadataRecords[0];

    if (FirstCutterMetadata->GetApplication().compare(CutterMetadata::RUNAPP_APPLICATION_ID) == 0)
    {
      CutOffset = FirstCutterMetadata->GetOffset();
    }
  }

  /* DEBUG
  cout << "Cut Offset = " << CutOffset << endl;
  */

  if (!ProcessSemanticCSV(CutOffset))
  {
    return false;
  }

  /* DEBUG
  for (map<string, list<BurstContainer*> >::iterator it = BurstsToLoad.begin();
       it != BurstsToLoad.end();
       ++it)
  {
    cout << "[" << it->first << "]: " << it->second.size() << " intervals read" << endl;
  }
  */

  CurrentPercentage = TraceParser->GetFilePercentage();

  system_messages::show_percentage_progress("Parsing Paraver Input Trace",
                                            CurrentPercentage);

  while (true)
  {
    INT32 PercentageRead;

    // CurrentRecord = TraceParser->GetNextRecord(STATE_REC|EVENT_REC);
    CurrentEvent = TraceParser->GetNextEvent();

    if (CurrentEvent == NULL)
      break;

    if (!ProcessEvent(CurrentEvent, TraceDataSet))
    {
      return false;
    }

    /* Show progress */
    PercentageRead = TraceParser->GetFilePercentage();
    if (PercentageRead > CurrentPercentage)
    {
      CurrentPercentage = PercentageRead;
      system_messages::show_percentage_progress("Parsing Paraver Input Trace", CurrentPercentage);
    }

    /* Free memory used by the parser */
    delete CurrentEvent;
  }

  if (TraceParser->GetError())
  {
    SetError(true);
    SetErrorMessage("error parsing trace ", TraceParser->GetLastError());
    return false;
  }

  system_messages::show_percentage_end("Parsing Paraver Input Trace");

  if (ferror(InputTraceFile) != 0)
  {
    SetError(true);
    SetErrorMessage("error reading input trace file", strerror(errno));
    return false;
  }

  if (!TraceDataSet->DataExtractionFinished())
  {
    SetError(true);
    SetErrorMessage(TraceDataSet->GetLastError());
    return false;
  }

  /* No more burst
  if (!TraceDataSet->NoMoreBursts())
  {
    SetError(true);
    SetErrorMessage("error finishing data adcquisition");
    return false;
  }
  */

#ifdef DEBUG_PARAVER_INPUT
  // cout << "Data Size = " << TraceDataSet->GetDataSetSize() << endl;
#endif

  if (TraceDataSet->GetClusteringBurstsSize() == 0)
  {
    SetError(true);
    SetErrorMessage("No bursts extracted, cluster analysis cannot proceed");
    return false;
  }
  else
  {
    ostringstream Message;
    Message << "Points to analyse " << TraceDataSet->GetClusteringBurstsSize() << endl;
    system_messages::silent_information(Message.str());
  }

  return true;
}


bool PRVSemanticGuidedDataExtractor::ProcessSemanticCSV(timestamp_t CutOffset)
{
  ostringstream  ErrorMessage;
  vector<string> Record;
  string         Line;
  UINT32         CurrentLine;

  ifstream::pos_type FirstPos;
  ifstream::pos_type EndPos;

  FirstPos = SemanticCSVStream.tellg();
  SemanticCSVStream.seekg(0, ifstream::end);
  EndPos = SemanticCSVStream.tellg();
  SemanticCSVStream.seekg(FirstPos);

  /* DEBUG
  cout << "CutOffset = " << CutOffset << endl;
  */

  system_messages::show_percentage_progress("Loading semantic CSV file \""+InputSemanticCSV+"\"",
                                            0);

  CurrentLine = 1;
  while(getline(SemanticCSVStream, Line)  && SemanticCSVStream.good())
  {
    Record.clear();

    PopulateRecord (Record, Line, '\t');

    if (!PartialBurstFill(Record, CutOffset, CurrentLine))
    {
      system_messages::show_percentage_progress_error("Loading Semantic CSV file '"+InputSemanticCSV+"\"");
      return false;
    }

    int Percentage = static_cast<int>(SemanticCSVStream.tellg()) * 100 / EndPos;

    system_messages::show_percentage_progress("Loading Semantic CSV file '"+InputSemanticCSV+"\"",
                                              Percentage);

    CurrentLine++;
  }

  if (!SemanticCSVStream.eof())
  {
    ErrorMessage << "Error while semantic CSV file " << InputSemanticCSV;
    SetError(true);
    SetErrorMessage(ErrorMessage.str().c_str(),
                    strerror(errno));
    return false;
  }

  system_messages::show_percentage_end("Loading Semantic CSV file '"+InputSemanticCSV+"'");

  return true;
}

void PRVSemanticGuidedDataExtractor::PopulateRecord(vector<string> &Record,
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

bool PRVSemanticGuidedDataExtractor::PartialBurstFill(vector<string> &Record,
                                                      timestamp_t     Offset,
                                                      UINT32          CurrentLine)
{
  BurstContainer *NewBurst = new BurstContainer();
  istringstream   ObjectsParser;
  istringstream   Converter;
  ostringstream   RecordConverter;
  string          Item;
  vector<string>  RecordObjects;
  event_type_t    SemanticValue;

  if (Record.size() != SEMANTIC_CSV_FIELDS)
  {
    ostringstream ErrorMessage;
    ErrorMessage <<  "[Line:" << CurrentLine << "] Record has " << Record.size();
    ErrorMessage << " fields, " << SEMANTIC_CSV_FIELDS << " expected";

    SetError(true);
    SetErrorMessage(ErrorMessage.str());

    return false;
  }

  /* Check if the semantic value of the record is different from 0 (no value) */
  Converter.str(Record[3]);
  if (!(Converter >> SemanticValue))
  {
    ostringstream ErrorMessage;
    ErrorMessage <<  "[" << InputSemanticCSV << ":"<< CurrentLine;
    ErrorMessage <<  "] Wrong Semantic Value field (" << Record[3] << ")";

    SetError(true);
    SetErrorMessage(ErrorMessage.str());

    return false;
  }

  /* DEBUG
  cout << "Semantic Value = " << SemanticValue << endl;
  */

  if (SemanticValue == 0)
  {
    delete NewBurst;
    return true;
  }

  /* Object (APP.TASK.THREAD) parsing */
  ObjectsParser.str(Record[0]);
  while(std::getline(ObjectsParser, Item, '.'))
  {
    RecordObjects.push_back(Item);
  }

  if (RecordObjects.size() > 3 || RecordObjects.size() <= 1)
  {
    ostringstream ErrorMessage;
    ErrorMessage <<  "[" << InputSemanticCSV << ":"<< CurrentLine;
    ErrorMessage <<  "] Unknown Paraver object ";
    ErrorMessage << "definition (" << Record[0] << ")";

    SetError(true);
    SetErrorMessage(ErrorMessage.str());

    return false;
  }
  else if (RecordObjects.size() == 2 && !OneThreadPerTask)
  {
    ostringstream ErrorMessage;
    ErrorMessage <<  "[" << InputSemanticCSV << ":"<< CurrentLine;
    ErrorMessage <<  "] Unable to parse Semantic CSV file defined at Task level ";
    ErrorMessage << "when trace has multiple threads per task";

    SetError(true);
    SetErrorMessage(ErrorMessage.str());
    return false;
  }

  Converter.clear();
  Converter.str(RecordObjects[1]);
  if (!(Converter >> NewBurst->TaskId))
  {
    ostringstream ErrorMessage;
    ErrorMessage <<  "[" << InputSemanticCSV << ":"<< CurrentLine;
    ErrorMessage <<  "] Wrong Task Id field (" << RecordObjects[1] << ")";

    SetError(true);
    SetErrorMessage(ErrorMessage.str());

    return false;
  }
  NewBurst->TaskId--;

  /* Here we consider those CSV registers with full hierachy specification
   * (thread level CSV) or those at task level, when each task just have one
   * threads */

  if (RecordObjects.size() == 3)
  {
    Converter.clear();
    Converter.str(RecordObjects[2]);
    if (!(Converter >> NewBurst->ThreadId))
    {
      ostringstream ErrorMessage;
      ErrorMessage <<  "[" << InputSemanticCSV << ":"<< CurrentLine;
      ErrorMessage <<  "] Wrong Thread Id field (" << RecordObjects[2] << ")";

      SetError(true);
      SetErrorMessage(ErrorMessage.str());

      return false;
    }
    NewBurst->ThreadId--;
  }
  else
  {
    NewBurst->ThreadId = 0;
  }

  /* Burst times */

  Converter.clear();
  Converter.str(Record[1]);
  if (!(Converter >> NewBurst->BeginTime))
  {
    ostringstream ErrorMessage;
    ErrorMessage <<  "[" << InputSemanticCSV << ":"<< CurrentLine;
    ErrorMessage <<  "] Wrong Begin Time field (" << Record[1] << ")";

    SetError(true);
    SetErrorMessage(ErrorMessage.str());

    return false;
  }
  NewBurst->BeginTime = NewBurst->BeginTime - Offset;

  Converter.clear();
  Converter.str(Record[2]);
  if (!(Converter >> NewBurst->Duration))
  {
    ostringstream ErrorMessage;
    ErrorMessage <<  "[" << InputSemanticCSV << ":"<< CurrentLine;
    ErrorMessage <<  "] Wrong Begin Time field (" << Record[1] << ")";

    SetError(true);
    SetErrorMessage(ErrorMessage.str());

    return false;
  }

  NewBurst->EndTime = NewBurst->BeginTime + NewBurst->Duration;

  NewBurst->EventsData[SEMANTIC_VALUE_EVT_TYPE] = SemanticValue;

  NewBurst->IntermediateHWChange = false;
  NewBurst->BurstEndEvents.clear();

  // BurstsToLoad.push_back(NewBurst);
  /* Load the new burst in the 'BurstsToLoad' map */
  RecordConverter << NewBurst->TaskId << "." << NewBurst->ThreadId;
  if ( BurstsToLoad.find(RecordConverter.str()) == BurstsToLoad.end())
  {
    BurstsToLoad.insert(make_pair(RecordConverter.str(), list<BurstContainer*>()));
  }

  BurstsToLoad[RecordConverter.str()].push_back(NewBurst);

  // cout << CurrentLine << ":" << NewBurst->toString() << endl;

  return true;

  /*
  TaskData.TaskId        = CurrentState->GetTaskId();
  TaskData.ThreadId      = CurrentState->GetThreadId();
  TaskData.Line          = CurrentState->GetLine();
  TaskData.OngoingBurst  = true;
  TaskData.BeginTime     = CurrentState->GetBeginTime();
  TaskData.EndTime       = CurrentState->GetEndTime();
  TaskData.BurstDuration =
  (UINT64) ((CurrentState->GetEndTime() - CurrentState->GetBeginTime()) * TimeFactor);
  */
}

bool PRVSemanticGuidedDataExtractor::ProcessEvent(Event *CurrentEvent,
                                                  TraceData   *TraceDataSet)
{
  BurstContainer* CurrentBurst;

  ostringstream Object;
  Object << CurrentEvent->GetTaskId() << "." << CurrentEvent->GetThreadId();

  /* DEBUG
  cout << "Event for object " << Object.str() << endl;
  */


  if (BurstsToLoad.find(Object.str()) == BurstsToLoad.end())
  {
    /* DEBUG
    cout << "Object not found!" << endl;
    */

    return true;
  }

  if (BurstsToLoad[Object.str()].empty())
  {
    return true;
  }

  CurrentBurst = BurstsToLoad[Object.str()].front();

  if (CurrentEvent->GetTimestamp()  > CurrentBurst->BeginTime &&
      CurrentEvent->GetTimestamp() <= CurrentBurst->EndTime)
  {
    for (INT32 i = 0; i < CurrentEvent->GetTypeValueCount(); i++)
    {
      event_type_t  CurrentEventType  = CurrentEvent->GetType(i);
      event_value_t CurrentEventValue = CurrentEvent->GetValue(i);

      /* Check if there is a HWC change. */
      if (CurrentEventType == HWC_GROUP_CHANGE_TYPE)
      {
        if (CurrentEvent->GetTimestamp() < CurrentBurst->EndTime)
        {
          CurrentBurst->IntermediateHWChange = true;
        }
        else
        { /* If it appears at the end of the burst, just store the burst and
           * stop the data processing. Further events are not eligible. */

          /* Those events that not appear during all the burst (common to all
           * data adcquisitions) are discarded */
          set<event_type_t>::iterator it;

          for (it  = CurrentBurst->NotCommonEvents.begin();
               it != CurrentBurst->NotCommonEvents.end();
             ++it)
          {
            CurrentBurst->EventsData.erase((*it));
          }

          /* DEBUG
          cout << "*** CALLING TraceDataSet->NewBurst ***" << endl;
          cout << CurrentBurst->toString() << endl;
          */

          if (!TraceDataSet->NewBurst(CurrentBurst->TaskId,
                                      CurrentBurst->ThreadId,
                                      CurrentBurst->Line,
                                      CurrentBurst->BeginTime,
                                      CurrentBurst->EndTime,
                                      CurrentBurst->Duration,
                                      CurrentBurst->EventsData,
                                      CurrentBurst->BurstEndEvents))
          {
            SetError(true);
            SetErrorMessage("error storing burst data",
                            TraceDataSet->GetLastError());
            return false;
          }


          BurstsToLoad[Object.str()].pop_front();
          delete CurrentBurst;

          break;
        }
      }
      else
      {
        if (CurrentBurst->EventsData.find(CurrentEventType) == CurrentBurst->EventsData.end())
        {
          if (CurrentBurst->IntermediateHWChange)
          { /* As the HWC group changed, this is a type of event that never
               appear previously, so it is not common to all the burst so
               it won't be taken in consideration */
            CurrentBurst->NotCommonEvents.insert(CurrentEventType);
          }
          else
          { /* As the HWC did not change, we can store the event and value
             * normally */
            CurrentBurst->EventsData[CurrentEventType] = CurrentEventValue;

            if (CurrentEvent->GetTimestamp() == CurrentBurst->EndTime)
            { /* Annotate that current burst has events at his end time */
              CurrentBurst->BurstEndEvents.insert(CurrentEventType);
            }
          }
        }
        else
        { /* This event type already appeared during the burst, so it can be
           * stored (adding its value to previous adcquisitions) */
          CurrentBurst->EventsData[CurrentEventType] += CurrentEventValue;

          if (CurrentEvent->GetTimestamp() == CurrentBurst->EndTime)
          { /* Annotate that current burst has events at his end time */
            CurrentBurst->BurstEndEvents.insert(CurrentEventType);
          }
        }
      }
    }
  }
  else if (CurrentEvent->GetTimestamp() > CurrentBurst->EndTime)
  {
    set<event_type_t>::iterator it;

    for (it  = CurrentBurst->NotCommonEvents.begin();
         it != CurrentBurst->NotCommonEvents.end();
         ++it)
    {
      CurrentBurst->EventsData.erase((*it));
    }

    /* DEBUG
    cout << "*** CALLING TraceDataSet->NewBurst ***" << endl;
    cout << CurrentBurst->toString() << endl;
    */

    if (!TraceDataSet->NewBurst(CurrentBurst->TaskId,
                                CurrentBurst->ThreadId,
                                CurrentBurst->Line,
                                CurrentBurst->BeginTime,
                                CurrentBurst->EndTime,
                                CurrentBurst->Duration,
                                CurrentBurst->EventsData,
                                CurrentBurst->BurstEndEvents))
    {
      SetError(true);
      SetErrorMessage("error storing burst data",
                      TraceDataSet->GetLastError());
      return false;
    }

    /* DEBUG
    cout << "Called NEW BURST 1" << endl;
    */

    BurstsToLoad[Object.str()].pop_front();
    delete CurrentBurst;

    return this->ProcessEvent(CurrentEvent, TraceDataSet);
  }


  return true;
}

