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

#include "BurstsDB.hpp"

#include <ctime>
#include <cstdio>
#include <cstring>

#include <SystemMessages.hpp>
using cepba_tools::system_messages;

#include <sstream>
using std::ostringstream;

/*****************************************************************************
 * BurstDB class
 ****************************************************************************/

/**
 * Empty constructor
 */
BurstsDB::BurstsDB(void)
{
  DB       = NULL;
  DBActive = false;

  return;
}

/**
 * Parametrized constructor
 *
 * \param Parameters Parameters manager to define the fields that must be
 *                   stored in the DB
 */
BurstsDB::BurstsDB(ParametersManager* Parameters)
{
  ostringstream NameManipulator;
  time_t        TableCreationTime;

  DBFileName = "ClusteringSuiteDB.sqlite";

  char* SQLErrorMessage;

  //if (sqlite3_open(DBFileName.c_str(), &DB) != SQLITE_OK)
  // if the database is open with no name, SQLite creates just a temporal file
  if (sqlite3_open("", &DB) != SQLITE_OK)
  {
    //SetErrorMessage(string("unable to open temporal bursts database "+DBFileName).c_str(),
    //                sqlite3_errmsg(DB));
    SetErrorMessage("unable to open temporal bursts database",
                    sqlite3_errmsg(DB));
    SetError(true);

    sqlite3_close(DB);
  }

  /*
  if (unlink(DBFileName.c_str()) != 0)
  {
    SetError(true);
    SetErrorMessage(string("unable to detach database file "+DBFileName).c_str(),
                    string(strerror(errno)));
    return;
  }
  */

  SQLErrorMessage = NULL;
  sqlite3_exec(DB, "PRAGMA synchronous = OFF", NULL, NULL, &SQLErrorMessage);

  if (SQLErrorMessage != NULL)
  {
    SetErrorMessage(string("unable to tune database "+DBFileName).c_str(),
                    SQLErrorMessage);
    SetError(true);
    sqlite3_close(DB);
  }

  sqlite3_exec(DB, "PRAGMA journal_mode = MEMORY", NULL, NULL, &SQLErrorMessage);
  if (SQLErrorMessage != NULL)
  {
    SetErrorMessage(string("unable to tune database "+DBFileName).c_str(),
                    SQLErrorMessage);
    SetError(true);
    sqlite3_close(DB);
  }

  TableCreationTime = time(NULL);

  NameManipulator << "AllBursts_" << TableCreationTime;
  AllBurstsTable = NameManipulator.str();
  NameManipulator.str("");
  NameManipulator << "CompleteBursts_" << TableCreationTime;
  CompleteBurstsView = NameManipulator.str();

  if (!CreateDB(Parameters))
  {
    return;
  }

  DBActive         = true;
  NormalizedBursts = false;

  return;
}

/**
 * Class destructor
 */
BurstsDB::~BurstsDB(void)
{
  if (DBActive)
  {
    if (DB)
    {
      sqlite3_close(DB);
    }
  }
  return;
}

/**
 * Database close
 */
void BurstsDB::CloseDB(void)
{
  if (DBActive)
  {
    if (DB)
    {
      sqlite3_close(DB);
    }
    DBActive = false;
  }

  return;
}

/**
 * Inserts a CPU bursts in the database
 *
 * \param Burst The burst to be inserted
 *
 * \return true on success, false otherwise
 */
bool BurstsDB::NewBurst(CPUBurst* Burst)
{
  ostringstream       InsertQuery;
  ostringstream       ExtraParamsValues;

  map<size_t, double> ExtraParamsMap = Burst->GetExtrapolationDimensions();
  map<size_t, double>::iterator ExtraParameter;

  sqlite3_stmt *CompiledSQLQuery;

  if (!DBActive)
  {
    SetErrorMessage("trying to add a new burst without haven't started the database");
    SetError(true);
    return false;
  }

  /* Row insertion common fields */
  InsertQuery << "INSERT INTO " << AllBurstsTable << "( ";
  InsertQuery << "instance, type, taskid, threadid";
  InsertQuery << ", begin_time, end_time, duration, line";

  if (Burst->GetBurstType() != MissingDataBurst)
  {
    /* Raw and Processed Clustering Parameters are inserted in a collated
     * way */
    for (vector<string>::iterator Parameter  = ParamsNames.begin();
                                  Parameter != ParamsNames.end();
                                ++Parameter)
    {
      InsertQuery << ", " << (*Parameter) << ", " << (*Parameter) << "_Norm";
    }

    for (ExtraParameter  = ExtraParamsMap.begin();
         ExtraParameter != ExtraParamsMap.end();
       ++ExtraParameter)
    {
      /* Extrapolation parameter name added to the insertion fields detail */
      InsertQuery << ", " << "Extr_" << ExtraParamsNames[ExtraParameter->first];

      /* Extrapolation parameter value added to the value string */
      ExtraParamsValues << ", " << ExtraParameter->second;
    }
  }

  InsertQuery << ") VALUES (";

  InsertQuery.precision(12);

  /* "# Instance, Type, TaskId,ThreadId,Begin_Time,End_Time,Duration, Line" */
  InsertQuery << Burst->GetInstance();
  InsertQuery << ", " << Burst->GetBurstType(); // burst type!
  InsertQuery << ", " << Burst->GetTaskId();
  InsertQuery << ", " << Burst->GetThreadId();
  InsertQuery << ", " << Burst->GetBeginTime();
  InsertQuery << ", " << Burst->GetEndTime();
  InsertQuery << ", " << Burst->GetDuration();
  InsertQuery << ", " << Burst->GetLine();

  if (Burst->GetBurstType() != MissingDataBurst)
  {
    vector<double>& RawClusteringValues = Burst->GetRawDimensions();
    vector<double>& ProcessedClusteringValues = Burst->GetDimensions();

    for (size_t i = 0; i < RawClusteringValues.size(); i++)
    {
      InsertQuery << ", " << RawClusteringValues[i];
      InsertQuery << ", " << ProcessedClusteringValues[i];
    }

    InsertQuery << ExtraParamsValues.str();
  }
  InsertQuery << ")";

  /* DEBUG
  ostringstream Messages;
  Messages << "*** INSERTING NEW BURST IN THE DATABASE ";
  Messages << "(Type = " << BurstsDB::BurstTypeStr(Burst->GetBurstType()) << ")";
  Messages << " ***" << std::endl;
  Messages << "@@@" << InsertQuery.str() << "@@@" << std::endl;
  system_messages::information(Messages.str());
  */

  if (sqlite3_prepare_v2(DB,
                         InsertQuery.str().c_str(),
                         -1,
                         &CompiledSQLQuery,
                         NULL) != SQLITE_OK)
  {
    SetErrorMessage("database insert error",
                    sqlite3_errmsg(DB));
    SetError(true);
    return false;
  }

  if (sqlite3_step(CompiledSQLQuery) == SQLITE_ERROR)
  {
    SetErrorMessage("database insert error",
                    sqlite3_errmsg(DB));
    SetError(true);
    return false;
  }

  sqlite3_finalize(CompiledSQLQuery);

  AllBurstsInstances.push_back(Burst->GetInstance());

  if (Burst->GetBurstType() == CompleteBurst)
  {
    CompleteBurstsInstances.push_back(Burst->GetInstance());
  }

  return true;
}

/**
 * Applies the range normalization of the clustering dimensions for each
 * complete burst in the data base
 *
 * \param MaxValues maximum values of each clustering dimension
 * \param MinValues minimum values of each clustering dimension
 * \param Factors scaling factor to be applied on each dimension
 *
 * \return true on success, false otherwise
 */
bool BurstsDB::NormalizeBursts(const vector<double>& MaxValues,
                               const vector<double>& MinValues,
                               const vector<double>& Factors)
{
  ostringstream SQLQuery;
  vector<string>::iterator ParamName;
  vector<vector <string> > Values;

  sqlite3_stmt *CompiledSQLQuery;
  char* SQLErrorMessage;

  if (ParamsNames.size() != MaxValues.size() ||
      ParamsNames.size() != MinValues.size() ||
      ParamsNames.size() != Factors.size())
  {
    SetError(true);
    SetErrorMessage("number of max/min range values different from number of parameters");
    return false;
  }

  /* First, create an index on the table to speed-up the update/querys */
  SQLQuery << "CREATE INDEX " << AllBurstsTable << "_idx ";
  SQLQuery << "ON " << AllBurstsTable << " (instance)";

  SQLErrorMessage = NULL;
  sqlite3_exec(DB, SQLQuery.str().c_str(), NULL, NULL, &SQLErrorMessage);

  if (SQLErrorMessage != NULL)
  {
    SetError(true);
    SetErrorMessage("database index creation problem", SQLErrorMessage);
    return false;
  }

  /* The update must be done on the main table, not the view */
  SQLQuery.str("");
  SQLQuery << "UPDATE " << AllBurstsTable << " SET ";

  ParamName = ParamsNames.begin();
  vector<double>::size_type i = 0;
  while (ParamName != ParamsNames.end())
  {
    SQLQuery << (*ParamName) << "_Norm = " << "(";
    SQLQuery << "(" << (*ParamName)  << "_Norm - (" << MinValues[i] << ")) ";
    SQLQuery << "/ ";
    SQLQuery << "( (" << MaxValues[i] << ") - (" << MinValues[i] << "))";
    SQLQuery << ") * (" << Factors[i] << ")";

    ++ParamName;
    ++i;
    if (ParamName != ParamsNames.end())
    SQLQuery << ",";
  }
  SQLQuery << " WHERE type=" << CompleteBurst << ";";

  // system_messages::information("Executing update query: ");
  // system_messages::information(SQLQuery.str());

  if (sqlite3_prepare_v2(DB,
                         SQLQuery.str().c_str(),
                         -1,
                         &CompiledSQLQuery,
                         NULL) != SQLITE_OK)
  {
    SetErrorMessage("error normalizing bursts dimensions", sqlite3_errmsg(DB));
    SetError(true);
    return false;
  }

  if (sqlite3_step(CompiledSQLQuery) == SQLITE_ERROR)
  {
    SetErrorMessage("error normalizing bursts dimensions",
                    sqlite3_errmsg(DB));
    SetError(true);
    return false;
  }

  sqlite3_finalize(CompiledSQLQuery);

  return true;

  return true;
}

/**
 * Creates the actual table in the database where the bursts will be stored
 *
 * \param Parameters Parameters manager to define the fields that must be
 *                   stored in the DB
 * \param NormalizeParams Marks if clustering parameters are normalized
 *
 * \return true on success, false otherwise
 */
bool BurstsDB::CreateDB(ParametersManager* Parameters)
{
  ostringstream CreationQuery;
  ostringstream InsertPrefix;

  sqlite3_stmt* CompiledSQLQuery;

  ParamsNames      = Parameters->GetClusteringParametersNames();
  ExtraParamsNames = Parameters->GetExtrapolationParametersNames();

  /* "instance, type, taskid,threadid, begin_time,end_time,duration, line" */
  CreationQuery << "CREATE TABLE " << AllBurstsTable << " ";
  CreationQuery << "(";
  CreationQuery << "instance INTEGER PRIMARY KEY";
  CreationQuery << ", type INTEGER ";
  CreationQuery << ", taskid INTEGER ";
  CreationQuery << ", threadid INTEGER ";
  CreationQuery << ", begin_time INTEGER ";
  CreationQuery << ", end_time INTEGER ";
  CreationQuery << ", duration INTEGER ";
  CreationQuery << ", line INTEGER ";

  /* Clustering Parameters */
  for (vector<string>::size_type  i = 0; i < ParamsNames.size(); i++)
  {
    CreationQuery << ", " << ParamsNames[i];
    // Clustering parameters are always stored as 'double' values
    CreationQuery << " REAL";

  }

  /* Clustering Parameters Normalized */
  for (vector<string>::size_type  i = 0; i < ParamsNames.size(); i++)
  {
    CreationQuery << ", " << ParamsNames[i] << "_Norm";
    CreationQuery << " REAL";
  }


  /* Extrapolation Parameters */
  for (vector<string>::size_type i = 0; i < ExtraParamsNames.size(); i++)
  {
    CreationQuery << ", " << "Extr_" << ExtraParamsNames[i];
    // Extrapolation dimensions are always 'double' values
    CreationQuery << " REAL";

  }
  CreationQuery << ");";

  TableCreationQuery = CreationQuery.str();

  if (sqlite3_prepare_v2(DB,
                         TableCreationQuery.c_str(),
                         -1,
                         &CompiledSQLQuery,
                         NULL) != SQLITE_OK)
  {
    SetErrorMessage("error creating bursts table", sqlite3_errmsg(DB));
    SetError(true);
    return false;
  }

  if (sqlite3_step(CompiledSQLQuery) == SQLITE_ERROR)
  {
    SetErrorMessage("error creating bursts table",
                    sqlite3_errmsg(DB));
    SetError(true);
    return false;
  }

  // sqlite3_finalize(CompiledSQLQuery);

  CreationQuery.str("");
  CreationQuery << "CREATE VIEW " << CompleteBurstsView << " AS ";
  CreationQuery << "SELECT * FROM " << AllBurstsTable << " ";
  CreationQuery << "WHERE " << AllBurstsTable << ".type=\"COMPLETE\"";

  if (sqlite3_prepare_v2(DB,
                         CreationQuery.str().c_str(),
                         -1,
                         &CompiledSQLQuery,
                         NULL) != SQLITE_OK)
  {
    SetErrorMessage("error creating bursts helper table", sqlite3_errmsg(DB));
    SetError(true);
    return false;
  }

  if (sqlite3_step(CompiledSQLQuery) == SQLITE_ERROR)
  {
    SetErrorMessage("error creating bursts helper table",
                    sqlite3_errmsg(DB));
    SetError(true);
    return false;
  }

  sqlite3_finalize(CompiledSQLQuery);

  return true;
}

/**
 * Returns a the CPU burst with the instance number selected
 *
 * \param instance iterator with the instance value to retrieve
 *
 * \return if the 'instance' is contained in the database, a CPUBurst object
 *         pointer containing the stored data, null otherwise
 */
CPUBurst* BurstsDB::GetBurst(vector<instance_t>::iterator instance)
{
  ostringstream SQLQuery;
  sqlite3_stmt *CompiledSQLQuery;

  vector<string>::iterator ParamName;

  /* Burst information */
  CPUBurst*            Burst = NULL;
  instance_t           Instance;
  burst_type_t         BurstType;
  task_id_t            TaskId;
  thread_id_t          ThreadId;
  timestamp_t          BeginTime;
  timestamp_t          EndTime;
  duration_t           Duration;
  line_t               Line;
  vector<double>       ClusteringRawData;
  vector<double>       ClusteringProcessedData;
  map<size_t, double>  ExtrapolationData;
  bool                 ToClassify;

  size_t QueryParameters = 0;

  if (instance == AllBurstsInstances.end() ||
      instance == CompleteBurstsInstances.end())
  {
    return NULL;
  }

  SQLQuery << "SELECT instance, type, taskid, threadid, begin_time, end_time";
  SQLQuery << ", duration, line";

  QueryParameters += 8;

  for (ParamName  = ParamsNames.begin();
       ParamName != ParamsNames.end();
     ++ParamName)
  {
    SQLQuery << ", " << (*ParamName);
    QueryParameters++;
  }

  for (ParamName  = ParamsNames.begin();
       ParamName != ParamsNames.end();
     ++ParamName)
  {
    SQLQuery << ", " << (*ParamName) << "_Norm";
    QueryParameters++;
  }

  for (ParamName  = ExtraParamsNames.begin();
       ParamName != ExtraParamsNames.end();
     ++ParamName)
  {
    SQLQuery << ", " << "Extr_" << (*ParamName);
    QueryParameters++;
  }

  SQLQuery << " FROM " << AllBurstsTable;
  SQLQuery << " WHERE instance=" << (*instance) << ";";

  if (sqlite3_prepare_v2(DB,
                         SQLQuery.str().c_str(),
                         -1,
                         &CompiledSQLQuery,
                         NULL) != SQLITE_OK)
  {
    SetErrorMessage("error retrieving burst from the database", sqlite3_errmsg(DB));
    SetError(true);
    return NULL;
  }

  /* Retrieve the information and create the CPU burst object */
  if (sqlite3_step(CompiledSQLQuery) == SQLITE_ERROR)
  {
    sqlite3_finalize(CompiledSQLQuery);

    SetErrorMessage("error retrieving burst from the database",
                    sqlite3_errmsg(DB));
    SetError(true);
    return NULL;
  }

  int TotalColumns = sqlite3_column_count(CompiledSQLQuery);
  int Column       = 0;

  /* Check if the number of fields retrieved is the expected */
  if (TotalColumns != (8 + ParamsNames.size() * 2 + ExtraParamsNames.size()))
  {
    ostringstream Message;
    int Expected = (8 + ParamsNames.size() * 2 + ExtraParamsNames.size());

    Message << "Error retriving burst data: expected " << Expected << " fields,";
    Message << " obtained " << TotalColumns << " fields";

    sqlite3_finalize(CompiledSQLQuery);

    SetError(true);
    SetErrorMessage(Message.str());
    return NULL;
  }

  /* Fixed values per burst
   * instance, type, taskid, threadid, begin_time, end_time, duration, line"
   */
  Instance  = (instance_t)   sqlite3_column_int64(CompiledSQLQuery, Column++);
  BurstType = (burst_type_t) sqlite3_column_int(CompiledSQLQuery, Column++);
  TaskId    = (task_id_t)    sqlite3_column_int64(CompiledSQLQuery, Column++);
  ThreadId  = (thread_id_t)  sqlite3_column_int64(CompiledSQLQuery, Column++);
  BeginTime = (timestamp_t)  sqlite3_column_int64(CompiledSQLQuery, Column++);
  EndTime   = (timestamp_t)  sqlite3_column_int64(CompiledSQLQuery, Column++);
  Duration  = (duration_t)   sqlite3_column_int64(CompiledSQLQuery, Column++);
  Line      = (line_t)       sqlite3_column_int64(CompiledSQLQuery, Column++);

  /* Clustering dimensions */
  for(int i = 0; i < ParamsNames.size(); i++, Column++)
  {
    if (sqlite3_column_type(CompiledSQLQuery, Column) != SQLITE_NULL)
    {
      ClusteringRawData.push_back(sqlite3_column_double(CompiledSQLQuery,
                                                        Column));
    }
  }

  /* Normalized clustering dimension */
  for(int i = 0; i < ParamsNames.size(); i++, Column++)
  {
    if (sqlite3_column_type(CompiledSQLQuery, Column) != SQLITE_NULL)
    {
      ClusteringProcessedData.push_back(sqlite3_column_double(CompiledSQLQuery,
                                                              Column));
    }
  }

  /* Extrapolation dimensions */
  for (int i = 0; i < ExtraParamsNames.size(); i++, Column++)
  {
    if (sqlite3_column_type(CompiledSQLQuery, Column) != SQLITE_NULL)
    {
      ExtrapolationData[(size_t)i] = sqlite3_column_double(CompiledSQLQuery, Column);
    }
  }

  sqlite3_finalize(CompiledSQLQuery);

  if (BurstType != MissingDataBurst &&
      (ClusteringRawData.size() == 0 || ClusteringProcessedData.size() == 0))
  {
    ostringstream Message;

    Message << "wrong number of fields in burst #" << Instance;

    SetError(true);
    SetErrorMessage(Message.str());
    return NULL;
  }

  Burst = new CPUBurst(Instance,
                       TaskId,
                       ThreadId,
                       Line,
                       BeginTime,
                       EndTime,
                       Duration,
                       ClusteringRawData,
                       ClusteringProcessedData,
                       ExtrapolationData,
                       BurstType);

  return Burst;
}

/**
 * Returns the textual string of the different bursts types
 *
 * \param typeVal the type to be transformed into a string
 *
 * \return the string containing the text transformation of the internal type
 */
string BurstsDB::BurstTypeStr(burst_type_t typeVal)
{
  switch(typeVal)
  {
    case CompleteBurst:
      return string("COMPLETE");
    case MissingDataBurst:
      return string("MISSING_DATA");
    case DurationFilteredBurst:
      return string("DURATION_FILTERED");
    case  RangeFilteredBurst:
      return string("RANGE_FILTERED");
  }
}

/**
 * Returns the internal burst type from a DB string
 *
 * \param typeStr the string that contains the burst type in the DB
 *
 * \return the internal burst type equivalent
 */
burst_type_t BurstsDB::BurstTypeVal(string typeStr)
{
  if (typeStr.compare("COMPLETE") == 0)
    return CompleteBurst;
  else if (typeStr.compare("MISSING_DATA") == 0)
    return MissingDataBurst;
  else if (typeStr.compare("DURATION_FILTERED") == 0)
    return DurationFilteredBurst;
  else if (typeStr.compare("RANGE_FILTERED") == 0)
    return RangeFilteredBurst;
}

