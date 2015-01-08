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

  $Id::                                    $:  Id
  $Rev::                                          $:  Revision of last commit
  $Author::                                       $:  Author of last commit
  $Date::                                         $:  Date of last commit

\* -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=- */

#include <algorithm>
#include <iostream>
#include <sstream>
using std::ostringstream;
#include <fstream>
using std::ofstream;
#include <SystemMessages.hpp>
using cepba_tools::system_messages;

#include <FileNameManipulator.hpp>
using cepba_tools::FileNameManipulator;

#include <libDistributedClustering.hpp>

#include "TDBSCANRoot.h"
#include "NoiseManager.h"
#include "HullManager.h"
#include "TDBSCANTags.h"
#include "Utils.h"
#include "Support.h"


/**
 * Constructor sets the clustering configuration parameters.
 */
TDBSCANRoot::TDBSCANRoot (
  double Eps,
  int    MinPts,
  string ClusteringDefinitionXML,
  string InputTraceName,
  string OutputFileName,
  bool   Verbose,
  bool   ReconstructTrace)
{

  this->ClusteringDefinitionXML = ClusteringDefinitionXML;
  this->InputTraceName          = InputTraceName;
  this->OutputFileName          = OutputFileName;
  this->Verbose                 = Verbose;
  this->ReconstructTrace        = ReconstructTrace;
  system_messages::verbose      = Verbose;

  libClustering = new libDistributedClustering(Verbose, "FE");
  libClustering->InitClustering(ClusteringDefinitionXML, true, 0, 1);

  if (Eps == -1)
  {
    this->Epsilon = libClustering->GetEpsilon();
  }
  else
  {
    this->Epsilon = Eps;
  }
  if (MinPts == -1)
  {
    this->MinPoints = libClustering->GetMinPoints();
  }
  else
  {
    this->MinPoints = MinPts;
  }
}

TDBSCANRoot::TDBSCANRoot (
  string ClusteringDefinitionXML,
  bool   Verbose)
{
  this->ClusteringDefinitionXML = ClusteringDefinitionXML;
  this->Verbose                 = Verbose;

  libClustering = new libDistributedClustering(Verbose, "FE");
  libClustering->InitClustering(ClusteringDefinitionXML, true, 0, 1);

  this->Epsilon                 = libClustering->GetEpsilon();
  this->MinPoints               = libClustering->GetMinPoints();
}



/**
 * Register the streams and load the filters used in the TDBSCAN protocol.
 */
void TDBSCANRoot::Setup()
{
  stClustering = Register_Stream ("TDBSCAN", SFILTER_DONTWAIT);
  stClustering->set_FilterParameters (FILTER_UPSTREAM_TRANS, "%lf %d", Epsilon, MinPoints);
  stXchangeDims = Register_Stream ("XchangeDimensions", SFILTER_WAITFORALL);
  stSupport = Register_Stream("Support", SFILTER_WAITFORALL);
}


/**
 * Front-end side of the TDBSCAN clustering algorithm.
 */
int TDBSCANRoot::Run()
{
  ostringstream Messages;
  //ostringstream MergedDataFileNames;

  int       countGlobalHulls = 0;
  int       tag;
  PacketPtr p;

  string OutputFileExtension;

  OutputFileExtension = FileNameManipulator::GetExtension (OutputFileName);
  FileNameManipulator NameManipulator (OutputFileName, OutputFileExtension);
  OutputPrefix = NameManipulator.GetChoppedFileName();

  Messages << "[FE] Sending clustering configuration:"                       << endl;
  Messages << "[FE] + Epsilon     = " << Epsilon                             << endl;
  Messages << "[FE] + Min Points  = " << MinPoints                           << endl;
  Messages << "[FE] + XML         = " << ClusteringDefinitionXML             << endl;

  if (InputTraceName != "")
  {
    Messages << "[FE] + Input       = " << InputTraceName                      << endl;
  }

  Messages << "[FE] + Output      = " << OutputFileName                      << endl;
  Messages << "[FE] + Verbose     = " << ( Verbose          ? "yes" : "no" ) << endl;
  Messages << "[FE] + Reconstruct = " << ( ReconstructTrace ? "yes" : "no" ) << endl;
  Messages << endl;

  /* DEBUG: That should be tuned by parameter */
  system_messages::verbose = true;
  system_messages::messages_from_all_ranks = true;

  system_messages::information (Messages.str() );

  /* Send the clustering configuration to the back-ends */
  Send_Configuration();

  Messages.str ("");
  Messages << "[FE] Computing global dimensions..." << endl;
  system_messages::information (Messages.str() );

  /* Receive and broadcast back the global dimensions */
  double *MinGlobalDimensions=NULL, *MaxGlobalDimensions=NULL;
  int     NumberOfDimensions;

  MRN_STREAM_RECV (stXchangeDims, &tag, p, TAG_XCHANGE_DIMENSIONS);
  PACKET_unpack(p, "%alf %alf", &MinGlobalDimensions, &NumberOfDimensions, &MaxGlobalDimensions, &NumberOfDimensions);

  stXchangeDims->send (p);

  Messages.str ("");
  Messages << "[FE] In barrier while back-ends are clustering...." << endl;
  system_messages::information (Messages.str() );

  Barrier();

  Messages.str ("");
  Messages << "[FE] Computing global hulls..." << endl;
  system_messages::information (Messages.str() );

  do
  {
    MRN_STREAM_RECV (stClustering, &tag, p, TAG_ANY);
#if 0

    /* Receive the resulting global hulls one by one */
    if (tag == TAG_HULL)
    {
      HullModel *GlobalHull = NULL;
      HullManager HM        = HullManager();

      GlobalHull = HM.Unpack (p);
      GlobalModel.push_back ( GlobalHull );

      /* Broadcast back the global hull */
      stClustering->send (p);
      countGlobalHulls ++;

      /* DEBUG
      std::cout << "********** [FE] BROADCASTING HULL " << countGlobalHulls << std::endl;
      std::cout << "Current Hull size = " << GlobalHull->Size() << " density = " << GlobalHull->Density() << std::endl;
      GlobalHull->Flush();
      std::cout << "********** [FE] END BROADCASTING HULL " << countGlobalHulls << std::endl; */
    }

#endif

    /* Receive 1 packet with all resulting global hulls at once */
    if (tag == TAG_ALL_HULLS)
    {
      HullManager HM = HullManager();
      HM.Unpack (p, GlobalModel);
      vector<HullModel *> FilteredGlobalModel;

      for (unsigned int i = 0; i < GlobalModel.size(); i++)
      {
        if (GlobalModel[i]->Density() >= MinPoints)
        {
          FilteredGlobalModel.push_back ( GlobalModel[i] );
        }
      }

      /* Broadcast back the global model */
      countGlobalHulls = FilteredGlobalModel.size();

      /* Sort hulls by their aggregated total time */
      std::sort(FilteredGlobalModel.begin(), FilteredGlobalModel.end(), SortHullsByTime());

      HM.SerializeAll (stClustering, FilteredGlobalModel);
    }

#if defined(PROCESS_NOISE)
    /* Count the remaining noise points */
    else if (tag == TAG_NOISE)
    {
      vector<const Point *> NoisePoints;
      vector<long long>     NoiseDurations;

      NoiseManager Noise = NoiseManager();
      Noise.Unpack (p, NoisePoints, NoiseDurations);

      Messages.str ("");
      Messages << "[FE] Remaining noise points = " << NoisePoints.size() << endl;
      system_messages::information (Messages.str() );
    }
#endif
  }
  while (tag != TAG_ALL_HULLS_SENT);

  stClustering->send (TAG_ALL_HULLS_SENT, "");

  Messages.str ("");
  Messages << "[FE] Broadcasted " << countGlobalHulls << " global hulls!" << endl;
  system_messages::information (Messages.str() );

  /* Write the final aggregate scatter plot */
  Messages.str ("");
  Messages << "Printing full clustering scripts" << endl;
  system_messages::information (Messages.str() );

  string FinalDataFileName = OutputPrefix + ".FINAL.DATA.csv";
#if 0
  MergedDataFileNames << "head -n 1 " << OutputPrefix << ".GLOBAL_CLUSTERING_0.csv ";
  MergedDataFileNames << "> " + FinalDataFileName + "; ";
  MergedDataFileNames << "tail -qn +2 ";

  for (unsigned int i = 0; i < NumBackEnds(); i++)
  {
    MergedDataFileNames << OutputPrefix << ".GLOBAL_CLUSTERING_" << i << ".csv ";
  }
  MergedDataFileNames << ">> " + FinalDataFileName + "; ";  
#endif

  if (!libClustering->PrintGlobalPlotScripts(
    FinalDataFileName,
    OutputPrefix + ".FINAL",
    countGlobalHulls)
  )
  {
    cout << "Error printing full clustering scripts: " << libClustering->GetErrorMessage() << endl;
  }


  /* Receive the support */
  MRN_STREAM_RECV (stSupport, &tag, p, TAG_SUPPORT);
  Support GlobalSupport(NumberOfDimensions, MinGlobalDimensions, MaxGlobalDimensions);
  GlobalSupport.Unpack(p);
  GlobalSupport.Serialize(stSupport);
  //GlobalSupport.dump();
  GlobalSupport.plot2(OutputPrefix + ".FINAL.support.csv");

  /* Receive the averaged clusters info stats */
  ClustersInfo ClustersStats;
  MRN_STREAM_RECV (stClustering, &tag, p, TAG_CLUSTERS_INFO);
  ClustersStats.Unpack (p);
  //ClustersStats.Print();
  //cout << ClustersStats;
  ofstream ClustersInfoFile;
  ClustersInfoFile.open (string(OutputPrefix + ".FINAL.clusters_info.csv").c_str());
  ClustersInfoFile << ClustersStats;
  ClustersInfoFile.close();

  /* Receive the statistics from all nodes */
  Statistics NetworkStats (WhoAmI(), false);
  MRN_STREAM_RECV (stClustering, &tag, p, TAG_STATISTICS);
  NetworkStats.Unpack (p);
  PrintGraphStats (NetworkStats);

  /* Write the final DATA file */
  cout << "Concatenating data files..." << endl;
  ostringstream ConcatCMD;
  ConcatCMD << getenv("TDBSCAN_HOME") << "/bin/concat-csv " << NumBackEnds() << " " << OutputPrefix << ".GLOBAL_CLUSTERING_" << " " << FinalDataFileName; 
  cout << ConcatCMD.str() << endl;
  cout.flush();
  system( ConcatCMD.str().c_str() );

  xfree(MinGlobalDimensions);
  xfree(MaxGlobalDimensions);

  return countGlobalHulls;
}

void TDBSCANRoot::PrintGraphStats (Statistics &NetworkStats)
{
  ostringstream Messages;

  string TreeLayoutFileName = OutputPrefix + ".MRNETSTATS.layout";
  GetNetwork()->get_NetworkTopology()->print_DOTGraph ( TreeLayoutFileName.c_str() );

  string StatsFileName = OutputPrefix + ".MRNETSTATS.data";
  ofstream StatsFile;
  StatsFile.open (StatsFileName.c_str() );
  NetworkStats.DumpAllStats (StatsFile);
  StatsFile.close();

  string OutputDOTName = OutputPrefix + ".MRNETSTATS.dot";
  string CMD = string (getenv ("TDBSCAN_HOME") ) + "/bin/draw_stats " + TreeLayoutFileName + " " + StatsFileName + " " + OutputDOTName;

  Messages << "[FE] Generating debug statistics... ";
  system_messages::information (Messages.str() );

  system (CMD.c_str() );

  Messages.str ("");
  Messages << "done!" << endl;
  system_messages::information (Messages.str() );

  //unlink (TreeLayoutFileName.c_str() );
  //unlink (StatsFileName.c_str() );
}

