lib_LTLIBRARIES = libBasicClasses.la

libBasicClasses_la_CPPFLAGS = $(top_srcdir)/include @CLUSTERING_CPPFLAGS@
libBasicClasses_la_LDFLAGS  = @CLUSTERING_LDFLAGS@ @CLUSTERING_CPPFLAGS@
libBasicClasses_la_LIBADD   = @CLUSTERING_LIBS@


instdir = $(includedir)

inst_HEADERS = \
	Error.hpp \
	FileNameManipulator.hpp \
	SystemMessages.hpp \
	Timer.hpp

libBasicClasses_la_SOURCES= \
	Error.cpp \
	Error.hpp \
	FileNameManipulator.cpp \
	FileNameManipulator.hpp \
	SystemMessages.h \
	SystemMessages.cpp \
	SystemMessages.hpp \
	Utilities.cpp \
	Utilities.hpp \
	Timer.hpp \
	Timer.cpp

