
SOURCEME_SH = Tree_DBSCAN_env.sh
FE_WRAPPER  = Tree_DBSCAN_FE 
BE_WRAPPER  = Tree_DBSCAN_BE
DRAW_HULLS  = draw_hulls
DRAW_STATS  = draw_stats

EXTRA_DIST = $(SOURCEME_SH) $(FE_WRAPPER) $(BE_WRAPPER) $(DRAW_HULLS) $(DRAW_STATS)

bin_SCRIPTS    = $(FE_WRAPPER) $(BE_WRAPPER) $(DRAW_HULLS) $(DRAW_STATS)
noinst_SCRIPTS = $(SOURCEME_SH)

install-data-hook:
	$(mkdir_p) $(prefix)/etc
	cp $(SOURCEME_SH) $(prefix)/etc
	$(top_srcdir)/substitute-all $(SED) "@sub_PREFIX@" ${prefix} $(prefix)/etc
	$(top_srcdir)/substitute-all $(SED) "@sub_MRNETAPP_HOME@" @MRNETAPP_HOME@ $(prefix)/etc
	$(top_srcdir)/substitute-all $(SED) "@sub_MRNETAPP_LIBSDIR@" @MRNETAPP_LIBSDIR@ $(prefix)/etc
	$(top_srcdir)/substitute-all $(SED) "@sub_CLUSTERING_DYNAMIC_PATH@" @CLUSTERING_DYNAMIC_PATH@ $(prefix)/etc
	$(top_srcdir)/substitute-all $(SED) "@sub_CGAL_DYNAMIC_PATH@" @CGAL_DYNAMIC_PATH@ $(prefix)/etc
	$(top_srcdir)/substitute     $(SED) "@sub_PREFIX@" ${prefix} $(bindir)/$(FE_WRAPPER)
	$(top_srcdir)/substitute     $(SED) "@sub_PREFIX@" ${prefix} $(bindir)/$(BE_WRAPPER)
	chmod a+x $(bindir)/$(FE_WRAPPER) $(bindir)/$(BE_WRAPPER)
