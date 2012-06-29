/*****************************************************************************\
 *                        ANALYSIS PERFORMANCE TOOLS                         *
 *                             ClusteringSuite                               *
 *   Infrastructure and tools to apply clustering analysis to Paraver and    *
 *                              Dimemas traces                               *
 *                                                                           *
 *****************************************************************************
 *     ___     This library is free software; you can redistribute it and/or *
 *    /  __         modify it under the terms of the GNU LGPL as published   *
 *   /  /  _____    by the Free Software Foundation; either version 2.1      *
 *  /  /  /     \   of the License, or (at your option) any later version.   *
 * (  (  ( B S C )                                                           *
 *  \  \  \_____/   This library is distributed in hope that it will be      *
 *   \  \__         useful but WITHOUT ANY WARRANTY; without even the        *
 *    \___          implied warranty of MERCHANTABILITY or FITNESS FOR A     *
 *                  PARTICULAR PURPOSE. See the GNU LGPL for more details.   *
 *                                                                           *
 * You should have received a copy of the GNU Lesser General Public License  *
 * along with this library; if not, write to the Free Software Foundation,   *
 * Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA          *
 * The GNU LEsser General Public License is contained in the file COPYING.   *
 *                                 ---------                                 *
 *   Barcelona Supercomputing Center - Centro Nacional de Supercomputacion   *
\*****************************************************************************/

/* -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=- *\

  $Id::                                           $:  Id
  $Rev::                                          $:  Revision of last commit
  $Author::                                       $:  Author of last commit
  $Date::                                         $:  Date of last commit

\* -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=- */

#ifndef __NOISE_MANAGER_H__
#define __NOISE_MANAGER_H__

#include "libDistributedClustering.hpp"
#include "MRNet_wrappers.h"

#define PROCESS_NOISE

using MRN::Stream;
using MRN::Packet;
using MRN::PacketPtr;

/**
 * This class manages the serialization and clustering of the noise data through the MRNet.
 */
class NoiseManager
{
   public:
#if defined(FRONTEND) || defined(FILTER)
      /* Front-end/Filter interface */
      NoiseManager();
      NoiseManager(double Epsilon, int MinPoints);

      bool ClusterNoise(vector<const Point*>& Points, vector<HullModel*>& NoiseModel, int &CountRemainingNoise);

      void Serialize(int StreamID, std::vector< PacketPtr >& OutputPackets);

      int  Unpack(PACKET_PTR in_packet, vector<const Point *> &NoisePoints);

#endif /* FRONTEND or FILTER */

#if defined(BACKEND)
      /* Back-end interface */
      NoiseManager(libDistributedClustering *libClustering);

      void Serialize(Stream *OutputStream);
#endif /* BACKEND */

   private:
      libDistributedClustering *libClustering;

      void Serialize(int &DimensionsCount, double *&SerialPoints, unsigned int &SerialPointsCount);
};

#endif /* __NOISE_MANAGER_H__ */
