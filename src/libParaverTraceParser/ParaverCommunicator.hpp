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

#ifndef _PARAVERCOMMUNICATOR_H
#define _PARAVERCOMMUNICATOR_H

#include <types.h>

#include <ostream>
using std::ostream;
using std::endl;
#include <set>
using std::set;

#include "Error.hpp"
using cepba_tools::Error;

class Communicator: public Error
{
  protected:

    INT32       CommunicatorId;
    INT32       ApplicationId;
    set<INT32>  CommunicatorTasks;
    bool        COMM_SELF;
    bool        COMM_WORLD;
  
    bool        isIntercommunicator = false;
    int         Intracomm1;
    int         Intracomm2;
    int         Leader1;
    int         Leader2;

  public:
    Communicator(char* ASCIICommunicator);
    Communicator(const Communicator& Comm);

    INT32 GetCommunicatorId(void) { return CommunicatorId; };
    INT32 GetApplictionId(void)   { return ApplicationId; };
    INT32 GetCommunicatorSize(void)
    {
      return (INT32) CommunicatorTasks.size();
    };
    set<INT32> GetCommunicatorTasks(void) { return CommunicatorTasks; };

    /* INT32 GetTaskId(UINT32 Index) { return CommunicatorTasks[Index]; }; */

    bool IsCOMM_SELF(void)              { return COMM_SELF; };
    bool IsCOMM_WORLD(void)             { return COMM_WORLD; };
    bool SetCOMM_WORLD(bool COMM_WORLD) { return (this->COMM_WORLD = COMM_WORLD); };

    void AddTask(INT32 TaskId);

    bool IsTaskIncluded(INT32 TaskId);

    bool Flush(FILE* OutputFile);

    void Write ( ostream& os) const;

  private:
    bool ParseTaskList(char* ASCIITaskList);
};
typedef Communicator* Communicator_t;

ostream& operator<< (ostream& os, const Communicator& Comm);

#endif /* _PARAVERCOMMUNICATOR_H */
