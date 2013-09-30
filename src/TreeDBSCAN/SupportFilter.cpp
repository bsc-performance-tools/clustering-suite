#include <iostream>
#include <mrnet/MRNet.h>
#include "Support.h"
#include "TDBSCANFilter.h"
#include "TDBSCANTags.h"

using namespace MRN;
using namespace std;

extern "C" {

const char *filterSupport_format_string = "%d %ad %ad";

void filterSupport( std::vector< PacketPtr >& packets_in,
                    std::vector< PacketPtr >& packets_out,
                    std::vector< PacketPtr >& /* packets_out_reverse */,
                    void ** /* client data */,
                    PacketPtr& params,
                    const TopologyLocalInfo& top_info)
{
   int tag = packets_in[0]->get_Tag();

   /* Bypass the filter in the back-ends, there's nothing to merge at this level! */
   if (BOTTOM_FILTER(top_info))
   {
      for (unsigned int i=0; i<packets_in.size(); i++)
      {
         packets_out.push_back(packets_in[i]);
      }
      return;
   }

   switch(tag)
   {
      case TAG_SUPPORT:
      {
         Support CumulativeSupport;

         for (int i=0; i<packets_in.size(); i++)
         {
           CumulativeSupport.Unpack( packets_in[i] );
         }
         CumulativeSupport.Serialize(packets_in[0]->get_StreamId(), packets_out);
         break;
      }
   }
}

}
