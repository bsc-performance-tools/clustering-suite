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

  $Id::                                       $:  Id
  $Rev::                                          $:  Revision of last commit
  $Author::                                       $:  Author of last commit
  $Date::                                         $:  Date of last commit

\* -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=- */

#include <iostream>
#include <fstream>
using std::ofstream;
using std::cerr;
using std::endl;

#include <sstream>
using std::ostringstream;

#include <SystemMessages.hpp>
using cepba_tools::system_messages;

#include "TDBSCANWorkerOnline.h"
#include "TDBSCANTags.h"
#include "ClustersInfo.h"
#include "Utils.h"

#include "ClustersInfo.h"

/**
 * Constructor sets-up a callback to extract the clustering data from the tracing runtime.
 */
TDBSCANWorkerOnline::TDBSCANWorkerOnline(FetchCallback DataExtractCallback, FeedCallback DataFeedCallback)
{
    this->DataExtractCallback = DataExtractCallback;
    this->DataFeedCallback = DataFeedCallback;
}


/**
 * Initializes the clustering library for on-line use.
 * @param libClustering Instance of the clustering library.
 * @return true on success; false otherwise.
 */
bool TDBSCANWorkerOnline::InitLibrary(void)
{
  if (!libClustering->InitClustering (ClusteringDefinitionXML,
/*
                                      Epsilon,
                                      MinPoints,
*/
                                      (Protocol::WhoAmI() == 0), // true == Root task
                                      Protocol::WhoAmI(),
                                      Protocol::NumBackEnds()))
  {
    cerr << "[BE " << WhoAmI() << "] Error setting up clustering library: " << libClustering->GetErrorMessage() << endl;
    return false;
  }
  cepba_tools::system_messages::verbose                 = true;
  cepba_tools::system_messages::messages_from_all_ranks = false;
  cepba_tools::system_messages::print_timers            = true;
  return true;
}


/**
 * Feeds the clustering library with points from the on-line tracing buffers.
 * @param libClustering Instance of the clustering library.
 * @return true on success; false otherwise.
 */
bool TDBSCANWorkerOnline::ExtractData(void)
{
  ostringstream Messages;

  int tag, NumberOfDimensions=0, InputSize=0;
  vector<double> MinLocalDimensions, MaxLocalDimensions;
  double *MinGlobalDimensions=NULL, *MaxGlobalDimensions=NULL;
  PACKET_new(p);

  DataExtractCallback(libClustering);

  libClustering->GetParameterRanges(MinLocalDimensions, MaxLocalDimensions);

  /* DEBUG -- print the min/max local dimensions
  Messages.str("");
  Messages << "MinLocalDimensions = { ";
  for (unsigned int i=0; i<MinLocalDimensions.size(); i++)
  {
    Messages << MinLocalDimensions[i];
    if (i < MinLocalDimensions.size()-1)
    {
      Messages << ", ";
    }
  }
  Messages << " }" << endl;
  system_messages::information(Messages.str());

  Messages.str("");
  Messages << "MaxLocalDimensions = { ";
  for (unsigned int i=0; i<MaxLocalDimensions.size(); i++)
  {
    Messages << MaxLocalDimensions[i];
    if (i < MaxLocalDimensions.size()-1)
    {
      Messages << ", ";
    }
  }
  Messages << " }" << endl;
  system_messages::information(Messages.str());
  */

  /* Send the local dimensions range and receive the global range */
  STREAM_send(stXchangeDims, TAG_XCHANGE_DIMENSIONS, "%alf %alf",
    &MinLocalDimensions[0], MinLocalDimensions.size(),
    &MaxLocalDimensions[0], MaxLocalDimensions.size());

  STREAM_recv(stXchangeDims, &tag, p, TAG_XCHANGE_DIMENSIONS);

  PACKET_unpack(p, "%alf %alf", &MinGlobalDimensions, &NumberOfDimensions, &MaxGlobalDimensions, &NumberOfDimensions);

  vector<double> GlobalMin(MinGlobalDimensions, MinGlobalDimensions + NumberOfDimensions);
  vector<double> GlobalMax(MaxGlobalDimensions, MaxGlobalDimensions + NumberOfDimensions);

  for (size_t i = 0; i < NumberOfDimensions; i++)
  {
    GlobalMin[i] = MinGlobalDimensions[i];
    GlobalMax[i] = MaxGlobalDimensions[i];
  }

  /* DEBUG -- print the min/max global dimensions */
  Messages.str("");
  Messages << "MinGlobalDimensions = { ";
  for (unsigned int i=0; i<NumberOfDimensions; i++)
  {
    Messages << GlobalMin[i];
    if (i < NumberOfDimensions-1)
    {
      Messages << ", ";
    }
  }
  Messages << " }" << endl;
  system_messages::information(Messages.str());

  Messages.str("");
  Messages << "MaxGlobalDimensions = { ";
  for (unsigned int i=0; i<NumberOfDimensions; i++)
  {
    Messages << GlobalMax[i];
    if (i < NumberOfDimensions-1)
    {
      Messages << ", ";
    }
  }
  Messages << " }" << endl;
  system_messages::information(Messages.str());

  /* Normalize the input data with the global dimensions */
  libClustering->NormalizeData( GlobalMin, GlobalMax );

  xfree(MinGlobalDimensions);
  xfree(MaxGlobalDimensions);
  PACKET_delete(p);

  return true;
}


/**
 * Analyzes the data fetched from the tracing runtime.
 * @return true on success; false otherwise.
 */
bool TDBSCANWorkerOnline::AnalyzeData(void)
{
   if (!libClustering->ClusterAnalysis(LocalModel))
   {
      ostringstream Messages;
      Messages << "Error clustering data: " << libClustering->GetErrorMessage() << endl;
      system_messages::information(Messages.str(), stderr);
      return false;
   }
   return true;
}


/**
 * Prints the output plots.
 * @return true on success; false otherwise.
 */
bool TDBSCANWorkerOnline::ProcessResults(Support &GlobalSupport)
{

#if 0
  ofstream LocalDataFile, GlobalDataFile;
  LocalDataFile.open (LocalModelDataFileName.c_str());

  /* Print the local model */
  for (unsigned int i=0; i<LocalModel.size(); i++)
  {
    if (!LocalModel[i]->Flush(LocalDataFile, MIN_CLUSTERID+i+PARAVER_OFFSET))
    {
      ostringstream Messages;
      Messages << "Error writing local hull #" << i;
      Messages << " data (density=" << GlobalModel[i]->Density() << "): ";
      Messages << libClustering->GetErrorMessage() << endl;
      system_messages::information(Messages.str(), stderr);
      return false;
    }
  }
  LocalDataFile.close();

  /* Print the global model */
  if (WhoAmI() == 0)
  {
    GlobalDataFile.open (GlobalModelDataFileName.c_str());

    for (unsigned int i=0; i<GlobalModel.size(); i++)
    {
      if (!GlobalModel[i]->Flush(GlobalDataFile, MIN_CLUSTERID+i+PARAVER_OFFSET))
      {
        ostringstream Messages;

        GlobalDataFile.close();

        Messages << "Error writing global hull #" << i;
        Messages << " data (density=" << GlobalModel[i]->Density() << "): ";
        Messages << libClustering->GetErrorMessage() << endl;
        system_messages::information(Messages.str(), stderr);
        return false;
      }
    }

    GlobalDataFile.close();
  }
#else
  ostringstream ModelTitle, Messages;
  cepba_tools::Timer t;

  vector<Point *>      Points;
/*
  vector<task_id_t>    TaskIDs;
  vector<thread_id_t>  ThreadIDs;
*/
  vector<timestamp_t>  BeginTimes;
  vector<timestamp_t>  EndTimes;
  vector<cluster_id_t> ClusterIDs;

  libClustering->GetFullBurstsInformation(Points, /* TaskIDs, ThreadIDs, */ BeginTimes, EndTimes, ClusterIDs);

  vector<int> BurstsSupport;
  GlobalSupport.GetSupport( BurstsSupport );


  DataFeedCallback( BeginTimes, EndTimes, ClusterIDs, BurstsSupport );

  if (WhoAmI() == 0)
  {
    /* Print the global model (only 1 back-end) */
    system_messages::information ("Printing global model script\n");

    ModelTitle.str ("");
    ModelTitle << "Global Model MinPoints = " << MinPoints << " Eps = " << Epsilon;

    // fprintf(stderr, "[DEBUG PrintModels] %s %s\n", GlobalModelDataFileName.c_str(), GlobalModelPlotFileNamePrefix.c_str());

    if (!libClustering->PrintModels (GlobalModel,
                                     GlobalModelDataFileName,
                                     GlobalModelPlotFileNamePrefix,
                                     ModelTitle.str() ) )
    {
      Messages.str ("");
      Messages << "Error printing global model script: " << libClustering->GetErrorMessage() << endl;
      system_messages::information (Messages.str(), stderr);
      return false;
    }
  }

  /* Print the local model (all back-ends) */
  system_messages::information ("Printing local model\n");

  ModelTitle.str ("");
  ModelTitle << "Local Hull Models BE " << WhoAmI() << " MinPoints = " << MinPoints << " Eps = " << Epsilon;

  if (!libClustering->PrintModels (LocalModel,
                                   LocalModelDataFileName,
                                   LocalModelPlotFileNamePrefix,
                                   ModelTitle.str() ) )
  {
    Messages.str ("");
    Messages << "Error printing local model scripts: " << libClustering->GetErrorMessage() << endl;
    system_messages::information (Messages.str(), stderr);
    return false;
  }

  /* Print local clustering plots (initial clustering on each back-end) */
  system_messages::information ("Printing local data plot script\n");

  if (!libClustering->PrintPlotScripts (OutputLocalClusteringFileName, "", true) ) // true = Local partition
  {
    Messages.str ("");
    Messages << "Error printing local data plot scripts: " << libClustering->GetErrorMessage() << endl;
    system_messages::information (Messages.str(), stderr);
    return false;
  }

  /* Print the global clustering plots (classification using the global model) */
  system_messages::information ("Printing global data plot script\n");

  if (!libClustering->PrintPlotScripts (OutputGlobalClusteringFileName, "", false) ) // false = Global classification
  {
    Messages.str ("");
    Messages << "Error printing global data plot scripts: " << libClustering->GetErrorMessage() << endl;
    system_messages::information (Messages.str(), stderr);
    return false;
  }

  /* DEBUG: Print local clustering plots (initial clustering on each back-end) */
  system_messages::information ("Retrieving local statistics per cluster\n");

  vector<ClusterStatistics*> Statistics;

  if (!libClustering->GetClusterStatistics(Statistics))
  {
    Messages.str ("");
    Messages << "Error retrieving cluster statistics: " << libClustering->GetErrorMessage() << endl;
    system_messages::error (Messages.str());
    return false;
  }

  if (WhoAmI() == 0)
  {
    int k = 0;
    for (k=0; k<Statistics.size(); k++)
    {
      cerr << "Cluster " << k+1 << endl << *(Statistics[k]) << endl;
    }
  }

  Messages.str("");
  Messages << "Received statistics from " << Statistics.size() << " clusters" << endl;
  system_messages::information(Messages.str());

  ClustersInfo CInfoData (Statistics);
  CInfoData.Serialize(stClustering);
#endif

  return true;
}

