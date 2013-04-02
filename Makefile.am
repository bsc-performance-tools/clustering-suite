## Process this file with automake to produce Makefile.in
## Created by Anjuta

INCLUDES = \
    -I$(top_srcdir)/include\
	-I$(top_srcdir)/common_utilities

##    -I@common_filesdir@

SUBDIRS = include common_utilities src scripts

instdir = $(includedir)

inst_HEADERS = \
	clustering_suite_config.h

EXTRA_DIST = \
  $(top_srcdir)/SVN_VERSION \
  $(clusteringsuitedoc_DATA) \
  $(top_srcdir)/include \
  $(top_srcdir)/substitute \
  $(top_srcdir)/substitute-all

# Remove doc directory on uninstall
uninstall-local:
	-rm -r $(clusteringsuitedocdir)
