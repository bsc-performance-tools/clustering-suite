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

  $Id:: libDistributedClustering.cpp 51 2011-11-2#$:  Id
  $Rev:: 51                                       $:  Revision of last commit
  $Author:: jgonzale                              $:  Author of last commit
  $Date:: 2011-11-24 15:47:29 +0100 (Thu, 24 Nov #$:  Date of last commit

\* -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=- */

#ifndef __HULL_MANAGER_H__
#define __HULL_MANAGER_H__

#include <vector>
#include "libDistributedClustering.hpp"
#include "MRNet_wrappers.h"

using namespace std;

/**
 * This class manages the serialization of convex hulls through the MRNet.
 */
class HullManager
{
  public:
    HullManager();

    /* Common API */
    HullModel* Unpack(PACKET_PTR InputPacket);

    /* Back-end API */
    void Serialize(STREAM *OutputStream, vector<HullModel*> &HullsList);

    /* Filter API */
    void Serialize(int StreamID, vector<PacketPtr> &OutputPackets, vector<HullModel*> &HullsList);

  private:
    /* Back-end API */
    void SerializeOne(STREAM *OutputStream, HullModel *Hull);
    void SerializeDone(STREAM *OutputStream);

    /* Filter API */
    void SerializeOne(int StreamID, vector<PacketPtr> &OutputPackets, HullModel *Hull);
    void SerializeDone(int StreamID, vector<PacketPtr> &OutputPackets);
};

#endif /* __HULL_MANAGER_H__ */

