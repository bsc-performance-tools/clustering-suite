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

  $URL:: https://svn.bsc.es/repos/ClusteringSuite/trunk/src/libClustering/#$:

  $Rev:: 13                         $:  Revision of last commit
  $Author:: jgonzale                $:  Author of last commit
  $Date:: 2011-05-03 10:47:59 +0200#$:  Date of last commit

\* -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=- */


#include "ClusterInformation.hpp"
#include <ParaverColors.h>

#include <cstdio>

#include <sstream>
using std::ostringstream;
#include <iomanip>
using std::setfill;
using std::setw;
using std::setprecision;

/* PARAMETERS TO TUNE CANDIDATE GENERATION */
#define CANDIDATES_GENERAL_SCORE 0.9

#define CANDIDATES_OCCURENCES_PERCENTAGE 0.25
#define CANDIDATES_OCCURENCES_SCORE 0.6

#define CANDIDATES_TIME_PERCENTAGE 0.25
#define CANDIDATES_TIME_SCORE      0.6

node_id_t ClusterInformation::NodeIDNumber = 0;

/******************************************************************************
 * CLASS 'ClusterInformation'
 *****************************************************************************/

ClusterInformation::ClusterInformation(cluster_id_t ID,
                                       percentage_t Score,
                                       size_t       Occurrences,
                                       timestamp_t  TotalDuration,
                                       size_t       Individuals)
{
  this->NodeID           = NodeIDNumber++;
  this->ID               = ID;
  this->Score            = Score;
  this->Occurrences      = Occurrences;
  this->TotalDuration    = TotalDuration;
  this->Individuals      = Individuals;
  this->Discarded        = false;
  this->Reclassification = false;
}

bool ClusterInformation::IsCandidate(size_t      TotalOccurrences,
                                     timestamp_t ClustersTotalDuration)
{
  if (Discarded)
  {
    return false;
  }
  
  if (Occurrences > 1 && Score > CANDIDATES_GENERAL_SCORE)
  {
    return true;
  }

  if ( (TotalDuration/ClustersTotalDuration) > CANDIDATES_TIME_PERCENTAGE &&
       Score > CANDIDATES_TIME_SCORE)
  {
    return true;
  }
  
  if ( (Occurrences/TotalOccurrences) > CANDIDATES_OCCURENCES_PERCENTAGE &&
        Score > CANDIDATES_OCCURENCES_SCORE)
  {
    return true;
  }
    
  return false;
}

size_t ClusterInformation::TotalClusters(void)
{
  size_t Result = 0;

  if (Discarded)
  {
    return Result;
  }
    
  if (Children.size() == 0)
  {
    Result++;
    return Result;
  }

  for (size_t i = 0; i < Children.size(); i++)
  {
    if (Children[i]->GetID() != NOISE_CLUSTERID &&
        !Children[i]->IsDiscarded())
    {
      Result++;
    }
  }

  return Result;
}

void ClusterInformation::RenameChildren(cluster_id_t& RestOfChildrenID)
{
  bool FirstChildVisited = false;
  
  for (size_t i = 0; i < Children.size(); i++)
  {
    if (Children[i]->GetID() != NOISE_CLUSTERID)
    {
      if (!FirstChildVisited)
      {
        Children[i]->SetID(ID);
        FirstChildVisited = true;
      }
      else
      {
        Children[i]->SetID(RestOfChildrenID);
        RestOfChildrenID++;
      }
    }
  }
  
  return;
}

bool ClusterInformation::IsLeaf(void)
{
  if (Children.size() == 0)
  {
    return true;
  }

  if (Children[0]->IsDiscarded())
  {
    return true;
  }

  return false;
}

bool ClusterInformation::AddChild(ClusterInformation* NewChild)
{
  Children.push_back(NewChild);
  
  return true;
}

string ClusterInformation::NodeName(void)
{
  ostringstream NodeNameStr;
  NodeNameStr << "node" << NodeID;
  return NodeNameStr.str();
}

string ClusterInformation::NodeLabel(void)
{
  ostringstream NodeLabelStr;
  
  NodeLabelStr << " [label=\"";

  if (ID == NOISE_CLUSTERID)
  {
    NodeLabelStr << "Noise";
  }
  else
  {
    NodeLabelStr << "Cl. " << ID;
  }

  if (!Reclassification)
  {
    NodeLabelStr << " \\n Score = " << setw(3) << Score*100 << "%\\n";
    NodeLabelStr << " Indiv. = " << Individuals;
    NodeLabelStr << " Occr. = " << Occurrences << "\\n";
    NodeLabelStr << " Duration = " << TotalDuration;
  }
  else
  {
    if (ID == NOISE_CLUSTERID)
    {
      NodeLabelStr << "\\n Re-Classified Noise";
    }
    else
    {
      NodeLabelStr << "\\n Nodes Added on Re-Classification\\n";
      NodeLabelStr << "Indiv. = " << Individuals;
    }
  }
  
  if (Discarded)
  {
    NodeLabelStr << "\" color=\"" << Color() << "\"]";
  }
  else
  {
    NodeLabelStr << "\" style=\"filled\" color=\"" << Color() << "\"]";
  }

  

  return NodeLabelStr.str();
}

string ClusterInformation::Color(void)
{
  char RGBColor[8];

  if (ID+PARAVER_OFFSET < DEF_NB_COLOR_STATE)
  {

    if ((ParaverDefaultPalette[ID+PARAVER_OFFSET].RGB[0] == 0xFF) &&
        (ParaverDefaultPalette[ID+PARAVER_OFFSET].RGB[1] == 0xFF) &&
        (ParaverDefaultPalette[ID+PARAVER_OFFSET].RGB[2] == 0xFF))
    {
      sprintf(RGBColor, "#000000");
    }
    else
    {
      sprintf(RGBColor, "#%02x%02x%02x",
        ParaverDefaultPalette[ID+PARAVER_OFFSET].RGB[0],
        ParaverDefaultPalette[ID+PARAVER_OFFSET].RGB[1],
        ParaverDefaultPalette[ID+PARAVER_OFFSET].RGB[2]);
    }
  }
  else
  {
    sprintf(RGBColor, "#%02x%02x%02x",
      ParaverDefaultPalette[DEF_NB_COLOR_STATE-1].RGB[0],
      ParaverDefaultPalette[DEF_NB_COLOR_STATE-1].RGB[1],
      ParaverDefaultPalette[DEF_NB_COLOR_STATE-1].RGB[2]);
  }

  return RGBColor;
}

