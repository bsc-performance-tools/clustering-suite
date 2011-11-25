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
      typedef int (*CallbackType)(vector<Point*> &ClusteringInput,
                                  vector<double> &MinDimensions,
                                  vector<double> &MaxDimensions);

      ClusteringBackEndOnline(CallbackType DataExtractCallback);

      bool InitLibrary();
      bool ExtractData();
      bool AnalyzeData();

   private:
      CallbackType DataExtractCallback;
      vector<Point*> ExternalPoints;

      void Normalize(double *MinGlobalDimensions, double *MaxGlobalDimensions);
};

#endif /* __CLUSTERING_BACKEND_ONLINE_H__ */
