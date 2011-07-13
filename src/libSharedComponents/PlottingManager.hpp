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

#ifndef _PLOTTINGMANAGER_HPP_
#define _PLOTTINGMANAGER_HPP_

#include <trace_clustering_types.h>
#include <Error.hpp>
using cepba_tools::Error;

#include <ParametersManager.hpp>

#include <set>
using std::set;

class PlotDefinition;

class PlottingManager: public Error 
{
  private:
    bool                    DataExtraction;

    vector<PlotDefinition*> Plots;

    vector<bool>   CorrectPlots;
    vector<string> PlotsWarnings;

    INT32  ClusterIdPosition;
    
    static PlottingManager* _PlottingManager;
    
  public:
    static PlottingManager* GetInstance(bool DataExtraction);

    static PlottingManager* GetInstance(void);

    vector<bool>   GetCorrectPlots(void);
    vector<string> GetPlotsWarning(void);
    
    bool   PrintPlots(string             PlotsDataFileName,
                      string             PlotsFileNamePrefix,
                      string             ClusteringAlgorithmName,
                      set<cluster_id_t>& DifferentIDs);

  private:
    PlottingManager(bool DataExtraction);

    void GenerateAllPlots(ParametersManager* Parameters);
    
    void  LoadSinglePlot(PlotDefinition*    CurrentPlot,
                         ParametersManager* Parameters);

    INT32 CheckParameterPosition(string             ParameterName,
                                 bool               RawMetric,
                                 ParametersManager *Parameters);

    bool  PrintSinglePlot(string             FileNamePrefix,
                          string             DataFileName,
                          PlotDefinition    *Definition,
                          string             ClusteringAlgorithmName,
                          set<cluster_id_t>& DifferentIDs);

    void  Write2D_Definition(ostream& str,
                             INT32    X,
                             INT32    Y,
                             INT32    ClusterId,
                             string   ClusterName,
                             string   DataFileName);

    void  Write3D_Definition(ostream& str,
                             INT32    X,
                             INT32    Y,
                             INT32    Z,
                             INT32    ClusterId,
                             string   ClusterName,
                             string   DataFileName);

    string RGBStateColor(INT32 StateValue);
    
    string GetClusterName(cluster_id_t ID);
};

#endif // _PLOTTINGMANAGER_HPP_
