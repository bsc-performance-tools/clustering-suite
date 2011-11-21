#include <BackProtocol.h>
#include <libDistributedClustering.hpp>
#include <vector>

class Clustering : public BackProtocol
{
   public:
      Clustering();

      string ID() { return "CLUSTERING"; }
      void   Setup(void);
      int    Run  (void);

   private:
      int  Verbose;
      bool ReconstructTrace;
      double Epsilon;
      int MinPoints;

      string ClusteringDefinitionXML;    /* Clustering definition XML file name */
      string InputTraceName;             /* Input trace name */
      string OutputFileName;             /* Data extracted from input trace */
      string GlobalModelDataFileName;
      string GlobalModelPlotFileName;
      string LocalModelDataFileName;
      string LocalModelPlotFileName;
      string OutputDataFileName;
      string OutputLocalClusteringFileName;
      string ClustersInformationFileName;

      STREAM *stClustering;
          libDistributedClustering *libClustering;
          std::vector<ConvexHullModel> GlobalModel;

      void CheckOutputFile();
      void Read_Configuration(void);

};
