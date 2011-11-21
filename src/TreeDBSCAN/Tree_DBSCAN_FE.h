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
