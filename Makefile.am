## Process this file with automake to produce Makefile.in
## Created by Anjuta

INCLUDES = \
    -I$(top_srcdir)/include\
    -I@common_filesdir@

SUBDIRS = include src 

instdir = $(includedir)/bsc_clustering_suite

inst_HEADERS = \
	config.h

EXTRA_DIST = $(clusteringsuitedoc_DATA) \
	$(top_srcdir)/include

# Remove doc directory on uninstall
uninstall-local:
	-rm -r $(clusteringsuitedocdir)
