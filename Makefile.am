## Process this file with automake to produce Makefile.in
## Created by Anjuta

AM_CPPFLAGS =\
 $(AM_CPPFLAGS) \
 -I$(top_srcdir)/include\
 -I$(top_srcdir)/common_utilities

##    -I@common_filesdir@

## The 'pcfparser_svn1816' and 'common_utilities' directory will
## be merged in the new 'common-files' package
SUBDIRS = include pcfparser_svn1816 common_utilities src scripts doc

instdir = $(includedir)

inst_HEADERS = \
	clustering_suite_config.h

EXTRA_DIST = \
  $(top_srcdir)/SVN_VERSION \
  $(top_srcdir)/example \
  $(top_srcdir)/include \
  $(top_srcdir)/substitute \
  $(top_srcdir)/substitute-all

# Remove doc directory on uninstall
uninstall-local:
	-rm -r $(clusteringsuitedocdir)

# Install the example
EXAMPLE_FILES = \
 example/cluster_explained.xml\
 example/cluster.xml\
 example/su3.lat256_2it.16tasks.pcf\
 example/su3.lat256_2it.16tasks.prv\
 example/su3.lat256_2it.16tasks.row

DOC_FILES = \
 doc/clusteringsuite_manual.pdf
 
install-data-hook: $(EXAMPLE_FILES) $(DOC_FILES)
	$(MKDIR_P) $(DESTDIR)$(datadir)/example
	cp -f $(EXAMPLE_FILES) $(DESTDIR)$(datadir)/example
	$(MKDIR_P) $(DESTDIR)$(datadir)/doc
	cp -f $(DOC_FILES) $(DESTDIR)$(datadir)/doc
