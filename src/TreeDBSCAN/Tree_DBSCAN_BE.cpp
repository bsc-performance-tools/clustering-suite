#include <BackEnd.h>
#include "ClusteringBackEndOffline.h"

/**
 * The back-end application loads the TreeDBSCAN protocol and waits for 
 * the front-end to start the analysis.
 * @param argc Number of arguments.
 * @param argv Array of arguments.
 * @return 0 on success; -1 otherwise.
 */ 
int main(int argc, char *argv[])
{
   BackEnd *BE = new BackEnd();
   BE->Init(argc, argv);

   BackProtocol *protClustering = new ClusteringBackEndOffline();
   BE->LoadProtocol( protClustering );

   BE->Loop();

   return 0;
}

