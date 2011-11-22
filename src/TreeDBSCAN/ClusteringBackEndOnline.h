#ifndef __CLUSTERING_BACKEND_ONLINE_H__
#define __CLUSTERING_BACKEND_ONLINE_H__

#include "ClusteringBackEnd.h"

/**
 * This class implements an specific back-end protocol that
 * extracts data from an on-line tracing system.
 */
class ClusteringBackEndOnline: public ClusteringBackEnd
{
   public:
      typedef int (*CallbackType)(vector<const Point*> &ClusteringInput);

      ClusteringBackEndOnline(CallbackType DataExtractCallback);

      bool InitLibrary();
      bool ExtractData();
      bool AnalyzeData();

   private:
      CallbackType DataExtractCallback;
      vector<const Point*> ExternalPoints;
};

#endif /* __CLUSTERING_BACKEND_ONLINE_H__ */
