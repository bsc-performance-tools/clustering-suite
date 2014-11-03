SOURCEME_SH     = TDBSCAN_env.sh
FE_WRAPPER      = TDBSCAN_FE 
BE_WRAPPER      = TDBSCAN_BE
DRAW_HULLS      = draw_hulls
DRAW_STATS      = draw_stats
EXPAND_NODELIST = sl_expand_nodelist.pl

EXTRA_DIST = $(SOURCEME_SH) $(FE_WRAPPER) $(BE_WRAPPER) $(DRAW_HULLS) $(DRAW_STATS) $(EXPAND_NODELIST)

bin_SCRIPTS    = $(FE_WRAPPER) $(BE_WRAPPER) $(DRAW_HULLS) $(DRAW_STATS) $(EXPAND_NODELIST)
noinst_SCRIPTS = $(SOURCEME_SH)

install-data-hook:
	$(mkdir_p) $(prefix)/etc
	cp $(SOURCEME_SH) $(prefix)/etc
	$(top_srcdir)/substitute-all $(SED) "@sub_PREFIX@" ${prefix} $(prefix)/etc
	$(top_srcdir)/substitute-all $(SED) "@sub_MRNETAPP_HOME@" @MRNETAPP_HOME@ $(prefix)/etc
	$(top_srcdir)/substitute-all $(SED) "@sub_MRNETAPP_LIBSDIR@" @MRNETAPP_LIBSDIR@ $(prefix)/etc
	$(top_srcdir)/substitute-all $(SED) "@sub_CLUSTERING_LD_LIBRARY_PATH@" @CLUSTERING_LD_LIBRARY_PATH@ $(prefix)/etc
	$(top_srcdir)/substitute     $(SED) "@sub_PREFIX@" ${prefix} $(bindir)/$(FE_WRAPPER)
	$(top_srcdir)/substitute     $(SED) "@sub_PREFIX@" ${prefix} $(bindir)/$(BE_WRAPPER)
	chmod a+x $(bindir)/$(FE_WRAPPER) $(bindir)/$(BE_WRAPPER)
	ln -fs $(bindir)/$(FE_WRAPPER) $(bindir)/TDBSCAN
