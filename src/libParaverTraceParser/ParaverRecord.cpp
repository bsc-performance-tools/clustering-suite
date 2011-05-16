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
#include <string.h>

#include "ParaverRecord.hpp"
using std::endl;

#include "EventEncoding.h"
#include "Dimemas_Generation.h"


/*****************************************************************************
 * class ParaverRecord
 ****************************************************************************/

ParaverRecord::ParaverRecord(UINT64 Line,
                             UINT64 Timestamp,
                             INT32  CPU, 
                             INT32  AppId,
                             INT32  TaskId,
                             INT32  ThreadId)
{
  this->Line      = Line;
  this->Timestamp = Timestamp;
  this->CPU       = CPU-1;
  this->AppId     = AppId-1;
  this->TaskId    = TaskId-1;
  this->ThreadId  = ThreadId-1;
}

bool ParaverRecord::Flush(FILE* OutputFile)
{
  /* Print common fields on each record */
  if (!FlushRecordType(OutputFile))
    return false;
  
  if (fprintf(OutputFile,
              ":%d:%d:%d:%d:%llu",
              CPU+1,
              AppId+1,
              TaskId+1,
              ThreadId+1,
              Timestamp) < 0)
  {
    SetError(true);
    SetErrorMessage("unable to print record common files",
                    strerror(errno));
    return false;
  }
  
  if (!FlushSpecificFields(OutputFile))
  {
    return false;
  }
  
  if (fprintf(OutputFile, "\n") < 0)
  {
    SetError(true);
    SetErrorMessage("unable to print communicator record", strerror(errno));
    return false;
  }
  
  return true;
}

ostream& operator<< (ostream& os, const ParaverRecord& Rec)
{
  Rec.Write(os);
  return os;
}

/*****************************************************************************
 * class State
 ****************************************************************************/
State::State(UINT64 Line,
             INT32  CPU, INT32  AppId, INT32  TaskId, INT32  ThreadId,
             UINT64 BeginTime, 
             UINT64 EndTime,
             INT32  StateValue)
:ParaverRecord(Line, BeginTime, CPU, AppId, TaskId, ThreadId)
{
  this->TimestampEnd = EndTime;
  this->StateValue   = StateValue;
}

bool
State::FlushRecordType(FILE* OutputFile)
{
  if (fprintf(OutputFile, "%d", PARAVER_STATE) < 0)
  {
    SetError(true);
    SetErrorMessage("unable to print record type",
                    strerror(errno));
    return false;
  }
  return true;
}

bool
State::FlushSpecificFields(FILE* OutputFile)
{
  if (fprintf(OutputFile, ":%llu:%d", TimestampEnd, StateValue) < 0)
  {
    SetError(true);
    SetErrorMessage("unable to print state record fields",
                    strerror(errno));
    return false;
  }
  return true;
}

void
State::Write(ostream& os) const
{
  os << "State: " << " [";

  os.width(3);
  os.fill('0');
  os << TaskId << ":";

  os.width(2);
  os.fill('0');
  os << ThreadId << "]";
  
  os << Timestamp << " T:" << TimestampEnd << " State: " << StateValue;
  os << endl;
}

ostream& operator<< (ostream& os, const State& Comm)
{
  Comm.Write(os);
  return os;
}

/*****************************************************************************
 * class EventTypeValue
 ****************************************************************************/

INT64 EventTypeValue::CurrentTraceOrder = 0;

bool
EventTypeValue::IsDimemasBlockBegin(void)
{

  if (!MPIEventEncoding_Is_MPIBlock( (INT64)  Type ) &&
      !MPIEventEncoding_Is_UserBlock( (INT64) Type ))
  {
    return false;
  }
  
  if (MPIEventEncoding_Is_BlockBegin( Value ))
    return true;
  
  return false;
}

bool
EventTypeValue::IsDimemasBlockEnd(void)
{
  if (!MPIEventEncoding_Is_MPIBlock( (INT64) Type ) &&
      !MPIEventEncoding_Is_UserBlock( (INT64) Type ))
  {
    return false;
  }
  
  if (!MPIEventEncoding_Is_BlockBegin( Value ))
    return true;
  
  return false;
}

bool
EventTypeValue::IsCaller(void)
{
  if (Type >= MPI_CALLER_EV && Type <= MPI_CALLER_EV_END)
    return true;

  return false;
}

bool
EventTypeValue::IsCallerLine(void)
{
  if (Type >= MPI_CALLER_LINE_EV && Type <= MPI_CALLER_LINE_EV_END)
    return true;

  return false;
}

INT64
EventTypeValue::NewTraceOrder(void)
{
  return CurrentTraceOrder++;
}

bool
EventTypeValue::FlushSpecificFields(FILE* OutputFile)
{
  if (fprintf(OutputFile, ":%d:%lld", Type, Value) < 0)
  {
    SetError(true);
    SetErrorMessage("unable to print type/value pair",
                    strerror(errno));
    return false;
  }
  return true;
}

/*****************************************************************************
 * class Event
 ****************************************************************************/

Event::Event(UINT64 Line,
             UINT64 Timestamp,
             INT32 CPU, INT32 AppId, INT32 TaskId, INT32 ThreadId)
:ParaverRecord(Line, Timestamp, CPU, AppId, TaskId, ThreadId)
{
  ContentPresent = false;
}

Event::~Event(void)
{
  if (ContentPresent)
  {
    for (unsigned int i = 0; i < Content.size(); i++)
      delete Content[i];
  }
}

void Event::AddTypeValue(INT32 Type, INT64 Value)
{
  EventTypeValue_t newTypeValue = new EventTypeValue(Type, Value);
  Content.push_back(newTypeValue);
  ContentPresent = true;
};

UINT32
Event::GetTypeValueCount(void)
{
  return (UINT32) Content.size();
}

INT32
Event::GetFirstType(void)
{
  return GetType(0);
}

INT32
Event::GetType(UINT32 Index)
{
  if (Index < Content.size())
    return Content[Index]->GetType();
  else
    return -1;
}

INT64
Event::GetFirstValue(void)
{
  return GetValue(0);
}

INT64
Event::GetValue(UINT32 Index)
{
  if (Index < Content.size())
    return Content[Index]->GetValue();
  else
    return -1;
}

INT64
Event::GetFirstTraceOrder(void)
{
  return GetTraceOrder(0);
}

INT64
Event::GetTraceOrder(UINT32 Index)
{
  if (Index < Content.size())
    return Content[Index]->GetTraceOrder();
  else
    return -1;
}

bool
Event::IsDimemasBlockBegin(void)
{
  if (Content.size() < 1)
    return false;
  else
    return Content[0]->IsDimemasBlockBegin();
}

bool
Event::IsDimemasBlockEnd (void)
{
  if (Content.size() < 1)
    return false;
  else
    return Content[0]->IsDimemasBlockEnd();
}

bool
Event::IsCaller(void)
{
  if (Content.size() < 1)
    return false;
  else
    return Content[0]->IsCaller();
}

bool
Event::IsCallerLine(void)
{
  if (Content.size() < 1)
    return false;
  else
    return Content[0]->IsCallerLine();
}

bool
Event::FlushRecordType(FILE* OutputFile)
{
  if (fprintf(OutputFile, "%d", PARAVER_EVENT) < 0)
  {
    SetError(true);
    SetErrorMessage("unable to print record type",
                    strerror(errno));
    return false;
  }
  return true;
}

bool
Event::FlushSpecificFields(FILE* OutputFile)
{
  for (INT32 i = 0; i < Content.size(); i++)
  {
    if (!Content[i]->FlushSpecificFields(OutputFile))
      return false;
  }
  return true;
}

void
Event::Write(ostream& os) const
{
  os << "Evt: [";

  os.width(3);
  os.fill('0');
  os << TaskId << ":";

  os.width(2);
  os.fill('0');
  os << ThreadId << "] ";

  os << "T:" << Timestamp;
  
  for (UINT32 i = 0; i < Content.size(); i++)
  {
    os << " [" << Content[i]->GetTraceOrder() << "]: ";
    os << Content[i]->GetType() << ":" << Content[i]->GetValue();
  }
  
  os << endl;
}

ostream& operator<< (ostream& os, const Event& Evt)
{
  Evt.Write(os);
  return os;
}

/*****************************************************************************
 * class Communication
 ****************************************************************************/

INT64 Communication::CurrentTraceOrder = 0;

Communication::Communication(UINT64 Line,
                             UINT64 LogSend, UINT64 PhySend,
                             UINT64 LogRecv, UINT64 PhyRecv,
                             INT32  SrcCPU,    INT32 SrcAppId,
                             INT32  SrcTaskId, INT32 SrcThreadId,
                             INT32  DstCPU,    INT32 DstAppId,
                             INT32  DstTaskId, INT32 DstThreadId,
                             INT32  Size,
                             INT32  Tag)
{
  this->Line   = Line;
  
  CPU          = SrcCPU-1;
  AppId        = SrcAppId-1;
  TaskId       = SrcTaskId-1;
  ThreadId     = SrcThreadId-1;
  DestCPU      = DstCPU-1;
  DestAppId    = DstAppId-1;
  DestTaskId   = DstTaskId-1;
  DestThreadId = DstThreadId-1;
  
  Timestamp    = LogSend; /* Timestamp attribute corresponds to Logical Send*/
  PhysicalSend = PhySend;
  LogicalRecv  = LogRecv;
  PhysicalRecv = PhyRecv;
  
  this->Size = Size;
  this->Tag  = Tag;
  TraceOrder = Communication::NewTraceOrder();
  
}

bool
Communication::FlushRecordType(FILE* OutputFile)
{
  if (fprintf(OutputFile, "%d", PARAVER_COMMUNICATION) < 0)
  {
    SetError(true);
    SetErrorMessage("unable to print record type",
                    strerror(errno));
    return false;
  }
  return true;
}

bool
Communication::FlushSpecificFields(FILE* OutputFile)
{
  if (fprintf(OutputFile,
              ":%llu:%d:%d:%d:%d:%llu:%llu:%d:%d",
              PhysicalSend,
              DestCPU+1,
              DestAppId+1,
              DestTaskId+1,
              DestThreadId+1,
              LogicalRecv,
              PhysicalRecv,
              Size,
              Tag) < 0)
  {
    SetError(true);
    SetErrorMessage("unable to print communication record fields");
    return false;
  }
  return true;
}

void
Communication::Write(ostream& os) const
{
  os << "Communication Record" << endl;

  os << "Sender: [";

  os.width(3);
  os.fill('0');
  os << TaskId << ":";

  os.width(2);
  os.fill('0');
  os << ThreadId << "]";

  os << " LogSend: " << Timestamp << " PhySend: " << PhysicalSend << endl;
  
  os << "Recvr: [";
  
  os.width(3);
  os.fill('0');
  os << DestTaskId << ":";

  os.width(2);
  os.fill('0');
  os << DestThreadId << "]";

  os << " LogRecv: " << LogicalRecv << " PhyRecv: " << PhysicalRecv << endl;
  
  os << "Size: " << Size << " Tag: " << Tag << endl;
}

INT64 Communication::NewTraceOrder(void)
{
  return CurrentTraceOrder++;
}




ostream& operator<< (ostream& os, const Communication& Comm)
{
  Comm.Write(os);
  return os;
}

/*****************************************************************************
 * class GlobalOp
 ****************************************************************************/

GlobalOp::GlobalOp(UINT64 Line,
                   UINT64 Timestamp,
                   INT32  CPU, INT32 AppId, INT32 TaskId, INT32 ThreadId,
                   INT32  CommunicatorId,
                   INT32  SendSize, INT32 RecvSize,
                   INT32  GlobalOpId,
                   INT32  RootTaskId)
:ParaverRecord(Line, Timestamp, CPU, AppId, TaskId, ThreadId)
{
  this->CommunicatorId = CommunicatorId;
  this->SendSize       = SendSize;
  this->RecvSize       = RecvSize;
  this->GlobalOpId     = GlobalOpId;
  this->RootTaskId     = RootTaskId; /* RootTaskIds are in range 0..(n-1) */
  
  if (this->RootTaskId == this->TaskId)
    this->Root = true;
  else
    this->Root = false;
}

GlobalOp::GlobalOp(UINT64 Line,
                   UINT64 Timestamp,
                   INT32 CPU, INT32 AppId, INT32 TaskId, INT32 ThreadId,
                   INT32 CommunicatorId,
                   INT32 SendSize, INT32 RecvSize,
                   INT32 GlobalOpId,
                   bool  Root)
:ParaverRecord(Line, Timestamp, CPU, AppId, TaskId, ThreadId)
{
  this->CommunicatorId = CommunicatorId;
  this->SendSize       = SendSize;
  this->RecvSize       = RecvSize;
  this->GlobalOpId     = GlobalOpId;
  this->Root           = Root;
  
  if (this->Root)
    this->RootTaskId = TaskId;
  else
    this->RootTaskId = UNKNOWN_ROOT;
}

bool
GlobalOp::FlushRecordType(FILE* OutputFile)
{
  if (fprintf(OutputFile, "%d", PARAVER_GLOBALOP) < 0)
  {
    SetError(true);
    SetErrorMessage("unable to print record type",
                    strerror(errno));
    return false;
  }
  return true;
}

bool
GlobalOp::FlushSpecificFields(FILE* OutputFile)
{
  if (fprintf(OutputFile,
              ":%d:%d:%d:%d:%d",
              CommunicatorId,
              SendSize,
              RecvSize,
              GlobalOpId,
              RootTaskId) < 0)
  {
    SetError(true);
    SetErrorMessage("unable to print global operation record fields",
                    strerror(errno));
    return false;
  }
  return true;
}

void
GlobalOp::Write( ostream& os) const
{
  os << "GlobalOP [";
  
  os.width(3);
  os.fill('0');
  os << TaskId << ":";

  os.width(2);
  os.fill('0');
  os << ThreadId << "] ";
  
  os << "T: " << Timestamp << " CommId " << CommunicatorId;
  
  os << " GlobOpId: " << GlobalOpId << " RootTask: " << RootTaskId << endl;
}

ostream& operator<< (ostream& os, const GlobalOp& GlobOp)
{
  GlobOp.Write(os);
  return os;
}
