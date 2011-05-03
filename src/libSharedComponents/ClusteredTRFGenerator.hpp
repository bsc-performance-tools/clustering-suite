/*******************************************************************************
 *  $RCSfile: ClusteredTRFGenerator.h,v $
 *
 *  Last modifier: $Author: jgonzale $
 *  Last check in: $Date: 2008/08/08 14:20:31 $
 *  Revision:      $Revision: 1.1.1.1 $
 *
 *  Copyright (c) 2006  CEPBA Tools
 *  jgonzale@cepba.upc.edu
 *
 *  Description: definition of the class that apply the results of the
 *               clustering algorithm to the input TRF trace
 *
 ******************************************************************************/

#include "ClusteredTraceGenerator.hpp"


#include <climits>
#include <cstdio>

#include <string>

#include <vector>
using std::vector;

class ClusteredTRFGenerator: public ClusteredTraceGenerator
{
  public:
    static const INT32 NO_MORE_CLUSTERS             = INT_MIN;
    static const INT32 READ_ERROR                   = INT_MIN + 1;

  private:
    bool          PrintClusterBlocks;

  public:
    ClusteredTRFGenerator(string  InputTraceName,
                          string  OutputTraceName,
                          bool    PrintClusterBlocks = false);

    ~ClusteredTRFGenerator(void);
  
    void SetPrintClusterBlocks (bool PrintClusterBlocks)
    {
      this->PrintClusterBlocks = PrintClusterBlocks;
    }

    bool SetEventsToDealWith (set<event_type_t>& EventsToDealWith);
    
    bool Run(vector<CPUBurst*>&    ClusteringBursts,
             vector<cluster_id_t>& IDs,
             size_t                NumberOfClusters,
             bool                  MinimizeInformation = false);

  private:
    INT32 GetNextClusterId(void);

    bool CheckFirstBlockDefinition(char*  Buffer,
                                   size_t NumberOfClusters);
                                   

    bool PrintClusteredBurst(FILE        *Trace,
                             long         TaskId,
                             long         ThreadId,
                             double       BurstDuration,
                             cluster_id_t ClusterId,
                             bool         ReplaceDurations = false);
};

typedef ClusteredTRFGenerator* ClusteredTRFGenerator_t;
