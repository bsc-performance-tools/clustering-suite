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

#ifndef _PARAMETERSMANAGER_HPP_
#define _PARAMETERSMANAGER_HPP_

#include <Error.hpp>
using cepba_tools::Error;

#include <ClusteringConfiguration.hpp>
#include <ClusteringParameter.hpp>

#include <vector>
using std::vector;

/**
 * Singleton container of clustering and extrapolation parameters. Used to
 * extract data from traces, and also apply the basic normalizations and
 * transformations
 */
class ParametersManager: public Error 
{
  public:
    typedef map<string, INT32 >::iterator       ParametersPositionIterator;
    typedef map<string, INT32 >::const_iterator ParametersPositionConstIterator;

  private:
    static ParametersManager* _Parameters;

    ParametersManager(void);

    map<string, INT32>           ClusteringParametersIndex;
    vector<ClusteringParameter*> ClusteringParameters;

    map<string, INT32>           ExtrapolationParametersIndex;
    vector<ClusteringParameter*> ExtrapolationParameters;
    
  public:
    static ParametersManager* GetInstance(void);

    size_t GetClusteringParametersSize(void);
    size_t GetExtrapolationParametersSize(void);

    size_t GetClusteringParameterPosition(string ClusteringParameterName);
    size_t GetExtrapolationParameterPosition(string ExtrapolationParameterName);

    vector<string> GetClusteringParametersNames    (void);
    vector<string> GetExtrapolationParametersNames (void);

    vector<bool> GetClusteringParametersPrecision    (void);
    vector<bool> GetExtrapolationParametersPrecision (void);

    vector<double> GetClusteringParametersFactors(void);
    
    void Clear(void);
    
    void NewData(map<event_type_t, event_value_t>& EventsData);

    burst_type_t GetData(vector<double>&      ClusteringRawData,
                         vector<double>&      ClusteringProcessedData,
                         map<size_t, double>& ExtrapolationData);

  private:
    bool LoadParameters(ClusteringConfiguration* Configuration);

    ClusteringParameter* CreateParameter(string              ParameterName,
                                         ParameterContainer& ParameterDefinition);


    derived_event_op_t GetOperation(char Operation);

    void LoadCPIStackExtrapolationParameters(void);

private:

};

#endif // _PARAMETERSMANAGER_HPP_
