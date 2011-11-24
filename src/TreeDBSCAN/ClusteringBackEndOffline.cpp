#include "ClusteringBackEndOffline.h"

#include <iostream>
using std::cout;
using std::cerr;
using std::endl;

/**
 * Initializes the clustering library for interactive use.
 * @param libClustering Instance of the clustering library.
 */
bool ClusteringBackEndOffline::InitLibrary(void)
{
   /* Initialize the clustering library */
   if (!libClustering->InitClustering(ClusteringDefinitionXML, Epsilon, MinPoints, (Protocol::WhoAmI() == 0), Protocol::WhoAmI(), Protocol::NumBackEnds())) // true == Root Task
   {
      cerr << "[BE " << Protocol::WhoAmI() << "] Error setting up clustering library: " << libClustering->GetErrorMessage() << endl;
      return false;
   }
   return true;
}


/**
 * Feeds the clustering library with points from a Paraver trace.
 * @param libClustering Instance of the clustering library.
 * @return true on success; false otherwise.
 */
bool ClusteringBackEndOffline::ExtractData(void)
{
   if (!libClustering->ExtractData(InputTraceName))
   {
      cerr << "[BE " << Protocol::WhoAmI() << "] Error extracting data: " << libClustering->GetErrorMessage() << endl;
      return false;
   }
   return true;
}


/**
 * Analyzes the data extracted from the Paraver trace.
 * @return true on success; false otherwise.
 */
bool ClusteringBackEndOffline::AnalyzeData(void)
{
   if (!libClustering->ClusterAnalysis(LocalModel))
   {
      cerr << "[BE " << WhoAmI() << "] Error clustering data: " << libClustering->GetErrorMessage() << endl;
      return false;
   }
   return true;
}

