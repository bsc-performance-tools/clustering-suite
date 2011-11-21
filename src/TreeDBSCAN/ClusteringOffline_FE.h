#include <FrontProtocol.h>
#include <libDistributedClustering.hpp>
#include <vector>

class Clustering : public FrontProtocol
{
   public:
      Clustering(
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

      void set_Epsilon(double Eps);
      void set_MinPoints(int MinPts);

   private:
      STREAM *stClustering;


      double Epsilon;
      int    MinPoints;
      string ClusteringDefinitionXML;
      string InputTraceName;
      string OutputFileName;
      bool   Verbose;
      bool   ReconstructTrace;



      std::vector<ConvexHullModel> GlobalModel;
};

