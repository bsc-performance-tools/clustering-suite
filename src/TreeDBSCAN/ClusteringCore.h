#ifndef __CLUSTERING_CORE_H__
#define __CLUSTERING_CORE_H__

#include <MRNet_wrappers.h>
#include "libDistributedClustering.hpp"

/**
 * This is the base class both for the front-end and the back-ends.
 * It stores common configuration attributes and data structures
 * that have to be present in both sides of the MRNet.
 */
class ClusteringCore
{
   public:
      ClusteringCore();

   protected:
      STREAM                 *stClustering;
      STREAM                 *stXchangeDims;
      std::vector<HullModel*> GlobalModel;

      double Epsilon;
      int    MinPoints;
      string ClusteringDefinitionXML;
      string InputTraceName;
      string OutputFileName;
      bool   Verbose;
      bool   ReconstructTrace;

      void Send_Configuration(void);
      void Recv_Configuration(void);
};

#endif /* __CLUSTERING_CORE_H__ */

