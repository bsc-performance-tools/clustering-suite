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

  $Id:: SystemMessages.cpp 23 2011-05-17 09:47:12#$:  Id
  $Rev:: 23                                       $:  Revision of last commit
  $Author:: jgonzale                              $:  Author of last commit
  $Date:: 2011-05-17 11:47:12 +0200 (Tue, 17 May #$:  Date of last commit

\* -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=- */

#include "Utilities.hpp"

#include <iostream>
using std::cout;
using std::endl;

#include <cstring>

#include <execinfo.h>
#include <cxxabi.h>

std::string cepba_tools::GetCWD(void)
{
  char cCurrentPath[FILENAME_MAX];

  if (!GetCurrentDir(cCurrentPath, sizeof(cCurrentPath)))
  {
    return std::string("unable to retrive CWD");
  }

  cCurrentPath[sizeof(cCurrentPath) - 1] = '\0'; /* not really required */

  return std::string(cCurrentPath);
}

#include <algorithm>
#include <functional>
#include <cctype>
#include <locale>

// trim from start
std::string& cepba_tools::ltrim(std::string &s)
{
  s.erase(s.begin(), std::find_if(s.begin(), s.end(), std::not1(std::ptr_fun<int, int>(std::isspace))));
  return s;
}

// trim from end
std::string& cepba_tools::rtrim(std::string &s)
{
  s.erase(std::find_if(s.rbegin(), s.rend(), std::not1(std::ptr_fun<int, int>(std::isspace))).base(), s.end());
  return s;
}

// trim from both ends
std::string& cepba_tools::trim(std::string &s)
{
  return ltrim(rtrim(s));
}

unsigned long long cepba_tools::getTimeFromStr (char *time, char *envvar)
{
  unsigned long long MinTimeFactor;
  char tmp_buff[256];
  size_t strl;

  if (time == NULL)
  return 0;

  strncpy (tmp_buff, time, sizeof(tmp_buff));

  strl = strlen(tmp_buff);

  if (strl > 2 && cepba_tools::is_Alphabetic(tmp_buff[strl-2]) && tmp_buff[strl-1] == 's')
  {
    tmp_buff[strl-1] = 0x0; // Strip the last 's' of ms/ns/us
  }

  switch (tmp_buff[strlen(tmp_buff)-1])
  {
  case 'D': /* Days */
    MinTimeFactor = 24*60*60;
    MinTimeFactor *= 1000000000;
    tmp_buff[strlen(tmp_buff)-1] = (char) 0;
    break;

  case 'M': /* Minutes */
    MinTimeFactor = 60;
    MinTimeFactor *= 1000000000;
    tmp_buff[strlen(tmp_buff)-1] = (char) 0;
    break;

  case 'H': /* Hours */
    MinTimeFactor = 60*60;
    MinTimeFactor *= 1000000000;
    tmp_buff[strlen(tmp_buff)-1] = (char) 0;
    break;

  case 's': /* Seconds */
  case 'S':
    MinTimeFactor = 1;
    MinTimeFactor *= 1000000000;
    tmp_buff[strlen(tmp_buff)-1] = (char) 0;
    break;

  case 'm': /* Milliseconds */
    MinTimeFactor = 1000000;
    tmp_buff[strlen(tmp_buff)-1] = (char) 0;
    break;

  case 'u': /* Microseconds */
    MinTimeFactor = 1000;
    tmp_buff[strlen(tmp_buff)-1] = (char) 0;
    break;

  case 'n': /* Nanoseconds */
    MinTimeFactor = 1;
    tmp_buff[strlen(tmp_buff)-1] = (char) 0;
    break;

  default :
    /* If no time-factor or wrong time-factor introduced, we apply microseconds! */
    MinTimeFactor = 1;
    MinTimeFactor *= 1000;
    /* If the last char is a number, then the time units are omitted!
    if (tmp_buff[strlen(tmp_buff)-1] >= '0' && tmp_buff[strlen(tmp_buff)-1] <= '9')
    {
      ostringstream Message;
      Message << envvar << " time units not specified. Using microseconds";
      SetWarning(true);
      SetWarningMessage(Message.str());
    }
    else
    {
      ostringstream Message;
      Message << envvar << " time units not unknown! Using microseconds";
      SetWarning(true);
      SetWarningMessage(Message.str());
    }
    */

    break;
  }

  return atoll (tmp_buff) * MinTimeFactor;
}

int cepba_tools::is_Alphabetic(char c)
{
  /* Avoid using isspace() and iscntrl() to remove internal dependency with ctype_b/ctype_b_loc.
   * This symbol name depends on the glibc version; newer versions define ctype_b_loc and compat
   * symbols have been removed. This dependency may end up in undefined references when porting
   * binaries to machines with different glibc versions.
   */

  return (c >= 'A' && c <= 'Z') || (c >= 'a' && c <= 'z');
}

/** Print a demangled stack backtrace of the caller function to FILE* out.
 *  OBTAINED FROM:
 * http://panthema.net/2008/0901-stacktrace-demangled/
 */
void cepba_tools::print_stacktrace(FILE *out, unsigned int max_frames)
{
  fprintf(out, "stack trace:\n");

  // storage array for stack trace address data
  void* addrlist[max_frames+1];

  // retrieve current stack addresses
  int addrlen = backtrace(addrlist, sizeof(addrlist) / sizeof(void*));

  if (addrlen == 0) {
    fprintf(out, "  <empty, possibly corrupt>\n");
    return;
  }

  // resolve addresses into strings containing "filename(function+address)",
  // this array must be free()-ed
  char** symbollist = backtrace_symbols(addrlist, addrlen);

  // allocate string which will be filled with the demangled function name
  size_t funcnamesize = 256;
  char* funcname = (char*)malloc(funcnamesize);

  // iterate over the returned symbol lines. skip the first, it is the
  // address of this function.
  for (int i = 1; i < addrlen; i++)
  {
    char *begin_name = 0, *begin_offset = 0, *end_offset = 0;

    // find parentheses and +address offset surrounding the mangled name:
    // ./module(function+0x15c) [0x8048a6d]
    for (char *p = symbollist[i]; *p; ++p)
    {
      if (*p == '(')
        begin_name = p;
      else if (*p == '+')
        begin_offset = p;
      else if (*p == ')' && begin_offset)
      {
        end_offset = p;
        break;
      }
    }

    if (begin_name && begin_offset && end_offset
        && begin_name < begin_offset)
    {
      *begin_name++ = '\0';
      *begin_offset++ = '\0';
      *end_offset = '\0';

      // mangled name is now in [begin_name, begin_offset) and caller
      // offset in [begin_offset, end_offset). now apply
      // __cxa_demangle():

      int status;
      char* ret = abi::__cxa_demangle(begin_name,
              funcname, &funcnamesize, &status);
      if (status == 0)
      {
        funcname = ret; // use possibly realloc()-ed string
        fprintf(out, "  %s : %s+%s\n",
                symbollist[i], funcname, begin_offset);
      }
      else
      {
        // demangling failed. Output function name as a C function with
        // no arguments.
        fprintf(out, "  %s : %s()+%s\n",
                symbollist[i], begin_name, begin_offset);
      }
    }
    else
    {
      // couldn't parse the line? print the whole line.
      fprintf(out, "  %s\n", symbollist[i]);
    }
  }

  free(funcname);
  free(symbollist);

  return;
}


