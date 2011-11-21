#include <BackEnd.h>
#include "ClusteringOffline_BE.h"

#if 0
string ClusteringDefinitionXML;  /* Clustering definition XML file name */
string InputTraceName;           /* Input trace name */
string OutputFileName;           /* Data extracted from input trace */
bool   Verbose = false;
bool   ReconstructTrace = false;

void PrintUsage(char* ApplicationName)
{
   cout << "Usage: " << ApplicationName << " [-s] -d <clustering_def.xml> ";
   cout << "-i <input_trace> -o <output_trace>" << endl;
}

void ReadArgs(int argc, char *argv[])
{
   bool   ClusteringDefinitionRead = false;
   bool   InputTraceNameRead       = false;
   bool   OutputFileNameRead       = false;

   INT32 j = 1;
   INT32 ParametersRequired = 1;

   if (argc == 1 ||
       argc == 2 &&
       ((strcmp(argv[1], "-h") == 0) || (strcmp(argv[1], "--help") == 0)))
   {
      fprintf(stdout, HELP, argv[0]);
      exit(EXIT_SUCCESS);
   }

   if (argc == 1 ||
       argc == 2 &&
       ((strcmp(argv[1], "-v") == 0 || (strcmp(argv[1], "--version") == 0))))
   {
      fprintf(stdout, ABOUT, argv[0], VERSION, __DATE__);
      exit(EXIT_SUCCESS);
   }

   if (argv[1][0] == '-')
   {
      for (j = 1; (j < argc - 1) && (argv[j][0] == '-'); j++)
      {
         switch (argv[j][1])
         {
            case 'd':
               j++;
               ClusteringDefinitionXML  = argv[j];
               ClusteringDefinitionRead = true;
               break;
            case 'i':
               j++;
               InputTraceName     = argv[j];
               InputTraceNameRead = true;
               break;
            case 'o':
               j++;
               OutputFileName     = argv[j];
               OutputFileNameRead = true;
               break;
            case 's':
               Verbose = false;
               break;
            case 'r':
               ReconstructTrace = true;
               break;
            default:
               cerr << "**** INVALID PARAMETER " << argv[j][1] << " **** " << endl << endl;
               PrintUsage(argv[0]);
               exit(EXIT_FAILURE);
               break;
         }
      }
   }

   if (!ClusteringDefinitionRead)
   {
      cerr << "Definition XML file missing ( \'-d\' parameter)" << endl;
      exit (EXIT_FAILURE);
   }

   if (!InputTraceNameRead)
   {
      cerr << "Input trace missing ( \'-i\' parameter)" << endl;
      exit (EXIT_FAILURE);
   }

   if (!OutputFileNameRead)
   {
      cerr << "Output data file file missing ( \'-o\' parameter)" << endl;
      exit (EXIT_FAILURE);
   }

   return;
}
#endif

int main(int argc, char *argv[])
{
//   ReadArgs(argc, argv);

   BackEnd *BE = new BackEnd();
   BE->Init(argc, argv);

//   BackProtocol *protClustering = new Clustering(ClusteringDefinitionXML, InputTraceName, OutputFileName, ReconstructTrace, Verbose);
   BackProtocol *protClustering = new Clustering();
   BE->LoadProtocol( protClustering );

   BE->Loop();

   return 0;
}


