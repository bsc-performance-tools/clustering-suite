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

  $URL::                                          $:  File
  $Rev::                                          $:  Revision of last commit
  $Author::                                       $:  Author of last commit
  $Date::                                         $:  Date of last commit

\* -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=- */

#include "Partition.hpp"

/**
 * Default constructor
 */
Partition::Partition(void)
{
  _NumberOfClusters        = 0;
  _HasNoise                = false;
  _ClusterAssignmentVector = vector<cluster_id_t> (0);
}

/**
 * Returns the assignment vector
 *
 * \return The assignment vector
 */
vector<cluster_id_t>& Partition::GetAssignmentVector(void)
{
  return _ClusterAssignmentVector;
}

/**
 * Sets the assignment vector
 *
 * \param The new assignment vector
 */ 
void Partition::SetAssignmentVector(vector<cluster_id_t>& ClusterAssignmentVector)
{
  _ClusterAssignmentVector = ClusterAssignmentVector;
}

/**
 * Returns the set containing the IDs used
 *
 * \return A set containing the IDs used in the partition
 */
set<cluster_id_t>& Partition::GetIDs(void)
{
  return _IDs;
}

/**
 * Sets the set of used IDs
 *
 * \param IDs A set containing the IDs used in the partition
 */
void Partition::SetIDs(set<cluster_id_t>& IDs)
{
  _IDs = IDs;
}

/**
 * Sets the number of clusters discovered
 *
 * \param NumberOfClusters Number of clusters discovered in the data
 */
void Partition::NumberOfClusters(size_t NumberOfClusters)
{
  _NumberOfClusters = NumberOfClusters;
}

/**
 * Return the number of clusters discovered
 *
 * \return The number of clusters discovered
 */
size_t Partition::NumberOfClusters(void) const
{
  return _NumberOfClusters;
}

/**
 * Sets if current partition has a noise cluster
 *
 */
void Partition::HasNoise(bool HasNoise)
{
  _HasNoise = HasNoise;
}

/**
 * Return if the current partition has a noise cluster
 *
 * \return True if the current partition has a noise cluster
 */ 
bool Partition::HasNoise(void) const
{
  return _HasNoise;
}



