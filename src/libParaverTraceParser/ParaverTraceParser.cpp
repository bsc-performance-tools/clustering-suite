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

#include "ParaverTraceParser.hpp"
#include <cerrno>
#include <cmath>
#include <cstring>
#include <sys/stat.h>
#include <cstdlib>

#include <iostream>
using std::cout;
using std::endl;

/******************************************************************************
 * Public functions
 ******************************************************************************/

ParaverTraceParser::ParaverTraceParser(string ParaverTraceName,
                                       FILE* ParaverTraceFile)
{
  CurrentLine        = 1;
  ParsingInitialized = false;
  
  if (ParaverTraceFile != NULL)
  {
    this->ParaverTraceFile = ParaverTraceFile;
    this->ParaverTraceName = ParaverTraceName;
    return;
  }
  
  if ( (ParaverTraceFile = fopen(ParaverTraceName.c_str(), "r")) == NULL)
  {
    SetError(true);
    SetErrorMessage("Unable to open Paraver trace", strerror(errno));
    return;
  }

  this->ParaverTraceName = ParaverTraceName;
}

bool
ParaverTraceParser::InitTraceParsing(void)
{
  struct stat FileStat;
  char* StrHeader;
  INT32 HeaderLength;
  vector<ApplicationDescription_t> AppsDescription;
  
  if (ParsingInitialized)
  {
    Reload();
    return true;
  }
  
  if (fstat(fileno(ParaverTraceFile), &FileStat) < 0)
  {
    SetErrorMessage("Error reading Paraver trace statistics",
                    strerror(errno));
    return false;
  }
  this->TraceSize = FileStat.st_size;
  
  if (fseeko(ParaverTraceFile, 0, SEEK_SET) == -1)
  {
    SetErrorMessage("Error seeking header position",
                    strerror(errno));
    return false;
  }
  
  if ( (HeaderLength = GetLongLine(&StrHeader)) < 0 )
  {
    SetErrorMessage("Error seeking Paraver trace file while reading header",
                    strerror(errno));
    return false;
  }
  
  FirstCommunicatorOffset = ftello(ParaverTraceFile);
  
  /* Initialization of structures needed to trace parsing 
   * (task handlers, thread handlers, ecc) */
  
  Header = new ParaverHeader(StrHeader, HeaderLength);
  
  /* Free Header line */
  free((void*) StrHeader);
  
  if (Header->GetError())
  {
    LastError = Header->GetLastError();
    return false;
  }
  
  if (fseeko (ParaverTraceFile, FirstCommunicatorOffset, SEEK_SET) == -1)
  {
    SetErrorMessage("Unable to seek on first communicator",
                    strerror(errno));
    return false;
  }
  
  AppsDescription = Header->GetAppsDescription();
  for (unsigned int i = 0; i < AppsDescription.size(); i++)
  {
    if (!GetAppCommunicators(AppsDescription[i]))
      return false;
  }
  
  FirstRecordOffset = ftello(ParaverTraceFile);
  FirstRecordLine   = CurrentLine;
  
  ParsingInitialized = true;
  
  return true;
}

vector<ApplicationDescription_t>
ParaverTraceParser::GetApplicationsDescription(void)
{
  if (!ParsingInitialized)
  {
    SetError(true);
    LastError = "Parsing not initialized";
    return vector<ApplicationDescription_t> (0);
  }
  
  return Header->GetAppsDescription();
}

ParaverHeader*
ParaverTraceParser::GetHeader(void)
{
  if (!ParsingInitialized)
  {
    SetError(true);
    LastError = "Parsing not initialized";
    return NULL;
  }
  
  return Header;
}


INT32
ParaverTraceParser::GetTimeUnits(void)
{
  if (!ParsingInitialized)
  {
    SetError(true);
    LastError = "Parsing not initialized";
    return -1;
  }
  
  return Header->GetTimeUnits();
}

ParaverRecord_t
ParaverTraceParser::GetNextRecord(void)
{
  return NextTraceRecord(ANY_REC);
}

ParaverRecord_t
ParaverTraceParser::GetNextRecord(UINT32 RecordTypeMask)
{
  return NextTraceRecord(RecordTypeMask);
}

ParaverRecord_t
ParaverTraceParser::GetNextRecord(UINT32 RecordTypeMask, set<INT32> TaskIds)
{
  ParaverRecord_t      Record = NULL;
  bool                 End = false;
  set<INT32>::iterator TaskIdsIterator;

  do {
    if (Record != NULL)
      delete Record;
    
    if ((Record = NextTraceRecord(RecordTypeMask)) == NULL)
      return NULL;

    TaskIdsIterator = TaskIds.find(Record->GetTaskId());

    if (TaskIdsIterator != TaskIds.end())
    {
      End = true;
    }
  } while (!End);

  return Record;
}

ParaverRecord_t
ParaverTraceParser::GetNextTaskRecord(INT32  TaskId)
{
  ParaverRecord_t Record;
  
  Record = NextTraceRecord(EVENT_REC | STATE_REC);
  
  while (Record->GetTaskId() != TaskId)
  {
    delete Record;
    if ( (Record = NextTraceRecord(EVENT_REC | STATE_REC)) == NULL)
      return NULL;
  }
  
  return Record;
}

ParaverRecord_t ParaverTraceParser::GetNextTaskRecord(set<INT32> TaskIds)
{
  ParaverRecord_t      Record = NULL;
  bool                 End = false;
  set<INT32>::iterator TaskIdsIterator;

  do {
    if (Record != NULL)
      delete Record;
    
    if ((Record = NextTraceRecord(EVENT_REC | STATE_REC)) == NULL)
      return NULL;

    TaskIdsIterator = TaskIds.find(Record->GetTaskId());

    if (TaskIdsIterator != TaskIds.end())
    {
      End = true;
    }
  } while (!End);

  return Record;
}

State_t
ParaverTraceParser::GetNextState(void)
{
  return (State*) (NextTraceRecord(STATE_REC));
}

Event_t
ParaverTraceParser::GetNextEvent(void)
{
  return (Event*) (NextTraceRecord(EVENT_REC));
}

Communication_t
ParaverTraceParser::GetNextCommunication(void)
{
  return (Communication*) (NextTraceRecord(COMM_REC));
}

GlobalOp_t
ParaverTraceParser::GetNextGlobalOp(void)
{
  return (GlobalOp*) (NextTraceRecord(GLOBOP_REC));
}

INT32 ParaverTraceParser::GetFilePercentage(void)
{
  off_t CurrentPosition   = ftello(ParaverTraceFile);
  INT32 CurrentPercentage = lround (100.0*CurrentPosition/TraceSize);
  
  return CurrentPercentage;
}

bool
ParaverTraceParser::Reload(void)
{
  if (!ParsingInitialized)
  {
    LastError = "Parsing not initialized";
    return false;
  }
  
  if (fseeko (ParaverTraceFile, FirstRecordOffset, SEEK_SET) == -1)
  {
    SetErrorMessage("Unable to seek on first record",
                    strerror(errno));
    return false;
  }
  
  CurrentLine = FirstRecordLine;
  return true;
}

/*****************************************************************************
 * Private functions
 ****************************************************************************/

bool
ParaverTraceParser::GetAppCommunicators(ApplicationDescription_t AppDescription)
{
  char  *TraceLine;
  INT32  CurrentLineSize;
  INT32  CurrentCommunicator;
  bool   COMM_WORLD_set = false;
  
  Communicator_t NewCommunicator;
  
  for(CurrentCommunicator = 0;
      CurrentCommunicator < AppDescription->GetCommunicatorCount();
      CurrentCommunicator++)
  {
    if ((CurrentLineSize = GetLongLine(&TraceLine)) < 0)
    {
      free( (void*) TraceLine);
      return false;
    }

    NewCommunicator = new Communicator(TraceLine);
    
    if (!COMM_WORLD_set)
    {
      if (NewCommunicator->GetCommunicatorSize() == 
          AppDescription->GetTaskCount())
      {
        NewCommunicator->SetCOMM_WORLD(true);
        COMM_WORLD_set = true;
      }
    }
    
    if (NewCommunicator->GetError())
    {
      char CurrentError[128]; 
      sprintf(CurrentError,
              "Error parsing communicator (line %lu)",
              (long unsigned int) CurrentLine);
      SetErrorMessage (CurrentError,
                       NewCommunicator->GetLastError().c_str());
      free( (void*) TraceLine);
      return false;
    }
    
    AppDescription->AddCommunicator(NewCommunicator);
    free( (void*) TraceLine);
  }

  return true;
}

INT32
ParaverTraceParser::GetLongLine(char** Line)
{
  INT32 LineLength = 0;
  off_t InitialPosition;
  int   CharRead;
  
  InitialPosition = ftello(ParaverTraceFile);
  
  if (feof(ParaverTraceFile))
    return 0;
  
  /* To avoid initial position on CR */
  if (fgetc(ParaverTraceFile) == '\n')
    InitialPosition++;
  else
  {
    if ( fseeko(ParaverTraceFile, InitialPosition, SEEK_SET) < 0)
    {
      return -1;
    }
  }
  
  if (feof(ParaverTraceFile))
    return 0;
  
  /*
  if (fseeko(ParaverTraceFile, 0, SEEK_SET) < 0)
  {
    SetErrorMessage("Error seeking Paraver trace file while reading header",
                    strerror(errno));
    return -1;
  }
  */
  
  while ( (CharRead = fgetc(ParaverTraceFile)) != '\n' )
  {
    if (CharRead == EOF && LineLength < 1)
      return -1;
    LineLength++;
  }
  
  /* FirstCommunicatorOffset = ftello(ParaverTraceFile); */
  
  /* cout << "Header size: " << HeaderLength << endl;; */
  
  *Line = (char*) calloc(sizeof(char), LineLength+2);
  
  if ( fseeko(ParaverTraceFile, InitialPosition, SEEK_SET) < 0)
  {
    return -1;
  }
  
  fgets(*Line, LineLength+1, ParaverTraceFile);
  
  CurrentLine++;
  return LineLength;
}

bool
ParaverTraceParser::GetTraceLine(char* Line, int LineSize)
{
  if (feof(ParaverTraceFile))
    return false;
  
  if (fgets (Line, LineSize, ParaverTraceFile) == NULL)
  {
    SetError(true);
    SetErrorMessage("Error reading Paraver trace file", strerror(errno));
    return false;
  }
  
  CurrentLine++;
  return true;
}

ParaverRecord_t
ParaverTraceParser::NextTraceRecord(UINT32 RecordTypeMask)
{
  ParaverRecord_t Result;
  INT32           CurrentRecordType;
  UINT32          CurrentRecordTypeMask;
  char*           Line;
  bool            found = false;
  
  if (!ParsingInitialized)
  {
    SetError(true);
    LastError = "Parsing not initialized";
    return NULL;
  }
  
  while (!found)
  {
    if (feof(ParaverTraceFile))
    {
      return NULL;
    }
    
    if (GetLongLine(&Line) <= 0)
    {
      if (!feof(ParaverTraceFile))
      {
        SetError(true);
        SetErrorMessage("Error reading file", strerror(errno));
      }
      return NULL;
    }
    
    if ( sscanf(Line, "%d:", &CurrentRecordType) != 1 )
    {

      if (Line[0] != '#')
      {
        char CurrentError[128]; 
        
        SetError(true);
        sprintf(CurrentError,
                  "wrong record format on line %lu",
                  (long unsigned int) CurrentLine);
        LastError = CurrentError;

        fprintf (stderr, "Current line: %s", Line);
        
        return NULL;
      }
      else
      { /* Skip comments! */
        continue;
      }
    }
    
    CurrentRecordTypeMask = 1 << CurrentRecordType;
    
    /* DEBUG!! 
    cout << "Desired Mask: " << RecordTypeMask;
    cout << " Current Mask: " << CurrentRecordTypeMask;
    cout << " Type: " << CurrentRecordType << endl;
    */
    
    // (RecordTypeMask == ANY_REC) || 
    if ((CurrentRecordTypeMask & RecordTypeMask) != 0)
    {
      /* cout << "Record PARSED!" << endl; */
      found = true;
      switch(CurrentRecordType)
      {
        case PARAVER_STATE:
          Result = (ParaverRecord*) ParseState(&Line[2]);
          break;
        case PARAVER_EVENT:
          Result = (ParaverRecord*) ParseEvent(&Line[2]);
          break;
        case PARAVER_COMMUNICATION:
          Result = (ParaverRecord*) ParseCommunication(&Line[2]);
          break;
        case PARAVER_GLOBALOP:
          Result = (ParaverRecord*) ParseGlobalOp(&Line[2]);
          break;
        default:
          char CurrentError[128]; 
        
          SetError(true);
          sprintf(CurrentError,
                    "Wrong record identifier (%d) on line %lu",
                    CurrentRecordType,
                    (long unsigned int) CurrentLine);
          LastError = CurrentError;
          Result = NULL;
          break;
      }
    }
    
    /* Free memory used reading line */
    free((void*) Line);
  }

  return Result;
}

State_t
ParaverTraceParser::ParseState(char* ASCIIState)
{
  State_t NewState;
  INT32   CPU, AppId, TaskId, ThreadId;
  UINT64  BeginTime, EndTime;
  INT32   StateValue;
  
  if (sscanf(ASCIIState,
             "%d:%d:%d:%d:%llu:%llu:%d",
             &CPU, &AppId, &TaskId, &ThreadId,
             &BeginTime, &EndTime,
             &StateValue) == 7)
  {
    NewState = new State(CurrentLine,
                         CPU, AppId, TaskId, ThreadId, 
                         BeginTime, EndTime,
                         StateValue);
  }
  else
  {
    char CurrentError[128]; 
    
    SetError(true);
    sprintf(CurrentError,
            "Wrong state record on line %lu",
            (long unsigned int) CurrentLine);
    LastError = CurrentError;
    NewState = NULL;
  }
  return NewState;
}

Event_t
ParaverTraceParser::ParseEvent(char* ASCIIEvent)
{
  Event_t NewEvent;
  UINT64  Timestamp;
  INT32   CPU, AppId, TaskId, ThreadId;
  INT32   Type;
  INT64   Value;
  char*   TypeValueStr = (char*) calloc(strlen(ASCIIEvent)+1, sizeof(char));
  char*   CurrToken;
  INT32   PairsReaded = 0;
  
  if (sscanf(ASCIIEvent,
             "%d:%d:%d:%d:%llu:%[^n]",
             &CPU, &AppId, &TaskId, &ThreadId,
             &Timestamp, TypeValueStr) == 6)
  {
    NewEvent = new Event(CurrentLine, Timestamp, CPU, AppId, TaskId, ThreadId);
    
    CurrToken = strtok(TypeValueStr, ":");
    while(CurrToken != NULL)
    {
      Type = atoi(CurrToken);
      
      CurrToken = strtok(NULL, ":");
      if (CurrToken == NULL)
      {
        char CurrentError[256];
        SetError(true);
        sprintf(CurrentError,
                "Unpaired type/value on event record on line %lu",
                (long unsigned int) CurrentLine);
        LastError = CurrentError;
        delete NewEvent;
        free( (void*) TypeValueStr);
        return NULL;
      }
      
      Value = atoll(CurrToken);
      PairsReaded++;
      
      NewEvent->AddTypeValue(Type, Value);
      
      CurrToken = strtok(NULL, ":");
    }
    
    if (PairsReaded == 0)
    {
      char CurrentError[256];
      SetError(true);
      sprintf(CurrentError,
              "Event record without type/value pairs on line %lu",
              (long unsigned int) CurrentLine);
      LastError = CurrentError;
      delete NewEvent;
      free( (void*) TypeValueStr);
      return NULL;
    }
  }
  else
  {
    char CurrentError[128]; 
    
    SetError(true);
    sprintf(CurrentError,
            "Wrong event record on line %lu",
            (long unsigned int) CurrentLine);
    LastError = CurrentError;
    NewEvent = NULL;
  }
  
  free( (void*) TypeValueStr);
  return NewEvent;
}

Communication_t
ParaverTraceParser::ParseCommunication(char* ASCIICommunication)
{
  Communication_t NewCommunication;
  
  UINT64 LogSend, PhySend, LogRecv, PhyRecv;
  INT32  SrcCPU, SrcAppId, SrcTaskId, SrcThreadId;
  INT32  DstCPU, DstAppId, DstTaskId, DstThreadId;
  INT32  Size;
  INT32  Tag;
  
  
  if (sscanf(ASCIICommunication,
             "%d:%d:%d:%d:%llu:%llu:%d:%d:%d:%d:%llu:%llu:%d:%d",
             &SrcCPU, &SrcAppId, &SrcTaskId, &SrcThreadId,
             &LogSend, &PhySend,
             &DstCPU, &DstAppId, &DstTaskId, &DstThreadId,
             &LogRecv, &PhyRecv,
             &Size, &Tag) == 14)
  {
    NewCommunication = 
      new Communication(CurrentLine,
                        LogSend, PhySend, LogRecv, PhyRecv,
                        SrcCPU, SrcAppId, SrcTaskId, SrcThreadId,
                        DstCPU, DstAppId, DstTaskId, DstThreadId,
                        Size, Tag);
  }
  else
  {
    char CurrentError[128]; 
    
    SetError(true);
    sprintf(CurrentError,
            "Wrong communication record on line %lu",
            (long unsigned int) CurrentLine);
    LastError = CurrentError;
    NewCommunication = NULL;
  }
  
  return NewCommunication;
}

GlobalOp_t
ParaverTraceParser::ParseGlobalOp(char* ASCIIGlobalOp)
{
  GlobalOp_t NewGlobalOp;

  UINT64 Timestamp;
  INT32  CPU, AppId, TaskId, ThreadId;
  INT32  CommunicatorId;
  INT32  SendSize, RecvSize;
  INT32  GlobalOpId;
  INT32  RootTaskId;
  
  if (sscanf(ASCIIGlobalOp,
             "%d:%d:%d:%d:%llu:%d:%d:%d:%d:%d",
             &CPU, &AppId, &TaskId, &ThreadId,
             &Timestamp,
             &CommunicatorId,
             &SendSize, &RecvSize,
             &GlobalOpId, &RootTaskId) == 10)
  {
    
    NewGlobalOp = new GlobalOp(CurrentLine,
                               Timestamp,
                               CPU, AppId, TaskId, ThreadId,
                               CommunicatorId,
                               SendSize, RecvSize,
                               GlobalOpId, RootTaskId);
  }
  else
  {
    char CurrentError[128]; 
    
    SetError(true);
    sprintf(CurrentError,
            "Wrong global operation record on line %lu",
            (long unsigned int) CurrentLine);
    LastError = CurrentError;
    NewGlobalOp = NULL;
  }
  
  return NewGlobalOp;
}
