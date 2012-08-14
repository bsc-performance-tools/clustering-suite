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

  $Id:: ConvexHullModel.hpp 36 2011-11-21 11:00:1#$:  Id
  $Rev:: 36                                       $:  Revision of last commit
  $Author:: jgonzale                              $:  Author of last commit
  $Date:: 2011-11-21 12:00:12 +0100 (Mon, 21 Nov #$:  Date of last commit

\* -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=- */

#ifndef _BURSTSDB_HPP_
#define _BURSTSDB_HPP_

#include "clustering_types.h"

#include <Error.hpp>
using cepba_tools::Error;

#include <ParametersManager.hpp>
#include <CPUBurst.hpp>

#include <iostream>
using std::ostream;
#include <iterator>

#include <sqlite3.h>

class BurstsDB: public Error
{
  private:
    sqlite3           *DB;

    string             DBFileName;
    string             AllBurstsTable;
    string             CompleteBurstsView;

    string             TableCreationQuery;
    string             BurstInsertPrefix;

    bool               DBActive;

    vector<string>     ParamsNames;
    vector<string>     ExtraParamsNames;

    vector<instance_t> AllBurstsInstances;

    vector<instance_t> CompleteBurstsInstances;

    bool                NormalizedBursts;

  public:
    BurstsDB(void);
    BurstsDB(ParametersManager* Parameters);

    ~BurstsDB(void);

    void CloseDB(void);

    bool NewBurst(CPUBurst* Burst);

    bool NormalizeBursts(const vector<double>& MaxValues,
                         const vector<double>& MinValues,
                         const vector<double>& Factors);

    size_t AllBurstsSize(void) const      { return AllBurstsInstances.size(); };
    size_t CompleteBurstsSize(void) const { return CompleteBurstsInstances.size();};



  private:

    bool CreateDB(ParametersManager* Parameters);

    vector<vector <string> > Query(string QueryStr);

    CPUBurst* GetBurst(vector<instance_t>::iterator instance);

    static string       BurstTypeStr(burst_type_t typeVal);
    static burst_type_t BurstTypeVal(string typeStr);


  public:

    class iterator
    {
      private:
        BurstsDB*                    _db;
        vector<instance_t>::iterator _cur;

      public:

        typedef iterator   self_type;
        typedef CPUBurst*  value_type;
        typedef CPUBurst*& reference;
        typedef CPUBurst** pointer;
        typedef std::forward_iterator_tag iterator_category;
        typedef int difference_type;

        iterator(void)        : _db(NULL) {};
        iterator(vector<instance_t>::iterator cur,
                 BurstsDB*                    db): _cur(cur), _db(db)  {};
        iterator(const self_type& rhs): _db(rhs._db), _cur(rhs._cur) {};

        self_type  operator++() { ++_cur; return (*this); };
        self_type  operator++(int junk) { self_type tmp(*this); operator++(); return tmp; };
        value_type operator*()
        {
          if (_db == NULL)
          {
            return NULL;
          }
          else
          {
            return (_db->GetBurst(_cur));
          }
        };

        // pointer operator->() { return ptr_; }
        bool operator==(const self_type& rhs) { return _db == rhs._db && _cur == rhs._cur; };
        bool operator!=(const self_type& rhs) { return _db != rhs._db || _cur != rhs._cur; };


    };

    iterator all_bursts_begin() { iterator iter(AllBurstsInstances.begin(), this); return iter; };
    iterator all_bursts_end()   { iterator iter(AllBurstsInstances.end(), this); return iter; };

    iterator complete_bursts_begin() { iterator iter(CompleteBurstsInstances.begin(), this); return iter; };
    iterator complete_bursts_end()   { iterator iter(CompleteBurstsInstances.end(), this); return iter; };
};

#endif

