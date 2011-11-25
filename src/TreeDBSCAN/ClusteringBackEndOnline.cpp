#include "ClusteringBackEndOnline.h"
#include "ClusteringTags.h"
#include "Utils.h"

#include <iostream>
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


