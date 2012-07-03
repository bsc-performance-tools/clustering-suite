lib_LTLIBRARIES = libBasicClasses.la

if HAVE_MPI
libBasicClasses_la_CXXFLAGS = @MPI_CFLAGS@
endif

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
	SystemMessage.h \
	SystemMessages.cpp \
	SystemMessages.hpp \
	Utilities.cpp \
	Utilities.hpp \
	Timer.hpp \
	Timer.cpp

