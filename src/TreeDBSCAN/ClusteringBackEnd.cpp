#include <Timer.hpp>
#include "FileNameManipulator.hpp"
#include "ClusteringBackEnd.h"
#include "NoiseManager.h"
#include "HullManager.h"
#include "ClusteringTags.h"
#include "Utils.h"

using namespace cepba_tools;

#include <iostream>
using std::cout;
using std::cerr;
using std::endl;

#include <sstream>
using std::ostringstream;


/**
 * TreeDBSCAN back-end protocol constructor.
 */
ClusteringBackEnd::ClusteringBackEnd()
{
   libClustering = NULL;
}


/**
 * Register the streams used in this protocol.
 */
void ClusteringBackEnd::Setup()
{
   Register_Stream(stClustering);
}


/**
 * Implements the back-end side of the TreeDBSCAN clustering algorithm.
 * @return 0 on success; -1 otherwise.
 */
int ClusteringBackEnd::Run()
{
   int tag;
   PACKET_new(p);
   vector<HullModel*>::iterator it;
   Timer t;

   /* Delete any previous clustering */
   if (libClustering != NULL) delete libClustering;
   libClustering = new libDistributedClustering(Verbose);

   /* Receive clustering configuration from the front-end */
   Recv_Configuration();
   /* Prepare all outputs file names */
   CheckOutputFile();

   if (!InitLibrary())
   {
      cerr << "[BE " << WhoAmI() << "] Error initializing clustering. Exiting..." << endl;
      exit (EXIT_FAILURE);
   }

   if (!ExtractData())
   {
      cerr << "[BE " << WhoAmI() << "] Error extracting clustering data. Exiting..." << endl;
      exit (EXIT_FAILURE);
   }
   cout << "[BE " << WhoAmI() << "] Bursts to analyze: " << libClustering->GetNumberOfPoints() << endl;

   /* Start the clustering analysis */
   t.begin();

   if (!AnalyzeData())
   {
      cerr << "[BE " << WhoAmI() << "] Error analyzing data. Exiting..." << endl;
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

   HullManager HM = HullManager();
   HM.Serialize(stClustering, LocalModel);

   /* Receive the resulting global hulls */
   do
   {
      STREAM_recv(stClustering, &tag, p, TAG_HULL);
      if (tag == TAG_HULL)
      {
         HullModel *Hull = NULL;
         HullManager HM  = HullManager();
         Hull            = HM.Unpack(p);

         GlobalModel.push_back(Hull);
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


/**
 * Prepares the outputs files names.
 */
void ClusteringBackEnd::CheckOutputFile()
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

