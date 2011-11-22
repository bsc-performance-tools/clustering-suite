#include "ClusteringBackEndOnline.h"


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
   int InputSize = 0;

   InputSize = DataExtractCallback(ExternalPoints);
   cout << "[BE " << WhoAmI() << "] Input size=" << InputSize << endl;

   return true;
}


/**
 * Analyzes the data from a tracing faciliy.
 * @return true on success; false otherwise.
 */ 
bool ClusteringBackEndOnline::AnalyzeData(void)
{
   cout << "[DEBUG] [BE " << WhoAmI() << "] ExternalPoints.size()=" << ExternalPoints.size() << endl;
   if (!libClustering->ClusterAnalysis(ExternalPoints, LocalModel))
   {
      cerr << "[BE " << WhoAmI() << "] Error clustering data: " << libClustering->GetErrorMessage() << endl;
      return false;
   }
   return true;
}


