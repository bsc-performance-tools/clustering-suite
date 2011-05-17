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


#ifndef CLUSTERINGPARAMETER_H
#define CLUSTERINGPARAMETER_H

#include <iostream>
using std::ostream;
#include <string>
using std::string;
#include <vector>
using std::vector;

#include <trace_clustering_types.h>
#include "Error.hpp"
using cepba_tools::Error;

/* ClusteringParameter class, top class in the hierarchy **********************/
class ClusteringParameter: public Error
{
  protected:
    string ParameterName;
    double Factor;
    bool   ApplyLog;
    bool   Ready;
    bool   Used;
    double RangeMin;
    double RangeMax;

  public:
    ClusteringParameter(void){};
    ClusteringParameter(string ParameterName, 
                        double Factor,
                        bool   ApplyLog,
                        double RangeMin,
                        double RangeMax);

    virtual ~ClusteringParameter(void){};

    virtual string GetParameterName(void) { return ParameterName; };
    virtual double GetFactor(void)        { return Factor; };
    virtual bool   GetApplyLog(void)      { return ApplyLog; };

    virtual void SetReady(bool Ready) { this->Ready = Ready; };
    virtual bool IsReady(void)        { return this->Ready; };
    virtual void Clear(void) = 0;

    virtual void NewData(event_type_t EventType, event_value_t EventValue) = 0;

    virtual bool HighPrecision(void)
    {
      return false;
    }

    virtual double GetRawMetric(void) = 0;
    virtual double GetMetric(void) = 0;
  
    virtual bool IsRangeFiltered(void) = 0;
  
    virtual bool IsDuration(void) { return false; }

    virtual ostream& Write(ostream& os) = 0;
};

typedef ClusteringParameter* ClusteringParameter_t;

ostream& operator<< (ostream& os, ClusteringParameter& ClusteringParameterObj);


/* SingleMetric class, represents single trace event parameter ****************/
class SingleEvent: public virtual ClusteringParameter
{
  private:
    event_type_t  EventType;
    INT32         EventPosition; // When reading parameters from a vector
    event_value_t EventValue;

  public:
    SingleEvent(string        ParameterName,
                double        Factor,
                bool          ApplyLog,
                event_type_t  EventType,
                double        RangeMin = -1.0,
                double        RangeMax = -1.0);

    ~SingleEvent(void) {};

    void Clear(void);

    event_type_t  GetEventType(void) { return EventType; };
    event_value_t GetEventValue(void) { return (Ready)? EventValue:-1; };

    void NewData(event_type_t EventType, event_value_t EventValue);
    void NewData(vector<event_value_t> EventValues);
  
    bool IsRangeFiltered(void);

    double GetRawMetric(void);
    double GetMetric(void);

    ostream& Write(ostream& os);
};

typedef SingleEvent* SingleEvent_t;


class SingleEventCompare {
  public:
    bool operator()(SingleEvent* Evt1, SingleEvent* Evt2) {
      return (Evt1->GetEventType() < Evt2->GetEventType());
    }
};
/* MixedEvents class, represents a derived metric from two trace events ********/
class MixedEvents: public virtual ClusteringParameter
{
  private:
    derived_event_op_t  Operation;
    event_type_t        EventTypeA,  EventTypeB;
    event_type_t        EventPositionA, EventPositionB; // When reading parameters from a vector
    event_value_t       EventValueA, EventValueB;
    bool   EventARead,  EventBRead;
    double Result;

  public:
    MixedEvents(string             ParameterName,
                double             Factor,
                bool               ApplyLog,
                event_type_t       EventTypeA,
                event_type_t       EventTypeB,
                derived_event_op_t Operation,
                double             RangeMin = -1.0,
                double             RangeMax = -1.0);


    ~MixedEvents(void) {};

    void Clear(void);

    event_type_t  GetEventTypeA(void) { return EventTypeA; };
    event_value_t GetEventValueA(void) { return (Ready)? EventValueA:-1; };

    event_type_t  GetEventTypeB(void) { return EventTypeB; };
    event_value_t GetEventValueB(void) { return (Ready)? EventValueB:-1; };

    char GetOperation(void)
    {
      switch(Operation)
      {
        case Add:
          return '+';
          break;
        case Substract:
          return '-';
          break;
        case Multiply:
          return '*';
          break;
        case Divide:
          return '/';
          break;
        default: /* This branch must be unreacheable */
          return 'E';
          break;
      }
    }

    void NewData(event_type_t EventType, event_value_t EventValue);
    void NewData(vector<event_value_t> EventValues);
  
    bool IsRangeFiltered(void);
  
    void SetReady(bool Ready);

    bool HighPrecision(void)
    {
      if (Operation == Divide)
        return true;
      else
        return false;
    }

    double GetRawMetric(void);
    double GetMetric(void);

    ostream& Write(ostream& os);
};

typedef MixedEvents* MixedEvents_t;
#endif /* CLUSTERINGPARAMETER_H */
