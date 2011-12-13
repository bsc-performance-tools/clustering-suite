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

#include "SystemMessages.hpp"
using cepba_tools::system_messages;

#include <iostream>
using std::cout;
using std::endl;

bool system_messages::verbose                 = false;
bool system_messages::distributed             = false;

int  system_messages::my_rank                 = -1;
bool system_messages::messages_from_all_ranks = false;

bool system_messages::print_timers            = false;

bool system_messages::percentage_ongoing      = false;
int  system_messages::last_percentage_written = 0;

void system_messages::information(string message, FILE* channel)
{
  system_messages::information(message.c_str(), channel);
}

void system_messages::information(const char* message, FILE* channel)
{
  if (system_messages::verbose)
  {
    if (system_messages::distributed)
    {
      if (system_messages::messages_from_all_ranks)
      {
        fprintf(channel, "[%d] %s", system_messages::my_rank, message);
      }
      else
      {
        if (my_rank == 0)
        {
          fprintf(channel, "[%d] %s", system_messages::my_rank, message);
        }
      }
    }
    else
    {
      fprintf(channel, "%s", message);
    }
  }
}

void system_messages::show_progress(const char* message,
                                    int         current,
                                    int         total,
                                    FILE*       channel)
{
  if (system_messages::verbose && !system_messages::distributed)
  {
      fprintf(channel, "\r%s %d/%d", message, current, total);
      fflush(channel);
  }
}

void system_messages::show_progress_end(const char* message,
                                        int         total,
                                        FILE*       channel)
{
  if (system_messages::verbose && !system_messages::distributed)
  {
    fprintf(channel, "\r%s %d/%d\n", message, total, total);
    fflush(channel);
  }
}

void system_messages::show_percentage_progress(const char* message,
                                               int         current_percentage,
                                               FILE*       channel)
{
  int real_percentage;

  if (current_percentage < 0)
    real_percentage = 0;
  else if (current_percentage > 100)
    real_percentage = 100;
  else
    real_percentage = current_percentage;

  if (system_messages::verbose)
  {
    if (!system_messages::distributed)
    {
      fprintf(channel, "\r%s %03d%%", message, real_percentage);
      fflush(channel);
    }
    else if (system_messages::messages_from_all_ranks ||
            (!system_messages::messages_from_all_ranks && my_rank == 0))
    {

      {
        if(!system_messages::percentage_ongoing)
        {
          fprintf(channel,
                  "[%d] %s %03d%%",
                  system_messages::my_rank,
                  message,
                  real_percentage);
          fflush(channel);
          system_messages::percentage_ongoing = true;
        }
        else
        {
          if (real_percentage % 10 == 0 && real_percentage != 100 )
          {
            fprintf(channel, "  %03d%%", real_percentage);
            fflush(channel);
          }
        }
      }
    }
  }

}

void system_messages::show_percentage_end(const char* message, FILE* channel)
{
  if (system_messages::verbose)
  {
    if (!system_messages::distributed)
    {
      fprintf(channel, "\r%s 100%%\n", message);
    }
    else
    {
      fprintf(channel, " 100%%\n");
      system_messages::percentage_ongoing = false;
    }
  }
}

void system_messages::show_timer(string message, Timer::diff_type Time, FILE* channel)
{
  show_timer(message.c_str(), Time, channel);
}

void system_messages::show_timer(const char* message, Timer::diff_type Time, FILE* channel)
{
  if (system_messages::print_timers)
  {
    if (!system_messages::distributed)
    {
      fprintf(channel, "%s %u us\n", message, Time);
    }
    else if (system_messages::messages_from_all_ranks ||
            (!system_messages::messages_from_all_ranks && my_rank == 0))
    {
      fprintf(channel, "%s %u us\n", message, Time);
    }
  }
}
