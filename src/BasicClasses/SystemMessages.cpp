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

#include <cstring>
#include <cstdlib>

#include <iostream>
using std::cout;
using std::endl;

#include <sstream>
using std::ostringstream;

bool system_messages::verbose                 = false;
bool system_messages::distributed             = false;

int   system_messages::my_rank                 = -1;
bool  system_messages::messages_from_all_ranks = false;
char* system_messages::rank_prefix             = NULL;

bool system_messages::print_timers            = false;

bool system_messages::percentage_ongoing      = false;
int  system_messages::last_percentage_written = 0;

bool system_messages::progress_ongoing        = false;


void system_messages::set_rank_prefix(const char* desired_rank_prefix)
{
  ostringstream prefix;
  prefix << desired_rank_prefix << " ";
  system_messages::rank_prefix = (char*) malloc(strlen(prefix.str().c_str())+1);
  strcpy(system_messages::rank_prefix, prefix.str().c_str());
}

void system_messages::information(string message,
                                  FILE*  channel)
{
  system_messages::information(message.c_str(), channel);
}

void system_messages::information(const char* message,
                                  FILE*       channel)
{
  if (system_messages::verbose)
  {
    if (system_messages::distributed)
    {
      if (system_messages::messages_from_all_ranks)
      {
        fprintf(channel,
                "[%s%d] %s",
                system_messages::rank_prefix,
                system_messages::my_rank,
                message);
      }
      else
      {
        if (my_rank == 0)
        {
          fprintf(channel,
                  "[%s%d] %s",
                  system_messages::rank_prefix,
                  system_messages::my_rank,
                  message);
        }
      }
    }
    else
    {
      fprintf(channel, "%s", message);
    }
  }
}

void system_messages::silent_information(string message,
                                         FILE  *channel)
{
  system_messages::silent_information(message.c_str(), channel);
}

void system_messages::silent_information(const char* message,
                                         FILE*       channel)
{
  if (!system_messages::verbose)
  {
    if (system_messages::distributed)
    {
      if (system_messages::messages_from_all_ranks)
      {
        fprintf(channel,
                "[%s%d] %s",
                system_messages::rank_prefix,
                system_messages::my_rank,
                message);
      }
      else
      {
        if (my_rank == 0)
        {
          fprintf(channel,
                  "[%s%d] %s",
                  system_messages::rank_prefix,
                  system_messages::my_rank,
                  message);
        }
      }
    }
    else
    {
      fprintf(channel, "%s", message);
    }
  }
}


void system_messages::show_progress(string message,
                                    int    current,
                                    int    total,
                                    FILE*  channel)
{
  system_messages::show_progress(message.c_str(),
                                 current,
                                 total,
                                 channel);
}

void system_messages::show_progress(const char* message,
                                    int         current,
                                    int         total,
                                    FILE*       channel)
{
  if (system_messages::verbose)
  {
    if (!system_messages::distributed)
    {
      fprintf(channel, "\r%s %d/%d", message, current, total);
      fflush(channel);
    }
    else
    {
      int current_percentage, real_percentage;

      current_percentage = (100*current)/total;

      if (current_percentage < 0)
        real_percentage = 0;
      else if (current_percentage > 100)
        real_percentage = 100;
      else
       real_percentage = current_percentage;

      if (system_messages::messages_from_all_ranks ||
         (!system_messages::messages_from_all_ranks && system_messages::my_rank == 0))
      {
        if(!system_messages::progress_ongoing)
        {
          fprintf(channel,
                  "[%s%d] %s %03d%%",
                  system_messages::rank_prefix,
                  system_messages::my_rank,
                  message,
                  real_percentage);
          fflush(channel);
          system_messages::progress_ongoing        = true;
          system_messages::last_percentage_written = 0;
        }
        else
        {
          if (real_percentage % 10 == 0 && real_percentage != 100 &&
              real_percentage > system_messages::last_percentage_written)
          {
            fprintf(channel, " %03d%%", real_percentage);
            fflush(channel);
            system_messages::last_percentage_written = real_percentage;
          }
        }
      }
    }
  }
}

void system_messages::show_progress_end(string message,
                                        int    total,
                                        FILE*  channel)
{
  system_messages::show_progress_end(message.c_str(),
                                     total,
                                     channel);
}

void system_messages::show_progress_end(const char* message,
                                        int         total,
                                        FILE*       channel)
{
  if (system_messages::verbose)
  {
    if (!system_messages::distributed)
    {
      fprintf(channel, "\r%s %d/%d\n", message, total, total);
      fflush(channel);
    }
    else
    {
      if (system_messages::messages_from_all_ranks ||
            (!system_messages::messages_from_all_ranks && system_messages::my_rank == 0))
      {
        fprintf(channel, " 100%%\n", system_messages::my_rank);
        system_messages::progress_ongoing = false;
      }

      fflush(channel);
    }
  }
}

void system_messages::show_percentage_progress(string  message,
                                               int     current_percentage,
                                               FILE*   channel)
{
  system_messages::show_percentage_progress(message.c_str(),
                                            current_percentage,
                                            channel);
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
            (!system_messages::messages_from_all_ranks && system_messages::my_rank == 0))
    {

      {
        if(!system_messages::percentage_ongoing)
        {
          fprintf(channel,
                  "[%s%d] %s %03d%%",
                  system_messages::rank_prefix,
                  system_messages::my_rank,
                  message,
                  real_percentage);
          fflush(channel);
          system_messages::percentage_ongoing      = true;
          system_messages::last_percentage_written = 0;
        }
        else
        {
          if (real_percentage % 10 == 0 && real_percentage != 100 &&
              real_percentage > system_messages::last_percentage_written)
          {
            fprintf(channel, " %03d%%", real_percentage);
            fflush(channel);
            system_messages::last_percentage_written = real_percentage;
          }
        }
      }
    }
  }

}

void system_messages::show_percentage_end(string message,
                                          FILE*  channel)
{
  system_messages::show_percentage_end(message.c_str(), channel);
}

void system_messages::show_percentage_end(const char* message,
                                          FILE*       channel)
{
  if (system_messages::verbose)
  {
    if (!system_messages::distributed)
    {
      fprintf(channel, "\r%s 100%%\n", message);
    }
    else if (system_messages::messages_from_all_ranks ||
            (!system_messages::messages_from_all_ranks && system_messages::my_rank == 0))
    {
      fprintf(channel, " 100%%\n", system_messages::my_rank);
      system_messages::percentage_ongoing = false;
    }

    fflush(channel);
  }
}

void system_messages::show_timer(string           message,
                                 Timer::diff_type Time,
                                 FILE*            channel)
{
  show_timer(message.c_str(), Time, channel);
}

void system_messages::show_timer(const char*      message,
                                 Timer::diff_type Time,
                                 FILE*            channel)
{
  if (system_messages::print_timers)
  {
    if (!system_messages::distributed)
    {
      fprintf(channel, "%s %lu us\n", message, Time);
    }
    else if (system_messages::messages_from_all_ranks ||
            (!system_messages::messages_from_all_ranks && system_messages::my_rank == 0))
    {
      fprintf(channel, "[%s%d] %s %lu us\n",
              system_messages::rank_prefix,
              system_messages::my_rank,
              message,
              Time);
    }

    fflush(channel);
  }
}

extern "C" void show_percentage_progress(const char* message,
                                         int         current_percentage,
                                         FILE*       channel)
{
  system_messages::show_percentage_progress(message,
                                            current_percentage,
                                            channel);
}

extern "C" void show_percentage_end(const char* message,
                                    FILE*       channel)
{
  system_messages::show_percentage_end(message, channel);
}
