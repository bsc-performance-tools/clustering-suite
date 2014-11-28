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

  $Id::                                       $:  Id
  $Rev::                                          $:  Revision of last commit
  $Author::                                       $:  Author of last commit
  $Date::                                         $:  Date of last commit

\* -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=- */

#ifndef __TDBSCAN_WORKER_ONLINE_H__
#define __TDBSCAN_WORKER_ONLINE_H__

#include "TDBSCANWorker.h"

/**
 * This class implements an specific back-end protocol that
 * extracts data from an on-line tracing system.
 */
class TDBSCANWorkerOnline: public TDBSCANWorker
{
   public:
      typedef void (*FetchCallback)(libDistributedClustering *libClustering);
      typedef void (*FeedCallback) (vector<timestamp_t> &BeginTimes, vector<timestamp_t> &EndTimes, vector<cluster_id_t> &ClusterIDs, vector<int> &BurstsSupport);

      TDBSCANWorkerOnline(FetchCallback DataExtractCallback, FeedCallback DataFeedCallback);

      bool InitLibrary();
      bool ExtractData();
      bool ProcessResults(Support &GlobalSupport);

   private:
      FetchCallback DataExtractCallback;
      FeedCallback  DataFeedCallback;
};

#endif /* __TDBSCAN_WORKER_ONLINE_H__ */
