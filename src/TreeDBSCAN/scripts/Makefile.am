SOURCEME_SH     = TDBSCAN_env.sh
FE_WRAPPER      = TDBSCAN_FE 
BE_WRAPPER      = TDBSCAN_BE
LAUNCHER        = tdbscan-launcher
SWEEP           = tdbscan-sweep
DRAW_HULLS      = draw_hulls
DRAW_STATS      = draw_stats
EXPAND_NODELIST = sl_expand_nodelist.pl

EXTRA_DIST = $(SOURCEME_SH) $(LAUNCHER) $(LAUNCHER).py $(SWEEP) $(SWEEP).py $(FE_WRAPPER) $(BE_WRAPPER) $(DRAW_HULLS) $(DRAW_STATS) $(EXPAND_NODELIST) tdbscan-mass-submit tdbscan-mass-submit.py tdbscan-process-results tdbscan-process-results.py concat-csv.py concat-csv

bin_SCRIPTS    = $(LAUNCHER) $(LAUNCHER).py $(SWEEP) $(SWEEP).py $(FE_WRAPPER) $(BE_WRAPPER) $(DRAW_HULLS) $(DRAW_STATS) $(EXPAND_NODELIST) tdbscan-mass-submit tdbscan-mass-submit.py tdbscan-process-results tdbscan-process-results.py concat-csv.py concat-csv
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
	$(top_srcdir)/substitute     $(SED) "@sub_PREFIX@" ${prefix} $(bindir)/$(LAUNCHER)
	$(top_srcdir)/substitute     $(SED) "@sub_PREFIX@" ${prefix} $(bindir)/$(LAUNCHER).py
	$(top_srcdir)/substitute     $(SED) "@sub_MRNETAPP_HOME@" @MRNETAPP_HOME@ $(bindir)/$(LAUNCHER).py
	$(top_srcdir)/substitute     $(SED) "@sub_PREFIX@" ${prefix} $(bindir)/$(SWEEP)
	$(top_srcdir)/substitute     $(SED) "@sub_PREFIX@" ${prefix} $(bindir)/$(SWEEP).py
	$(top_srcdir)/substitute     $(SED) "@sub_PREFIX@" ${prefix} $(bindir)/tdbscan-mass-submit
	$(top_srcdir)/substitute     $(SED) "@sub_PREFIX@" ${prefix} $(bindir)/tdbscan-process-results
	$(top_srcdir)/substitute     $(SED) "@sub_PREFIX@" ${prefix} $(bindir)/tdbscan-process-results.py
	$(top_srcdir)/substitute     $(SED) "@sub_PREFIX@" ${prefix} $(bindir)/concat-csv
	chmod a+x $(bindir)/$(LAUNCHER) $(bindir)/$(SWEEP)
	chmod a+x $(bindir)/$(FE_WRAPPER) $(bindir)/$(BE_WRAPPER)
	chmod a+x $(bindir)/tdbscan-mass-submit $(bindir)/tdbscan-process-results
	chmod a+x $(bindir)/concat-csv
	ln -fs `basename $(bindir)/$(FE_WRAPPER)` $(bindir)/TDBSCAN
