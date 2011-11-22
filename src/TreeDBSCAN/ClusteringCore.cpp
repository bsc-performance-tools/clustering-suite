#include "ClusteringCore.h"
#include "ClusteringTags.h"
#include "Utils.h"


/** 
 * Constructor sets default configuration values.
 */
ClusteringCore::ClusteringCore()
{
   Epsilon                 = 0.015;
   MinPoints               = 10;
   ClusteringDefinitionXML = "";
   InputTraceName          = "";
   OutputFileName          = "OUTPUT.prv";
   Verbose                 = false;
   ReconstructTrace        = false;
   stClustering            = NULL;
   GlobalModel.clear();
}


/**
 * Front-end call to send the clustering parameters to the back-ends.
 */
void ClusteringCore::Send_Configuration(void)
{
   stClustering->send(TAG_CLUSTERING_CONFIG, "%lf %d %s %s %s %d %d",
      Epsilon,
      MinPoints,
      ClusteringDefinitionXML.c_str(),
      InputTraceName.c_str(),
      OutputFileName.c_str(),
      ((int)Verbose),
      ((int)ReconstructTrace));
}


/**
 * Back-end call to receive the clustering parameters from the front-end.
 */
void ClusteringCore::Recv_Configuration(void)
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
}

