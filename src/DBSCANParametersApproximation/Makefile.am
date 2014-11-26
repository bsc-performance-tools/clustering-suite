###############################################################################
# 'DataExtractor' binary
###############################################################################

bin_PROGRAMS = DBSCANParametersApproximation.bin

DBSCANParametersApproximation_bin_SOURCES=\
 DBSCANParametersApproximationMain.cpp 

DBSCANParametersApproximation_bin_CPPFLAGS=\
 @CLUSTERING_CPPFLAGS@\
 -I$(top_srcdir)/src/libClustering\
 -I$(top_srcdir)/src/libTraceClustering\
 -I$(top_srcdir)/src/libSharedComponents
 
DBSCANParametersApproximation_bin_CFLAGS   = ${CFLAGS}
DBSCANParametersApproximation_bin_LDFLAGS  = @CLUSTERING_LDFLAGS@
DBSCANParametersApproximation_bin_LDADD=\
 $(top_srcdir)/src/libTraceClustering/libTraceClustering.la\
 $(top_srcdir)/src/BasicClasses/libBasicClasses.la\
 @CLUSTERING_LIBS@

EXTRA_DIST=DBSCANParametersApproximation
bin_SCRIPTS=DBSCANParametersApproximation
