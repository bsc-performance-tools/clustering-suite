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

#include <SystemMessages.hpp>
using cepba_tools::system_messages;

#include <ClusteringConfiguration.hpp>

#include "PlottingManager.hpp"

#include <ParaverColors.h>

#include <fstream>
using std::ofstream;

#include <fstream>
using std::ofstream;
using std::ios_base;

#include <sstream>
using std::ostringstream;

using std::endl;

#include <cerrno>
#include <cstring>

#include "trace_clustering_types.h"

/******************************************************************************
 * Singleton pointer
 *****************************************************************************/

/**
 * Singleton pointer
 */
PlottingManager* PlottingManager::_PlottingManager = NULL;

/******************************************************************************
 * GetInstance methods
 *****************************************************************************/

/**
 * Returns the single instance of the plotting manager and creates it if doesn't
 * exists
 * \param  Boolean indicating if data to plot is clustered or just extracted from a file
 * \return The plotting manager singleton instance
 */
PlottingManager*
PlottingManager::GetInstance(bool DataExtraction)
{
  if (PlottingManager::_PlottingManager == NULL)
  {
    PlottingManager::_PlottingManager = new PlottingManager(DataExtraction);
  }

  return PlottingManager::_PlottingManager;
}

/**
 * Returns the single instance of the plotting manager, *do not create it* if
 * it does not exists
 *
 * \return Plotting manager if it exists. NULL otherwise.
 *
 */
PlottingManager*
PlottingManager::GetInstance(void)
{
  return PlottingManager::_PlottingManager;
}

/******************************************************************************
 * State consults
 *****************************************************************************/

vector<bool>
PlottingManager::GetCorrectPlots(void)
{
  return this->CorrectPlots;
}

vector<string>
PlottingManager::GetPlotsWarning(void)
{
  return this->PlotsWarnings;
}

/******************************************************************************
 * Print Plots method, main method of the class
 *****************************************************************************/

/**
 * Print all plots correctly defined
 * \param PlotsFileNameHeader Heading text for the scripts file names
 * \param PlotsDataFileName Name of the file where data plots data will be located
 * \return True if all plots have been written correctly, false otherwise
 */
bool
PlottingManager::PrintPlots(string PlotsDataFileName,
                            string PlotsFileNamePrefix,
                            size_t NumberOfClusters)
{

  system_messages::show_progress("Writing plots to disc", 0, Plots.size());
  for (size_t i = 0; i < Plots.size(); i++)
  {
    if(CorrectPlots[i])
    {
      if (!PrintSinglePlot (PlotsFileNamePrefix,
                            PlotsDataFileName,
                            Plots[i],
                            NumberOfClusters))
      {
        SetError(true);
        return false;
      }
    }
    system_messages::show_progress("Writing plots to disc", i, Plots.size());
  }
  system_messages::show_progress_end("Writing plots to disc", Plots.size());
  
  return true;
}

/******************************************************************************
 * Class private constructor
 *****************************************************************************/

/**
 * Creates the single instance of the class using the clustering configuration
 */
PlottingManager::PlottingManager(bool DataExtraction)
{
  ClusteringConfiguration* Configuration;
  ParametersManager*       Parameters;

  this->DataExtraction = DataExtraction;

  Configuration = ClusteringConfiguration::GetInstance();
  if (!Configuration->IsInitialized())
  {
    SetError(true);
    SetErrorMessage("configuration not initialized");
    return;
  }

  Parameters = ParametersManager::GetInstance();

  if (Parameters == NULL)
  {
    SetError(true);
    SetErrorMessage("parameters not loaded correctly");
    return;
  }

  if (Configuration->GetAllPlots())
  {
    GenerateAllPlots (Parameters);
  }
  else
  {
    if (Configuration->GetPlotsDefinitionsError())
    {
      SetError(true);
      SetErrorMessage(Configuration->GetPlotsDefinitionsErrorMessage());
      return;
    }

    Plots = Configuration->GetPlotsDefinitions();

    for (INT32 i = 0; i < Plots.size(); i++)
    {
      LoadSinglePlot(Plots[i], Parameters);
    }
  }

  ClusterIdPosition = CSV_HEADING_FIELDS +
                      Parameters->GetClusteringParametersSize ()*2 +
                      Parameters->GetExtrapolationParametersSize () + 1;
  
  return;
}

/**
 * Generates all possible combinations of parameters
 *
 * \param Parameters Parameters manager to get parameters read from the XML
 */
void
PlottingManager::GenerateAllPlots(ParametersManager* Parameters)
{
  INT32 ClusteringParameters;
  INT32 ExtrapolationParameters;

  vector<string> ClusteringParametersNames;
  vector<string> ExtrapolationParametersNames;


  PlotDefinition* Definition;
  
  ClusteringParameters      = Parameters->GetClusteringParametersSize();
  ClusteringParametersNames = Parameters->GetClusteringParametersNames();
  
  ExtrapolationParameters      = Parameters->GetExtrapolationParametersSize();
  ExtrapolationParametersNames = Parameters->GetExtrapolationParametersNames();

  for (INT32 X = 0; X < ClusteringParameters-1; X++)
  {
    for (INT32 Y = X+1; Y < ClusteringParameters; Y++)
    { /* Combination of clustering parameters, raw and normalized */

      /* Raw combination */
      Definition = new PlotDefinition();

      Definition->RawMetrics      = true;
      Definition->ThreeDimensions = false;

      Definition->XMetric         = ClusteringParametersNames[X];
      Definition->XMetricTitle    = ClusteringParametersNames[X];
      Definition->XMetricPosition = CSV_HEADING_FIELDS + X + 1;

      Definition->YMetric         = ClusteringParametersNames[Y];
      Definition->YMetricTitle    = ClusteringParametersNames[Y];
      Definition->YMetricPosition = CSV_HEADING_FIELDS + Y + 1;

      Plots.push_back(Definition);
      CorrectPlots.push_back(true);
      PlotsWarnings.push_back("Correct Plot");
      
      /* Normalized */
      Definition = new PlotDefinition();

      Definition->RawMetrics      = false;
      Definition->ThreeDimensions = false;

      Definition->XMetric         = ClusteringParametersNames[X];
      Definition->XMetricTitle    = ClusteringParametersNames[X]+" Normalized";
      Definition->XMetricPosition = CSV_HEADING_FIELDS + ClusteringParameters + X + 1;

      Definition->YMetric         = ClusteringParametersNames[Y];
      Definition->YMetricTitle    = ClusteringParametersNames[Y]+" Normalized";
      Definition->YMetricPosition = CSV_HEADING_FIELDS + ClusteringParameters + Y + 1;

      Plots.push_back(Definition);
      CorrectPlots.push_back(true);
      PlotsWarnings.push_back("Correct Plot");


      for (INT32 Z = 0; Z < ExtrapolationParameters; Z++)
      { /* Combination of each pair of clustering metrics and one extrapolation metric */
        Definition = new PlotDefinition();

        Definition->RawMetrics      = true;
        Definition->ThreeDimensions = true;

        Definition->XMetric         = ClusteringParametersNames[X];
        Definition->XMetricTitle    = ClusteringParametersNames[X];
        Definition->XMetricPosition = CSV_HEADING_FIELDS + X + 1;

        Definition->YMetric         = ClusteringParametersNames[Y];
        Definition->YMetricTitle    = ClusteringParametersNames[Y];
        Definition->YMetricPosition = CSV_HEADING_FIELDS + Y + 1;

        Definition->ZMetric         = ExtrapolationParametersNames[Z];
        Definition->ZMetricTitle    = ExtrapolationParametersNames[Z];
        Definition->ZMetricPosition = CSV_HEADING_FIELDS + 
                                      (2*ClusteringParameters) + Z + 1;
        
        Plots.push_back(Definition);
        CorrectPlots.push_back(true);
        PlotsWarnings.push_back("Correct Plot");
      }

      /* And 3D plot with duration */
      Definition = new PlotDefinition();
      
      Definition->RawMetrics      = true;
      Definition->ThreeDimensions = true;

      Definition->XMetric         = ClusteringParametersNames[X];
      Definition->XMetricTitle    = ClusteringParametersNames[X];
      Definition->XMetricPosition = CSV_HEADING_FIELDS + X + 1;

      Definition->YMetric         = ClusteringParametersNames[Y];
      Definition->YMetricTitle    = ClusteringParametersNames[Y];
      Definition->YMetricPosition = CSV_HEADING_FIELDS + Y + 1;

      Definition->ZMetric         = "Duration";
      Definition->ZMetricTitle    = "Duration";
      Definition->ZMetricPosition = CSV_DURATION_FIELD;
      
      Plots.push_back(Definition);
      CorrectPlots.push_back(true);
      PlotsWarnings.push_back("Correct Plot");
    }
  }
}

/******************************************************************************
 * Plots definitions loader
 *****************************************************************************/

/**
 * Checks the definition of a single plot read from the XML, if some parameters are missing
 * it adds a warning message into the warnings vector, and sets the plot as incorrect plot
 * in the correct plots vector
 * \param CurrentPlot Configuration object describing the plot extracted from the XML
 * \param Parameters Parameters manager object to find the positions of the different parameters
 */
void
PlottingManager::LoadSinglePlot(PlotDefinition*    CurrentPlot,
                                ParametersManager *Parameters)
{
  INT32           XMetricPosition, YMetricPosition, ZMetricPosition;
  bool            Error = false;
  
  /* Check parameters positions on the XML file */

  /* X axis */
  CurrentPlot->XMetricPosition = CheckParameterPosition(CurrentPlot->XMetric,
                                                        CurrentPlot->RawMetrics,
                                                        Parameters);

  if (CurrentPlot->XMetricPosition == NOT_FOUND)
  {
    ostringstream Message;
    Message << "X metric not found on plot defined in line " << CurrentPlot->Line;

    PlotsWarnings.push_back(Message.str());
    CorrectPlots.push_back(false);
    Error = true;
    return;
  }

  /* Y axis */
  CurrentPlot->YMetricPosition = CheckParameterPosition(CurrentPlot->YMetric,
                                                        CurrentPlot->RawMetrics,
                                                        Parameters);

  if (CurrentPlot->YMetricPosition == NOT_FOUND)
  {
    ostringstream Message;
    Message << "Y metric not found on plot defined in line " << CurrentPlot->Line;

    PlotsWarnings.push_back(Message.str());
    CorrectPlots.push_back(false);
    Error = true;
    return;
  }

  
  
  if (CurrentPlot->ThreeDimensions)
  {
    CurrentPlot->ZMetricPosition = CheckParameterPosition(CurrentPlot->ZMetric,
                                                          CurrentPlot->RawMetrics,
                                                          Parameters);

    if (CurrentPlot->ZMetricPosition == NOT_FOUND)
    {
      ostringstream Message;
      Message << "Z metric not found on plot defined in line " << CurrentPlot->Line;

      PlotsWarnings.push_back(Message.str());
      CorrectPlots.push_back(false);
      Error = true;
    }
  }

  if (!Error)
  {
    PlotsWarnings.push_back("Correct Plot");
    CorrectPlots.push_back(true);
  }

  return;
}

/**
 * Returns for the position of a given paramater into the CSV file
 * \param ParameterName Name of the parameter we want the position
 * \param RawMetric Boolean indicating if the parameter values should be the raw or normalized
 * \param Parameters Parameters manager to obtain the positions
 * \return The parameter position if found, Constants::NOT_FOUND otherwise
 */
INT32
PlottingManager::CheckParameterPosition(string             ParameterName,
                                        bool               RawMetric,
                                        ParametersManager *Parameters)
{
  INT32 Position;

  if (ParameterName.compare("Duration") == 0 ||
      ParameterName.compare("duration") == 0)
  {
    return CSV_DURATION_FIELD;
  }
  
  if (!RawMetric)
  { /* Normalized metric, just look into clustering parameters */
    Position = Parameters->GetClusteringParameterPosition(ParameterName);

    if (Position != NOT_FOUND)
    { /* Adjust to real position in the CSV (add number of heading fields + number of raw metrics) */
      Position += CSV_HEADING_FIELDS;
      Position += Parameters->GetClusteringParametersSize();
    }
  }
  else
  { /* Raw metric, could be a clustering parameter or an extrapolation parameter */

    Position = Parameters->GetClusteringParameterPosition(ParameterName);

    if (Position != NOT_FOUND)
    {
      Position += CSV_HEADING_FIELDS;
    }
    else
    { /* Look into extrapolation parameters */
      Position = Parameters->GetExtrapolationParameterPosition(ParameterName);

      if (Position != NOT_FOUND)
      { /* Heading fields + Raw clustering metrics + Normalized clustering metrics */
        Position += CSV_HEADING_FIELDS;
        Position += (Parameters->GetClusteringParametersSize()*2);
      }
    }
  }

  return Position+1;
}


/**
 * Writes to disc a single plot of the plots vector
 *
 * \param FileNamePrefix Prefix of the output script
 * \param Definition Container of the plot definition
 * 
 * \return True if the plot has been written correctly, false otherwise
 */
bool
PlottingManager::PrintSinglePlot(string          FileNamePrefix,
                                 string          DataFileName,
                                 PlotDefinition *Definition,
                                 size_t          NumberOfClusters)
{
  ofstream OutputStream;
  string   OutputFileName;

  if (!Definition->ThreeDimensions)
  {
    OutputFileName  = FileNamePrefix;
    OutputFileName += "."+Definition->XMetric;
    OutputFileName += "."+Definition->YMetric;

    if (!Definition->RawMetrics)
    {
     OutputFileName += ".Normalized";
    }

    OutputFileName += ".gnuplot";
  }
  else
  {
    OutputFileName  = FileNamePrefix;
    OutputFileName += ".3D";
    OutputFileName += "."+Definition->XMetric;
    OutputFileName += "."+Definition->YMetric;
    OutputFileName += "."+Definition->ZMetric;
    OutputFileName += ".gnuplot";
  }
  
  OutputStream.open(OutputFileName.c_str(), ios_base::trunc);

  if (!OutputStream)
  {
    SetError(true);
    SetErrorMessage("unable to open script file", strerror(errno));
    return false;
  }

  /* Write the plot title and common commands */

  OutputStream << "set datafile separator \",\"" << endl;
  OutputStream << endl;
  OutputStream << "set title ";

  if (DataExtraction)
  {
    OutputStream << "\"Data from file " << DataFileName << "\"" << endl;
  }
  else
  {
    OutputStream << "\"Clustering (DBSCAN To Do Work)\"" << endl;
  }

  OutputStream << "set xlabel \"" << Definition->XMetricTitle << "\"" << endl;
  OutputStream << "set ylabel \"" << Definition->YMetricTitle << "\"" << endl;
  
  /*
  UNCLASSIFIED;
  MISSING_DATA_CLUSTERID;
  DURATION_FILTERED_CLUSTERID;
  RANGE_FILTERED_CLUSTERID;
  THRESHOLD_FILTERED_CLUSTERID;
  */ 
  
  /* Print concrete commands */
  if (Definition->ThreeDimensions)
  {
    INT32 X, Y, Z;

    X = Definition->XMetricPosition;
    Y = Definition->YMetricPosition;
    Z = Definition->ZMetricPosition;

    /* Z label */
    OutputStream << "set zlabel \"" << Definition->ZMetricTitle << "\"" << endl;

    OutputStream << "splot ";

    if (NumberOfClusters != 0)
    {
      Write3D_Definition (OutputStream, X, Y, Z, UNCLASSIFIED+PARAVER_OFFSET, "Unclassified", DataFileName);
    }

    if (Definition->RawMetrics)
    {
      // OutputStream << ",\\" << endl;
      // Write3D_Definition (OutputStream, X, Y, Z, MISSING_DATA_CLUSTERID, "Missing Data", DataFileName);
      OutputStream << ",\\" << endl;
      Write3D_Definition (OutputStream, X, Y, Z, DURATION_FILTERED_CLUSTERID, "Duration Filtered", DataFileName);
      OutputStream << ",\\" << endl;
      Write3D_Definition (OutputStream, X, Y, Z, RANGE_FILTERED_CLUSTERID, "Range Filtered", DataFileName);
      OutputStream << ",\\" << endl;
      Write3D_Definition (OutputStream, X, Y, Z, THRESHOLD_FILTERED_CLUSTERID, "Threshold Filtered", DataFileName);

      if (NumberOfClusters != 0)
      {
        OutputStream << ",\\" << endl;
        Write3D_Definition (OutputStream, X, Y, Z, NOISE_CLUSTERID+PARAVER_OFFSET, "Noise", DataFileName);
      
        for (size_t i = 0; i < NumberOfClusters; ++i)
        {
          ostringstream ClusterName;

          ClusterName << "Cluster " << (i+1);
          
          OutputStream << ",\\" << endl;
          Write3D_Definition (OutputStream, X, Y, Z, i+PARAVER_OFFSET, ClusterName.str(), DataFileName);
        }
      }
    }
  }
  else
  {
    INT32 X, Y;

    X = Definition->XMetricPosition;
    Y = Definition->YMetricPosition;

    OutputStream << "plot ";

    if (NumberOfClusters != 0)
    {
      Write2D_Definition (OutputStream, X, Y, UNCLASSIFIED+PARAVER_OFFSET, "Unclassified", DataFileName);
    }

    if (Definition->RawMetrics)
    {
      OutputStream << ",\\" << endl;
      // Write2D_Definition (OutputStream, X, Y, MISSING_DATA_CLUSTERID, "Missing Data", DataFileName);
      // OutputStream << ",\\" << endl;
      Write2D_Definition (OutputStream, X, Y, DURATION_FILTERED_CLUSTERID, "Duration Filtered", DataFileName);
      OutputStream << ",\\" << endl;
      Write2D_Definition (OutputStream, X, Y, RANGE_FILTERED_CLUSTERID, "Range Filtered", DataFileName);
      OutputStream << ",\\" << endl;
      Write2D_Definition (OutputStream, X, Y, THRESHOLD_FILTERED_CLUSTERID, "Threshold Filtered", DataFileName);

      if (NumberOfClusters != 0)
      {
        OutputStream << ",\\" << endl;
        Write2D_Definition (OutputStream, X, Y, NOISE_CLUSTERID+PARAVER_OFFSET, "Noise", DataFileName);
        
        for (size_t i = 0; i < NumberOfClusters; ++i)
        {
          ostringstream ClusterName;
          ClusterName << "Cluster " << (i+1);

          OutputStream << ",\\" << endl;
          Write2D_Definition (OutputStream, X, Y, MIN_CLUSTERID+i+PARAVER_OFFSET, ClusterName.str(), DataFileName);
        }
      }
    }
  }

  /* Print clusters */

  OutputStream << endl;
  
  /* Print pause command */
  OutputStream << "pause -1 \"Press return to continue...\"" << endl;
  
  /* Check if there is any error in the output */
  if (OutputStream.fail())
  {
    SetError(true);
    SetErrorMessage("error while writing a script file", strerror(errno));
    return false;
  }

  return true;
}

/**
 * Write the GNUPlot commands to print a 2 dimensional plot
 *
 * \param str Stream to write the script
 */
void
PlottingManager::Write2D_Definition(ostream& str,
                                    INT32    X,
                                    INT32    Y,
                                    INT32    ClusterId,
                                    string   ClusterName,
                                    string   DataFileName)
{
  str << "\'" << DataFileName << "\' using ";

  str << X << ":($" << ClusterIdPosition << " == " << ClusterId << " ? $" << Y << " : 1/0) ";

  str << "w points ps 1.5 lt rgbcolor \"" << RGBStateColor(ClusterId) << "\" ";

  str << "title \"" << ClusterName << "\"";

}

/**
 * Write the GNUPlot commands to print a 3 dimensional plot
 *
 * \param str Stream to write the script
 */
void
PlottingManager::Write3D_Definition(ostream& str,
                                    INT32    X,
                                    INT32    Y,
                                    INT32    Z,
                                    INT32    ClusterId,
                                    string   ClusterName,
                                    string   DataFileName)
{
  str << "\'" << DataFileName << "\' using ";

  str << X << ":" << Y << ":($" << ClusterIdPosition << " == " << ClusterId << " ? $" << Z << " : 1/0) ";

  str << "w points ps 1.5 lt rgbcolor \"" << RGBStateColor(ClusterId) << "\" ";

  str << "title \"" << ClusterName << "\"";
}

/**
 * Returns the standard Paraver color state color for a given state number
 *
 * \param ClusterId/State color to find the RGB codification
 *
 * \return A string containing the RGB codification of the given state number
 */
string
PlottingManager::RGBStateColor(INT32 StateValue)
{
  char RGBColor[8];

  if (StateValue < DEF_NB_COLOR_STATE)
  {

    if ((ParaverDefaultPalette[StateValue].RGB[0] == 0xFF) &&
        (ParaverDefaultPalette[StateValue].RGB[1] == 0xFF) &&
        (ParaverDefaultPalette[StateValue].RGB[2] == 0xFF))
    {
      sprintf(RGBColor, "#000000");
    }
    else
    {
      sprintf(RGBColor, "#%02x%02x%02x",
        ParaverDefaultPalette[StateValue].RGB[0],
        ParaverDefaultPalette[StateValue].RGB[1],
        ParaverDefaultPalette[StateValue].RGB[2]);
    }
  }
  else
  {
    sprintf(RGBColor, "#%02x%02x%02x",
      ParaverDefaultPalette[DEF_NB_COLOR_STATE-1].RGB[0],
      ParaverDefaultPalette[DEF_NB_COLOR_STATE-1].RGB[1],
      ParaverDefaultPalette[DEF_NB_COLOR_STATE-1].RGB[2]);
  }

  return RGBColor;
}
