#include "ClusteringBackEndOffline.h"
#include "ClusteringTags.h"

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
   int tag;
   PACKET_new(p);

   if (!libClustering->ExtractData(InputTraceName))
   {
      cerr << "[BE " << Protocol::WhoAmI() << "] Error extracting data: " << libClustering->GetErrorMessage() << endl;
      return false;
   }
   cout << "[BE " << WhoAmI() << "] Bursts to analyze: " << libClustering->GetNumberOfPoints() << endl;

   /* In the offline version there's no need to reduce the dimensions, because all back-ends get them from 
    * the trace they're parsing. We have to send something though, because the front-end is waiting this 
    * message, as it makes no distinction between online/offline back-ends.
    */
   MRN_STREAM_SEND(stXchangeDims, TAG_XCHANGE_DIMENSIONS, "%alf %alf", NULL, 0, NULL, 0);
   MRN_STREAM_RECV(stXchangeDims, &tag, p, TAG_XCHANGE_DIMENSIONS);

   PACKET_delete(p);
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

