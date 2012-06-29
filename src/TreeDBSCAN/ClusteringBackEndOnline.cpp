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

#include "ClusteringBackEndOnline.h"
#include "ClusteringTags.h"
#include "Utils.h"
#include <iostream>
#include <fstream>
using std::ofstream;
using std::cout;
using std::cerr;
using std::endl;

/**
 * Constructor sets-up a callback to extract the clustering data from.
 */
ClusteringBackEndOnline::ClusteringBackEndOnline(CallbackType DataExtractCallback)
{
    this->DataExtractCallback = DataExtractCallback;
}


/**
 * Initializes the clustering library for on-line use.
 * @param libClustering Instance of the clustering library.
 * @return true on success; false otherwise.
 */
bool ClusteringBackEndOnline::InitLibrary(void)
{
   if (!libClustering->InitClustering(Epsilon, MinPoints))
   {
      cerr << "[BE " << WhoAmI() << "] Error setting up clustering library: " << libClustering->GetErrorMessage() << endl;
      return false;
   }
   return true;
}


/**
 * Feeds the clustering library with points from the on-line tracing buffers.
 * @param libClustering Instance of the clustering library.
 * @return true on success; false otherwise.
 */
bool ClusteringBackEndOnline::ExtractData(void)
{
   int tag, NumberOfDimensions=0, InputSize=0;
   vector<double> MinLocalDimensions, MaxLocalDimensions;
   double *MinGlobalDimensions=NULL, *MaxGlobalDimensions=NULL;
   PACKET_new(p);

   InputSize = DataExtractCallback(ExternalPoints, MinLocalDimensions, MaxLocalDimensions);
   cout << "[BE " << WhoAmI() << "] Bursts to analyze: " << ExternalPoints.size() << endl;

   STREAM_send(stXchangeDims, TAG_XCHANGE_DIMENSIONS, "%alf %alf", 
      &MinLocalDimensions[0], MinLocalDimensions.size(),
      &MaxLocalDimensions[0], MaxLocalDimensions.size());
   STREAM_recv(stXchangeDims, &tag, p, TAG_XCHANGE_DIMENSIONS);
   PACKET_unpack(p, "%alf %alf", &MinGlobalDimensions, &NumberOfDimensions, &MaxGlobalDimensions, &NumberOfDimensions);

   /* DEBUG -- print the min/max global dimensions */
   if (WhoAmI() == 0)
   {
      cout << "[BE " << WhoAmI() << "] MinGlobalDimensions = { ";
      for (unsigned int i=0; i<NumberOfDimensions; i++)
      {
         cout << MinGlobalDimensions[i];
         if (i < NumberOfDimensions-1) cout << ", ";
      }
      cout << " }" << endl;
      cout << "[BE " << WhoAmI() << "] MaxGlobalDimensions = { ";
      for (unsigned int i=0; i<NumberOfDimensions; i++)
      {
         cout << MaxGlobalDimensions[i]; 
         if (i < NumberOfDimensions-1) cout << ", ";
      }
      cout << " }" << endl;
   }

   /* Normalize the input data with the global dimensions */
   Normalize(MinGlobalDimensions, MaxGlobalDimensions);

   xfree(MinGlobalDimensions);
   xfree(MaxGlobalDimensions);
   PACKET_delete(p);

   return true;
}

void ClusteringBackEndOnline::Normalize(double *MinGlobalDimensions, double *MaxGlobalDimensions)
{
   /* Iterate points */
   for (unsigned int i=0; i<ExternalPoints.size(); i++)
   {
      Point *p = ExternalPoints[i];
      /* Iterate dimensions */
      for (unsigned int j=0; j<p->size(); j++)
      {
         (*p)[j] = (((*p)[j] - MinGlobalDimensions[j]) / (MaxGlobalDimensions[j] - MinGlobalDimensions[j]));
      }
   }
}


/**
 * Analyzes the data from a tracing faciliy.
 * @return true on success; false otherwise.
 */
bool ClusteringBackEndOnline::AnalyzeData(void)
{
   if (!libClustering->ClusterAnalysis((vector<const Point*>&) ExternalPoints, LocalModel))
   {
      cerr << "[BE " << WhoAmI() << "] Error clustering data: " << libClustering->GetErrorMessage() << endl;
      return false;
   }
   return true;
}


/**
 * Prints the output plots.
 * @return true on success; false otherwise.
 */
bool ClusteringBackEndOnline::ProcessResults(void)
{
  ofstream LocalDataFile, GlobalDataFile;
  LocalDataFile.open (LocalModelDataFileName.c_str());
  /* Print the local model */
  for (unsigned int i=0; i<LocalModel.size(); i++)
  {
    if (!LocalModel[i]->Flush(LocalDataFile, MIN_CLUSTERID+i+PARAVER_OFFSET))
    {
      cerr << "[BE " << WhoAmI() << "] Error writing local hull #" << i;
      cerr << " data (density=" << GlobalModel[i]->Density() << "): ";
      cerr << libClustering->GetErrorMessage() << endl;
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
        GlobalDataFile.close();
        cerr << "[BE " << WhoAmI() << "] Error writing global hull #" << i;
        cerr << " data (density=" << GlobalModel[i]->Density() << "): ";
        cerr << libClustering->GetErrorMessage() << endl;
        return false;
      }
    }
    
    GlobalDataFile.close();
  }

  return true;
}

