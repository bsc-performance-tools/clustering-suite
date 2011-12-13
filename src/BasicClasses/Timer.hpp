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

/*
  Simple timer class
  ==================
  History:

  Created - Sarah "Voodoo Doll" White (2006/01/26)
  ==================
  Description:

  A quick and easy timer class to save people
  from using ad hoc timers. The constructor
  begins the timer and the end() member function
  stops it, as well as returns the result.
  Before starting the timer again with begin(),
  users can see the last result with last()...

  So unintuitive. ;)
  ==================
  Bugs:

  The timing method used is neither strictly
  portable, though it only uses standard
  functions, nor is it guaranteed to support
  timing resolution less than a second.
*/

#ifndef _TIMER_HPP_
#define _TIMER_HPP_

#include <ctime>
#include <sys/time.h>

namespace cepba_tools
{
  /* class Timer {

    public:
      typedef double diff_type;

      // Same as Timer t; t.begin();
      Timer(): start(std::clock()), elapsed(0) {}
      // Last result before a call to begin()
      diff_type last() const { return elapsed; }
      // Reset the timer
      void begin() { start = std::clock(); elapsed = 0; }
      // Save the result
      diff_type end();

    private:
      std::clock_t start;
      diff_type    elapsed;
  };
  */

  class Timer {

    public:
      typedef unsigned long int diff_type;

      // Same as Timer t; t.begin();
      Timer(void);
      // Last result before a call to begin()
      diff_type last(void) const { return elapsed; }
      // Reset the timer
      void begin(void);
      // Save the result
      diff_type end(void);

    private:
      struct timeval time_start;

      diff_type elapsed;
  };
}
#endif
