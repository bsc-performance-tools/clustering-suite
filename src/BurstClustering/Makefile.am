bin_PROGRAMS = \
	BurstClustering.bin

BurstClustering_bin_SOURCES = \
	BurstClustering.cpp

BurstClustering_bin_CPPFLAGS = \
 @CLUSTERING_CPPFLAGS@\
 -I$(top_srcdir)/src/libClustering\
 -I$(top_srcdir)/src/libTraceClustering\
 -I$(top_srcdir)/src/libSharedComponents

BurstClustering_bin_LDFLAGS  = @CLUSTERING_LDFLAGS@
BurstClustering_bin_LDADD = \
	$(top_builddir)/src/libTraceClustering/libTraceClustering.la \
	$(top_builddir)/src/BasicClasses/libBasicClasses.la \
	@CLUSTERING_LIBS@

bin_SCRIPTS = BurstClustering.sh
EXTRA_DIST  = BurstClustering.sh

install-data-hook:
	mv $(DESTDIR)${bindir}/BurstClustering.sh $(DESTDIR)${bindir}/BurstClustering
