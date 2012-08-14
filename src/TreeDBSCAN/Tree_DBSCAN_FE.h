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

#ifndef __TREE_DBSCAN_FE_H__
#define __TREE_DBSCAN_FE_H__

#define HELP                                                                        \
   "\n"                                                                             \
   "Usage:\n"                                                                       \
   "  %s [-s] -d <clustering_def.xml> -i <input_trace> -o <output_trace>\n"         \
   "\n"                                                                             \
   "  -v|--version               Information about the tool\n"                      \
   "\n"                                                                             \
   "  -h                         This help\n"                                       \
   "\n"                                                                             \
   "  -s                         Do not show information messages (silent mode)\n"  \
   "\n"                                                                             \
   "  -r                         Reconstruct the input trace adding the cluster\n"  \
   "                             information obtained\n"                            \
   "\n"                                                                             \
   "  -d <clustering_def_xml>    XML containing the clustering process\n"           \
   "                             definition\n"                                      \
   "\n"                                                                             \
   "  -i <input_file>            Input CSV / Dimemas trace / Paraver trace\n"       \
   "\n"                                                                             \
   "  -o <output_file>           Output CSV file / Dimemas trace / Paraver trace\n" \
   "                             clustering process or output data file if\n"       \
   "                             parameters '-x' or '-r' are used\n"                \
   "\n"                                                                             \
   "  -e <epsilon>               Specify the Epsilon for the density clustering\n"  \
   "\n"                                                                             \
   "  -m <min_points>            Specify the minimum points to form a cluster\n"    \
   "\n"


#define ABOUT                                            \
   "%s v%s (%s)\n"                                       \
   "(c) CEPBA-Tools - Barcelona Supercomputing Center\n" \
   "Automatic clustering analysis of Paraver/Dimemas traces and CSV files\n"


void ReadArgs  (int argc, char *argv[]);
void PrintUsage(char* ApplicationName);

#endif /* __TREE_DBSCAN_FE_H__ */
