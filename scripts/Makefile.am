
CLUSTERING_CONFIG = clustering-config

EXTRA_DIST=${CLUSTERING_CONFIG}.in

bin_SCRIPTS = $(CLUSTERING_CONFIG)

$(CLUSTERING_CONFIG): $(CLUSTERING_CONFIG).in
	cp $^ $@
	$(top_srcdir)/substitute $(SED) "@sub_PREFIX@" "${prefix}" $@
	$(top_srcdir)/substitute $(SED) "@sub_VERSION@" "${VERSION}" $@
	$(top_srcdir)/substitute $(SED) "@sub_CGAL_CPPFLAGS@" "${CGAL_CPPFLAGS}" $@
	$(top_srcdir)/substitute $(SED) "@sub_CGAL_LDFLAGS@" "${CGAL_LDFLAGS}" $@
if HAVE_MPI
	$(top_srcdir)/substitute $(SED) "@sub_MPI_CFLAGS@" "-I@MPI_INCLUDES@" $@
	$(top_srcdir)/substitute $(SED) "@sub_MPI_LDFLAGS@" "@MPI_LDFLAGS@ @MPI_LIBS@" $@
else
	$(top_srcdir)/substitute $(SED) "@sub_MPI_CFLAGS@" "" $@
	$(top_srcdir)/substitute $(SED) "@sub_MPI_LDFLAGS@" "" $@
endif
	chmod u+x $@

clean-local:
	rm -f $(CLUSTERING_CONFIG)
