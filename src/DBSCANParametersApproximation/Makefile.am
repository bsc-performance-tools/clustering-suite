AM_CPPFLAGS = \
	-I$(top_srcdir)/include \
	-I$(top_srcdir)/src/BasicClasses \
	-I$(top_srcdir)/src/libTraceClustering

###############################################################################
# 'DataExtractor' binary
###############################################################################

bin_PROGRAMS = DBSCANParametersApproximation

DBSCANParametersApproximation_SOURCES=\
 DBSCANParametersApproximationMain.cpp 

DBSCANParametersApproximation_CPPFLAGS=\
 @CLUSTERING_CPPFLAGS@\
 -I$(top_srcdir)/src/libClustering\
 -I$(top_srcdir)/src/libTraceClustering\
 -I$(top_srcdir)/src/libSharedComponents
 
DBSCANParametersApproximation_CFLAGS   = ${CFLAGS}
DBSCANParametersApproximation_LDFLAGS  = @CLUSTERING_LDFLAGS@
DBSCANParametersApproximation_LDADD=\
 $(top_srcdir)/src/libTraceClustering/libTraceClustering.la\
 $(top_srcdir)/src/BasicClasses/libBasicClasses.la\
 @CLUSTERING_LIBS@


