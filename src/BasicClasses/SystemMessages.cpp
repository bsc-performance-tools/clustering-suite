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

  $URL:: https://svn.bsc.#$:  File
  $Rev:: 20               $:  Revision of last commit
  $Author:: jgonzale      $:  Author of last commit
  $Date:: 2010-03-09 17:1#$:  Date of last commit

\* -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=- */

#include "SystemMessages.hpp"

bool system_messages::verbose = false;

void system_messages::show_progress(FILE*       channel,
                                    const char* message,
                                    int         current,
                                    int         total)
{
  if (system_messages::verbose)
  {
    fprintf(channel, "\r%s %d/%d", message, current, total);
    fflush(channel);
  }
}

void system_messages::show_progress_end(FILE*       channel,
                                        const char* message,
                                        int         total)
{
  if (system_messages::verbose)
  {
    fprintf(channel, "\r%s %d/%d\n", message, total, total);
    fflush(channel);
  }
}

void system_messages::show_percentage_progress(FILE*       channel,
                                               const char* message,
                                               int         current_percentage)
{
  int real_percentage;

  if (current_percentage < 0)
    real_percentage = 0;

  if (current_percentage > 100)
    real_percentage = 100;
  
  if (system_messages::verbose)
  {
    fprintf(channel, "\r%s %d/100\n", message, real_percentage);
    fflush(channel);
  }
}
