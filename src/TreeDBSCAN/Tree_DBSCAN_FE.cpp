#include <FrontEnd.h>
#include <stdlib.h>
#include <unistd.h>
#include "Tree_DBSCAN_FE.h"
#include "ClusteringOffline_FE.h"

/* Configuration variables */
double Epsilon   = 0.015;
int    MinPoints = 10;
string ClusteringDefinitionXML;  /* Clustering definition XML file name -- Epsilon and MinPoints are ignored! */
string InputTraceName;           /* Input trace name */
string OutputFileName;           /* Data extracted from input trace */
bool   Verbose          = true;
bool   ReconstructTrace = false;
string TREE_DBSCAN_HOME;

int main(int argc, char *argv[])
{
   /* Parse input argumens */
   ReadArgs(argc, argv);

   /* Create an MRNet front-end */
   FrontEnd *FE = new FrontEnd();
   const char **BE_argv = (const char **)(&(argv[1]));
   if (FE->Init(string(TREE_DBSCAN_HOME+"/bin/Tree_DBSCAN_BE").c_str(), BE_argv) == -1)
   {
      cerr << "MRNet front-end could not be initialized due to previous errors." << endl;
      exit(EXIT_FAILURE);
   }

   /* Load the clustering protocol */
   FrontProtocol *protClustering = new Clustering(Epsilon, MinPoints, ClusteringDefinitionXML, InputTraceName, OutputFileName, Verbose, ReconstructTrace);
   FE->LoadProtocol( protClustering );

   /* Tell the back-ends to run the clustering protocol */
   FE->Dispatch("CLUSTERING");

   /* Shutdown the network */
   FE->Shutdown();

   return 0;
}

void ReadArgs(int argc, char *argv[])
{
   char *env_TREE_DBSCAN_HOME     = NULL;
   bool  ClusteringDefinitionRead = false;
   bool  InputTraceNameRead       = false;
   bool  OutputFileNameRead       = false;

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
      for (j = 1; (j < argc) && (argv[j][0] == '-'); j++)
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
            case 'e':
               j++;
               Epsilon = atof(argv[j]);
               if (Epsilon <= 0)
               {
                  cerr << "**** INVALID PARAMETER '-e " << argv[j] << "' **** " << endl << endl;
                  exit(EXIT_FAILURE);
               }
               break;
            case 'm':
               j++;
               MinPoints = atoi(argv[j]);
               if (MinPoints <= 0)
               {
                  cerr << "**** INVALID PARAMETER '-m " << argv[j] << "' **** " << endl << endl;
                  exit(EXIT_FAILURE);
               }
               break;
            default:
               cerr << "**** INVALID PARAMETER '" << argv[j][1] << "' **** " << endl << endl;
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

   env_TREE_DBSCAN_HOME = getenv("TREE_DBSCAN_HOME");
   if (env_TREE_DBSCAN_HOME == NULL)
   {
      cerr << "ERROR: Missing environment variable 'TREE_DBSCAN_HOME'" << endl;
      cerr << "Run 'source etc/sourceme.sh' in the Tree-DBSCAN installation directory and try again." << endl;
      exit (EXIT_FAILURE);
   }
   TREE_DBSCAN_HOME = string(env_TREE_DBSCAN_HOME);

   /* Convert relative to absolute paths */
   string CWD = get_current_dir_name();
   if (ClusteringDefinitionXML[0] != '/')
   {
      ClusteringDefinitionXML = CWD + "/" + ClusteringDefinitionXML;
   }
   if (InputTraceName[0] != '/')
   {
      InputTraceName = CWD + "/" + InputTraceName;
   }
   if (OutputFileName[0] != '/')
   {
      OutputFileName = CWD + "/" + OutputFileName;
   }

   /* Check the files exist and are readable */
   std::ifstream fd_XML(ClusteringDefinitionXML.c_str());
   if (!fd_XML.good())
   {
      cerr << "Definition XML file '" << ClusteringDefinitionXML << "' does not exist!" << endl;
      exit(EXIT_FAILURE);
   }
   std::ifstream fd_Input(InputTraceName.c_str());
   if (!fd_Input.good())
   {
      cerr << "Input trace '" << InputTraceName << "' does not exist!" << endl;
      exit(EXIT_FAILURE);
   }

   return;
}


void PrintUsage(char* ApplicationName)
{
   cout << "Usage: " << ApplicationName;
   cout << " [-sr] [-e <epsilon>] [-m <min_points>]";
   cout << " -d <clustering_def.xml> -i <input_trace> -o <output_trace>";
   cout << endl;
}

