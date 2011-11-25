#include <iostream>
#include <vector>
#include <mrnet/MRNet.h>
#include "Utils.h"

using namespace MRN;
using namespace std;

extern "C" {

const char *filterXchangeDimensions_format_string = "%alf %alf";

/**
 * This filter receives the mininum and maximum dimensions from every children
 * and propagates the absolute mins and maxs.
 */
void filterXchangeDimensions( std::vector< PacketPtr >& packets_in,
                              std::vector< PacketPtr >& packets_out,
                              std::vector< PacketPtr >& /* packets_out_reverse */,
                              void ** /* client data */,
                              PacketPtr& params,
                              const TopologyLocalInfo& top_info)
{
  int NumberOfDimensions = 0;
  double *AbsMinDimensions=NULL, *AbsMaxDimensions=NULL;

  for (unsigned int i=0; i<packets_in.size( ); i++)
  {
    double *MinDimensions=NULL, *MaxDimensions=NULL;

    PacketPtr cur_packet = packets_in[i];
    cur_packet->unpack(filterXchangeDimensions_format_string, 
      &MinDimensions, &NumberOfDimensions,
      &MaxDimensions, &NumberOfDimensions);

    if (i==0)
    {
      AbsMinDimensions = (double *)malloc(sizeof(double) * NumberOfDimensions);
      AbsMaxDimensions = (double *)malloc(sizeof(double) * NumberOfDimensions);
      for (unsigned int j=0; j<NumberOfDimensions; j++)
      {
        AbsMinDimensions[j] = MinDimensions[j];
        AbsMaxDimensions[j] = MaxDimensions[j];
      }
    }
    else
    {
      for (unsigned int j=0; j<NumberOfDimensions; j++)
      {
        if (MinDimensions[j] < AbsMinDimensions[j]) AbsMinDimensions[j] = MinDimensions[j];
        if (MaxDimensions[j] < AbsMaxDimensions[j]) AbsMaxDimensions[j] = MaxDimensions[j];
      }
    }
    xfree(MinDimensions);
    xfree(MaxDimensions);
  }

  PacketPtr new_packet ( new Packet(packets_in[0]->get_StreamId(),
                                    packets_in[0]->get_Tag(),
                                    filterXchangeDimensions_format_string,
                                    AbsMinDimensions, NumberOfDimensions,
                                    AbsMaxDimensions, NumberOfDimensions) );
  packets_out.push_back( new_packet );
}
  
} /* extern "C" */

