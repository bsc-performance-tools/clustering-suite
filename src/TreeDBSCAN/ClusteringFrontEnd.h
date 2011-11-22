#ifndef __CLUSTERING_FRONTEND_H__
#define __CLUSTERING_FRONTEND_H__

#include <vector>
#include <FrontProtocol.h>
#include "ClusteringCore.h"

/**
 * This class implements the front-end side of the TreeDBSCAN protocol, 
 * which is the same both for the on-line and off-line back-ends.
 */
class ClusteringFrontEnd : public ClusteringCore, public FrontProtocol
{
   public:
      ClusteringFrontEnd(
         double Eps, 
         int    MinPts,
         string ClusteringDefinitionXML,
         string InputTraceName,
         string OutputFileName,
         bool   Verbose,
         bool   ReconstructTrace);

      string ID() { return "CLUSTERING"; }
      void   Setup(void);
      int    Run  (void);
};

#endif /* __CLUSTERING_FRONTEND_H__ */
