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

#include "ClusteringCore.h"
#include "ClusteringTags.h"
#include "Utils.h"


/** 
 * Constructor sets default configuration values.
 */
ClusteringCore::ClusteringCore()
{
   Epsilon                 = 0.015;
   MinPoints               = 10;
   ClusteringDefinitionXML = "";
   InputTraceName          = "";
   OutputFileName          = "OUTPUT.prv";
   Verbose                 = false;
   ReconstructTrace        = false;
   stClustering            = NULL;
   stXchangeDims           = NULL;
   GlobalModel.clear();
}


/**
 * Front-end call to send the clustering parameters to the back-ends.
 */
void ClusteringCore::Send_Configuration(void)
{
   stClustering->send(TAG_CLUSTERING_CONFIG, "%lf %d %s %s %s %d %d",
      Epsilon,
      MinPoints,
      ClusteringDefinitionXML.c_str(),
      InputTraceName.c_str(),
      OutputFileName.c_str(),
      ((int)Verbose),
      ((int)ReconstructTrace));
}


/**
 * Back-end call to receive the clustering parameters from the front-end.
 */
void ClusteringCore::Recv_Configuration(void)
{
   char *XML, *Input, *Output;
   int tag, Verb, Reconstruct;
   PACKET_new(p);

   /* Receive clustering configuration from the front-end */
   STREAM_recv(stClustering, &tag, p, TAG_CLUSTERING_CONFIG);
   PACKET_unpack(p, "%lf %d %s %s %s %d %d", &Epsilon, &MinPoints, &XML, &Input, &Output, &Verb, &Reconstruct);
   PACKET_delete(p);

   MinPoints               = 3;
   ClusteringDefinitionXML = string(XML);
   InputTraceName          = string(Input);
   OutputFileName          = string(Output);
   Verbose                 = (Verb == 1);
   ReconstructTrace        = (Reconstruct == 1);
   xfree(XML); xfree(Input); xfree(Output);
}

