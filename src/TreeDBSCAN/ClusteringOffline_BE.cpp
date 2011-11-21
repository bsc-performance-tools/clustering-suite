#include <Timer.hpp>
#include "FileNameManipulator.hpp"
#include "ClusteringOffline_BE.h"
#include "NoiseManager.h"
#include "tags.h"
#include "utils.h"

using namespace cepba_tools;

Clustering::Clustering()
{
   libClustering = NULL;
}

void Clustering::Setup()
{
   Register_Stream(stClustering);
}

void Clustering::Read_Configuration(void)
{
   char *XML, *Input, *Output;
   int tag, Verb, Reconstruct;
   PACKET_new(p);

   /* Receive clustering configuration from the front-end */
   STREAM_recv(stClustering, &tag, p, TAG_CLUSTERING_CONFIG);
   PACKET_unpack(p, "%lf %d %s %s %s %d %d", &Epsilon, &MinPoints, &XML, &Input, &Output, &Verb, &Reconstruct);
   PACKET_delete(p);

   ClusteringDefinitionXML = string(XML);
   InputTraceName          = string(Input);
   OutputFileName          = string(Output);
   Verbose                 = (Verb == 1);
   ReconstructTrace        = (Reconstruct == 1);
   xfree(XML); xfree(Input); xfree(Output);

   /* Prepare all outputs file names */
   CheckOutputFile();
}

int Clustering::Run()
{
   int tag;
   PACKET_new(p);
   vector<ConvexHullModel> LocalModel;
   vector<ConvexHullModel>::iterator it;
   Timer t;

   /* Delete any previous clustering */
   if (libClustering != NULL) delete libClustering;
   libClustering = new libDistributedClustering(Verbose);

   /* Receive clustering configuration from the front-end */
   Read_Configuration();

   /* Initialize the clustering library */
   if (!libClustering->InitClustering(ClusteringDefinitionXML, Epsilon, MinPoints, (WhoAmI() == 0), WhoAmI(), NumBackEnds())) // true == Root Task
   {
      cerr << "[BE " << WhoAmI() << "] Error setting up clustering library: " << libClustering->GetErrorMessage() << endl;
      exit (EXIT_FAILURE);
   }

   /* Read points from a Paraver trace */
   if (!libClustering->ExtractData(InputTraceName))
   {
      cerr << "[BE " << WhoAmI() << "] Error extracting data: " << libClustering->GetErrorMessage() << endl;
      exit (EXIT_FAILURE);
   }
   cout << "[BE " << WhoAmI() << "] Bursts to analyze: " << libClustering->GetNumberOfPoints() << endl;

   /* Start the clustering analysis */
   t.begin();

   if (!libClustering->ClusterAnalysis(LocalModel))
   {
      cerr << "[BE " << WhoAmI() << "] Error clustering data: " << libClustering->GetErrorMessage() << endl;
      exit (EXIT_FAILURE);
   }

#if defined(PROCESS_NOISE)
   /* DEBUG -- count remaining noise points
   vector<const Point *> NoisePoints;
   libClustering->GetNoisePoints(NoisePoints);
   if (Verbose) cerr << "[BE " << WhoAmI() << "] Number of noise points = " << NoisePoints.size() << endl; */

   NoiseManager Noise = NoiseManager(libClustering);
   Noise.Serialize(stClustering);
#endif

   /* Send the local hulls */
   if (Verbose) cout << "[BE " << WhoAmI() << "] Sending " << LocalModel.size() << " local hulls" << endl;

   for (it=LocalModel.begin(); it!=LocalModel.end(); ++it)
   {
      ConvexHullModel Hull = *it;

      long long  Density;
      int        NumPoints, NumDimensions;
      long long *Instances, *NeighbourhoodSizes;
      double    *DimValues;

      Hull.Serialize (Density, NumPoints, NumDimensions, Instances, NeighbourhoodSizes, DimValues);

      STREAM_send(stClustering, TAG_HULLS, "%ld %d %d %ald %ald %alf", Density, NumPoints, NumDimensions, Instances, NumPoints, NeighbourhoodSizes, NumPoints, DimValues, NumPoints*NumDimensions);

      xfree(DimValues);
   }

   /* Notify all hulls were sent */
   STREAM_send(stClustering, TAG_ALL_HULLS_SENT, "");
   if (Verbose) cout << "[BE " << WhoAmI() << "] All " << LocalModel.size() << " hulls sent." << endl;

   /* Receive the resulting global hulls */
   do
   {
      STREAM_recv(stClustering, &tag, p, TAG_HULLS);
      if (tag == TAG_HULLS)
      {
         long long Density;
         int NumPoints, NumDimensions, DimValuesSize;
         long long *Instances, *NeighbourhoodSizes;
         double * DimValues;

         p->unpack("%ld %d %d %ald %ald %alf", &Density, &NumPoints, &NumDimensions, &Instances, &NumPoints, &NeighbourhoodSizes, &NumPoints, &DimValues, &DimValuesSize);
         ConvexHullModel Hull(Density, NumPoints, NumDimensions, Instances, NeighbourhoodSizes, DimValues);
         GlobalModel.push_back( Hull );

         xfree(Instances);
         xfree(NeighbourhoodSizes);
         xfree(DimValues);


         /* DEBUG -- dump current hull
         if (WhoAmI() == 0)
         {
            std::cout << "********** [BE " << WhoAmI() << "] PRINT HULL " << GlobalModel.size() << "\n";
            hull.Print();
            std::cout << "********** [BE " << WhoAmI() << "] END PRINT HULL " << GlobalModel.size() << "\n";
         } */
      }
   } while (tag != TAG_ALL_HULLS_SENT);

   if (Verbose) cout << "[BE " << WhoAmI() << "] Received " << GlobalModel.size() << " global hulls." << endl;
   cout << "[BE " << WhoAmI() << "] >> Clustering time: " << t.end() << "[" << NumBackEnds() << " BEs]" << endl;


   /* Print the local model */
   ostringstream ModelTitle;

   cout << "[BE " << WhoAmI() << "] Printing local model" << endl;

   ModelTitle << "Local Hull Models BE " << WhoAmI() << " MinPoints = " << MinPoints << " Eps = " << Epsilon << "\\n";
   ModelTitle << "Trace \'" << InputTraceName << "\'";

   if (!libClustering->PrintModels(LocalModel,
                                   LocalModelDataFileName,
                                   LocalModelPlotFileName,
                                   ModelTitle.str()))
   {
      cerr << "[BE " << WhoAmI() << "] Error printing local model scripts: " << libClustering->GetErrorMessage() << endl;
      exit (EXIT_FAILURE);
   }


   /* All back-ends classify their local data */
   cout << "[BE " << WhoAmI() << "] START CLASSIFYING WITH " << GlobalModel.size() << " GLOBAL HULLS." << endl;
   t.begin();
   if (!libClustering->ClassifyData(GlobalModel))
   {
      cerr << "[BE " << WhoAmI() << "] Error classifying data: " << libClustering->GetErrorMessage() << endl;
      exit (EXIT_FAILURE);
   }
   cout << "[BE " << WhoAmI() << "] >> Classification time: " << t.end() << endl;


   if (WhoAmI() == 0)
   {
      /* Print the global model */
      cout << "[BE " << WhoAmI() << "] Printing global model script" << endl;

      ModelTitle.str("");
      ModelTitle << "Global Model MinPoints = " << MinPoints << " Eps = " << Epsilon << "\\n";
      ModelTitle << "Trace \'" << InputTraceName << "\'";

      if (!libClustering->PrintModels(GlobalModel,
                                      GlobalModelDataFileName,
                                      GlobalModelPlotFileName,
                                      ModelTitle.str()))
      {
        cerr << "[BE " << WhoAmI() << "] Error printing global model script: " << libClustering->GetErrorMessage() << endl;
        exit (EXIT_FAILURE);
      }

      /* Print the data plots */
      cout << "[BE " << WhoAmI() << "] Printing global data plot script" << endl;
      if (!libClustering->PrintPlotScripts(OutputDataFileName, "", false)) // false = Global Classification
      {
        cerr << "[BE " << WhoAmI() << "] Error printing global data plot scripts: " << libClustering->GetErrorMessage() << endl;
        exit (EXIT_FAILURE);
      }

      /* Reconstruct the trace */
      if (ReconstructTrace)
      {
        cout << "[BE " << WhoAmI() << "] Reconstructing trace..." << endl;
        if (!libClustering->ReconstructInputTrace(OutputFileName))
        {
          cerr << "Error writing output trace: " << libClustering->GetErrorMessage() << endl;
          exit (EXIT_FAILURE);
        }
      }

      if (!libClustering->FlushClustersInformation(ClustersInformationFileName))
      {
         cerr << "Error writing clusters information file: " << libClustering->GetErrorMessage() << endl;
	 exit (EXIT_FAILURE);
      }
   }

   /* Print local clustering plots */
   cout << "[BE " << WhoAmI() << "] Printing local data plot script" << endl;
   if (!libClustering->PrintPlotScripts(OutputLocalClusteringFileName, "", true)) // true = Local partition
   {
      cerr << "[BE " << WhoAmI() << "] Error printing local data plot scripts: " << libClustering->GetErrorMessage() << endl;
      exit (EXIT_FAILURE);
   }

   PACKET_delete(p);
   return 0;
}

void Clustering::CheckOutputFile()
{
   string OutputFileExtension;
   OutputFileExtension = FileNameManipulator::GetExtension(OutputFileName);

   FileNameManipulator NameManipulator(OutputFileName, OutputFileExtension);

   ostringstream ModelExtension;
   ModelExtension.str("");
   ModelExtension << "LOCAL_MODEL_" << WhoAmI();

   LocalModelDataFileName = NameManipulator.AppendStringAndExtension (ModelExtension.str(), "csv");
   LocalModelPlotFileName = NameManipulator.AppendStringAndExtension (ModelExtension.str(), "gnuplot");

   ModelExtension.str("");
   ModelExtension << "LOCAL_CLUSTERING_" << WhoAmI();

   OutputLocalClusteringFileName = NameManipulator.AppendStringAndExtension (ModelExtension.str(), "csv");

   GlobalModelDataFileName     = NameManipulator.AppendStringAndExtension ("JOINT_MODEL", "csv");
   GlobalModelPlotFileName     = NameManipulator.AppendStringAndExtension ("JOINT_MODEL", "gnuplot");
   OutputDataFileName          = NameManipulator.AppendStringAndExtension ("CLUSTERED", "csv");
   ClustersInformationFileName = NameManipulator.AppendStringAndExtension ("clusters_info", "csv");
}

