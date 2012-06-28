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

  $Id:: BurstClustering.cpp 57 2011-12-13 14:29:3#$:  Id
  $Rev:: 57                                       $:  Revision of last commit
  $Author:: jgonzale                              $:  Author of last commit
  $Date:: 2011-12-13 15:29:33 +0100 (Tue, 13 Dec #$:  Date of last commit

\* -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=- */

#include "MirkinDistance.hpp"

#include <trace_clustering_types.h>
#include <SystemMessages.hpp>
using cepba_tools::system_messages;

#include <cerrno>
#include <cstring>

#include <iostream>
#include <iterator>

#include <sstream>
using std::ostringstream;
using std::istringstream;
using std::endl;

#include <limits>
using std::numeric_limits;

#include <algorithm>
using std::set_intersection;

bool MirkinDistance::GetMirkinDistance(string  ClustersFileName1,
                                       string  ClustersFileName2,
                                       bool&   UseNoise,
                                       double& Distance)
{
  ostringstream Messages;

  map<cluster_id_t, vector<instance_t> >           ClustersSet1;
  map<cluster_id_t, vector<instance_t> >::iterator ClustersSet1It;

  map<cluster_id_t, vector<instance_t> >           ClustersSet2;
  map<cluster_id_t, vector<instance_t> >::iterator ClustersSet2It;

  size_t C1Sum = 0, C2Sum = 0, C1C2Sum = 0, n = 0;

  this->ClustersFileName1 = ClustersFileName1;
  this->ClustersFileName2 = ClustersFileName2;

  system_messages::information("**** Parsing input files ****\n");

  /* Populate the sets representing each partition */
  if (!GenerateSets(ClustersSet1, ClustersSet2))
  {
    return false;
  }

  /* Compute the distance */
  for (ClustersSet1It  = ClustersSet1.begin();
       ClustersSet1It != ClustersSet1.end();
       ++ClustersSet1It)
  {
    if (UseNoise || ClustersSet1It->first-PARAVER_OFFSET != NOISE_CLUSTERID)
    {
      C1Sum += (ClustersSet1It->second.size() * ClustersSet1It->second.size());
      n += ClustersSet1It->second.size();
    }
  }

  Messages << "C Sum            = " << C1Sum << " (" << ClustersSet1.size() << ")" << endl;
  system_messages::information(Messages.str());

  for (ClustersSet2It  = ClustersSet2.begin();
       ClustersSet2It != ClustersSet2.end();
       ++ClustersSet2It)
  {
    if (UseNoise || ClustersSet2It->first-PARAVER_OFFSET != NOISE_CLUSTERID)
    {
      C2Sum += (ClustersSet2It->second.size() * ClustersSet2It->second.size());
    }
  }

  /* DEBUG */
  Messages.str("");
  Messages << "C' Sum           = " << C2Sum << " (" << ClustersSet2.size() << ")" << endl;
  system_messages::information(Messages.str());

  for (ClustersSet1It  = ClustersSet1.begin();
       ClustersSet1It != ClustersSet1.end();
       ++ClustersSet1It)
  {
    for (ClustersSet2It  = ClustersSet2.begin();
         ClustersSet2It != ClustersSet2.end();
         ++ClustersSet2It)
    {
      cluster_id_t C1Id = ClustersSet1It->first-PARAVER_OFFSET;
      cluster_id_t C2Id = ClustersSet2It->first-PARAVER_OFFSET;

      vector<instance_t>& C1n = ClustersSet1It->second;
      vector<instance_t>& C2n = ClustersSet2It->second;

      vector<instance_t> Intersection;

      /* DEBUG
      Messages.str("");
      Messages << "C Cluster " << ClustersSet1It->first << " Size  = " << C1n.size() << " | ";
      Messages << "C' Cluster " << ClustersSet2It->first << " Size = " << C2n.size() << " | ";
      system_messages::information(Messages.str());
      */
      if (UseNoise || (C1Id != NOISE_CLUSTERID && C2Id != NOISE_CLUSTERID))
      {
        set_intersection(C1n.begin(), C1n.end(),
                         C2n.begin(), C2n.end(),
                         std::inserter(Intersection, Intersection.begin()));

        C1C2Sum += (Intersection.size() * Intersection.size());
      }

      /* DEBUG
      Messages << "Intersection size = " << Intersection.size() << endl;
      system_messages::information(Messages.str());
      */
    }
  }

  /* DEBUG */
  Messages.str("");
  Messages << "Intersection Sum = " << C1C2Sum << endl;
  system_messages::information(Messages.str());

  Distance = (C1Sum + C2Sum - (2*C1C2Sum)) / (double) (n*n);

  return true;
}

bool MirkinDistance::GenerateSets(map<cluster_id_t, vector<instance_t> >& ClustersSet1,
                                  map<cluster_id_t, vector<instance_t> >& ClustersSet2)
{
  ostringstream      ErrorMessage;
  string             Line;
  vector<string>     Record;

  ifstream::pos_type FirstPos;
  ifstream::pos_type EndPos;

  ClustersFile1.open(ClustersFileName1.c_str(), ifstream::in);

  if (ClustersFile1.fail())
  {
    ErrorMessage << "Unable to open file " << ClustersFileName1;
    SetError(true);
    SetErrorMessage(ErrorMessage.str().c_str(),
                    strerror(errno));
    return false;
  }

  ClustersFile2.open(ClustersFileName2.c_str(), ifstream::in);
  if (ClustersFile2.fail())
  {
    ErrorMessage << "Unable to open file " << ClustersFileName2;
    SetError(true);
    SetErrorMessage(ErrorMessage.str().c_str(),
                    strerror(errno));
    return false;
  }

  /* Check both headers */
  getline(ClustersFile1, Line);
  if (!ClustersFile1.good())
  {
    ErrorMessage << "Error reading header of file " << ClustersFileName1;
    SetError(true);
    SetErrorMessage(ErrorMessage.str().c_str(),
                    strerror(errno));
    return false;
  }

  PopulateRecord(Record, Line, ',');
  if (!CheckHeader (Record, ClustersFileName1))
    return false;

  getline(ClustersFile2, Line);
  if (!ClustersFile1.good())
  {
    ErrorMessage << "Error reading header of file " << ClustersFileName2;
    SetError(true);
    SetErrorMessage(ErrorMessage.str().c_str(),
                    strerror(errno));
    return false;
  }

  PopulateRecord(Record, Line, ',');
  if (!CheckHeader (Record, ClustersFileName2))
    return false;



  /* Load partition from file 1 */

  FirstPos = ClustersFile1.tellg();
  ClustersFile1.seekg(0, ifstream::end);
  EndPos = ClustersFile1.tellg();
  ClustersFile1.seekg(FirstPos);

  system_messages::show_percentage_progress("Loading file "+ClustersFileName1,
                                            0);

  while(getline(ClustersFile1, Line)  && ClustersFile1.good())
  {
    cluster_id_t CurrentClusterId;
    instance_t   CurrentInstance;

    PopulateRecord (Record, Line, ',');

    istringstream(Record[Record.size()-1]) >> CurrentClusterId;
    istringstream(Record[0])               >> CurrentInstance;

    ClustersSet1[CurrentClusterId].push_back(CurrentInstance);

    int Percentage = static_cast<int>(ClustersFile1.tellg()) * 100 / EndPos;

    system_messages::show_percentage_progress("Loading file '"+ClustersFileName1+"'",
                                              Percentage);
  }

  if (!ClustersFile1.eof())
  {
    ErrorMessage << "Error while loading " << ClustersFileName1;
    SetError(true);
    SetErrorMessage(ErrorMessage.str().c_str(),
                    strerror(errno));
    return false;
  }

  system_messages::show_percentage_end("Loading file '"+ClustersFileName1+"'");

  /* Load partition from file 2 */

  FirstPos = ClustersFile2.tellg();
  ClustersFile2.seekg(0, ifstream::end);
  EndPos = ClustersFile2.tellg();
  ClustersFile2.seekg(FirstPos);

  system_messages::show_percentage_progress("Loading file '"+ClustersFileName2+"'",
                                            0);

  while(getline(ClustersFile2, Line)  && ClustersFile2.good())
  {
    cluster_id_t CurrentClusterId;
    instance_t   CurrentInstance;

    PopulateRecord (Record, Line, ',');

    istringstream(Record[Record.size()-1]) >> CurrentClusterId;
    istringstream(Record[0])               >> CurrentInstance;

    ClustersSet2[CurrentClusterId].push_back(CurrentInstance);

    int Percentage = static_cast<int>(ClustersFile2.tellg()) * 100 / EndPos;

    system_messages::show_percentage_progress("Loading file '"+ClustersFileName2+"'",
                                              Percentage);
  }

  if (!ClustersFile2.eof())
  {
    ErrorMessage << "Error while loading " << ClustersFileName2;
    SetError(true);
    SetErrorMessage(ErrorMessage.str().c_str(),
                    strerror(errno));
    return false;
  }

  system_messages::show_percentage_end("Loading file '"+ClustersFileName2+"'");

  return true;
}


void MirkinDistance::PopulateRecord(vector<string> &Record,
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
      Record.push_back( curstring );
      curstring="";
    }
    else if (!inquotes && (c=='\r' || c=='\n') )
    {
      Record.push_back( curstring );
      return;
    }
    else
    {
      curstring.push_back(c);
    }
    linepos++;
  }
  Record.push_back( curstring );
  return;
}

bool MirkinDistance::CheckHeader(vector<string> &Record,
                                 string         &CurrentFileName)
{
  ostringstream ErrorMessage;

  if (Record.size() < 4)
  {
    ErrorMessage << "Wrong number of fields in file " << CurrentFileName;
    SetError(true);
    SetErrorMessage(ErrorMessage.str());
    return false;
  }

  if (Record[0].compare("# Instance") != 0)
  {
    ErrorMessage << "Wrong header in file " << CurrentFileName;
    SetError(true);
    SetErrorMessage(ErrorMessage.str());
    return false;
  }

  if (Record[Record.size()-1].compare("ClusterID") != 0)
  {
    ErrorMessage << "No clusters information in file " << CurrentFileName;
    SetError(true);
    SetErrorMessage(ErrorMessage.str());
    return false;
  }
  return true;
}
