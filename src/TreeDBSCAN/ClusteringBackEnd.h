#ifndef __CLUSTERING_BACKEND_H__
#define __CLUSTERING_BACKEND_H__

#include <vector>
#include <libDistributedClustering.hpp>
#include <BackProtocol.h>
#include "ClusteringCore.h"

/**
 * This class implements the back-end side of the TreeDBSCAN protocol.
 */
class ClusteringBackEnd : public ClusteringCore, public BackProtocol
{
   public:
      ClusteringBackEnd();

      string ID() { return "CLUSTERING"; }
      void   Setup(void);
      int    Run  (void);

      virtual bool InitLibrary(void) = 0;
      virtual bool ExtractData(void) = 0;
      virtual bool AnalyzeData(void) = 0;

   protected:
      libDistributedClustering *libClustering;
      vector<HullModel*> LocalModel;

   private:
      /* Names of the output scripts and plots */
      string GlobalModelDataFileName;
      string GlobalModelPlotFileName;
      string LocalModelDataFileName;
      string LocalModelPlotFileName;
      string OutputDataFileName;
      string OutputLocalClusteringFileName;
      string ClustersInformationFileName;

      void CheckOutputFile();
};

#endif /* __CLUSTERING_BACKEND_H__ */
