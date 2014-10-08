
CLUSTERING_CONFIG = clustering-config

EXTRA_DIST=${CLUSTERING_CONFIG}.in

bin_SCRIPTS = $(CLUSTERING_CONFIG)

$(CLUSTERING_CONFIG): $(CLUSTERING_CONFIG).in
	cp $^ $@
	$(top_srcdir)/substitute $(SED) "@sub_PREFIX@"          "${prefix}" $@
	$(top_srcdir)/substitute $(SED) "@sub_VERSION@"         "${VERSION}" $@
	$(top_srcdir)/substitute $(SED) "@sub_CPPFLAGS@"        "${CLUSTERING_CPPFLAGS}" $@
	$(top_srcdir)/substitute $(SED) "@sub_LDFLAGS@"         "${CLUSTERING_CLEAN_LDFLAGS}" $@
	$(top_srcdir)/substitute $(SED) "@sub_LIBTOOL_LDFLAGS@" "${CLUSTERING_LDFLAGS}" $@
	$(top_srcdir)/substitute $(SED) "@sub_LIBS@"            "${CLUSTERING_LIBS}" $@
	chmod u+x $@

clean-local:
	rm -f $(CLUSTERING_CONFIG)
