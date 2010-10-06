common_filesdir = @common_filesdir@
INCLUDES = \
    -I$(top_srcdir)/include \
	-I$(top_srcdir)/src/BasicClasses \
	-I$(top_srcdir)/src/libClustering \
	-I$(top_srcdir)/src/libTraceClustering \
	-I$(common_filesdir)

noinst_LTLIBRARIES = libClusteringUtils.la

libClusteringUtils_la_CFLAGS = @XML2_CFLAGS@

libClusteringUtils_la_CXXFLAGS = @XML2_CFLAGS@

#libClusteringUtils_la_LIBADD = \
#	$(top_srcdir)/src/BasicClasses/libBasicClasses.la

libClusteringUtils_la_SOURCES = \
  ClusteringConfiguration.cpp \
  ClusteringConfiguration.hpp \
  ClusteringParameter.cpp \
  ClusteringParameter.hpp \
  CPIStackModel.hpp \
  FileNameManipulator.cpp \
  FileNameManipulator.hpp \
  ParametersManager.cpp \
  ParametersManager.hpp \
  PlottingManager.cpp \
  PlottingManager.hpp \
  State.cpp \
  State.hpp \
  XMLParser.cpp \
  XMLParser.hpp 

