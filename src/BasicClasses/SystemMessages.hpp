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

#ifndef _SYSTEMMESSAGES_HPP_
#define _SYSTEMMESSAGES_HPP_

#include <config.h>

#include <cstdio>
#include <string>
using std::string;

namespace cepba_tools
{
  class system_messages
  {
    public:

      static bool verbose;

      static bool distributed;

      static int  my_rank;

      static bool messages_from_all_ranks;

      static bool percentage_ongoing;
      
      static int  last_percentage_written;

      static void information(string message, FILE* channel = stdout);
      
      static void information(const char* message, FILE* channel = stdout);
      
      static void die(const char* message, FILE* channel = stderr);

      static void show_progress(const char* message, int current, int total, FILE* channel = stdout);

      static void show_progress_end(const char* message, int total, FILE* channel = stdout);

      static void show_percentage_progress(const char* message, int current_percentage, FILE* channel = stdout);

      static void show_percentage_end(const char* message, FILE* channel = stdout);
  };
}
#endif // _SYSTEMMESSAGES_HPP_
