#ifndef __CLUSTERING_BACKEND_OFFLINE_H__
#define __CLUSTERING_BACKEND_OFFLINE_H__

#include "ClusteringBackEnd.h"

/**
 * This class implements an specific back-end protocol that
 * extracts data from a Paraver trace.
 */
class ClusteringBackEndOffline: public ClusteringBackEnd
{
   public:
      bool InitLibrary();
      bool ExtractData();
      bool AnalyzeData();
};

#endif /* __CLUSTERING_BACKEND_OFFLINE_H__ */


