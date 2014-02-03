## Process this file with automake to produce Makefile.in
## Created by Anjuta

AM_CPPFLAGS = $(AM_CPPFLAGS) \
    -I$(top_srcdir)/include\
	-I$(top_srcdir)/common_utilities

##    -I@common_filesdir@

## The 'pcfparser_svn1816' and 'common_utilities' directory will
## be merged in the new 'common-files' package
SUBDIRS = include pcfparser_svn1816 common_utilities src scripts

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
