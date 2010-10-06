## Process this file with automake to produce Makefile.in
## Created by Anjuta

INCLUDES = \
    -I$(top_srcdir)/include\
    -I$(top_srcdir)/../common-files

SUBDIRS = src 

EXTRA_DIST = $(clusteringsuitedoc_DATA) \
	$(top_srcdir)/include

# Remove doc directory on uninstall
uninstall-local:
	-rm -r $(clusteringsuitedocdir)
