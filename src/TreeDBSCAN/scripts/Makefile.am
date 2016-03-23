SOURCEME_SH     = TDBSCAN_env.sh
FE_WRAPPER      = TDBSCAN_FE 
BE_WRAPPER      = TDBSCAN_BE
SWEEP           = tdbscan-sweep
DRAW_HULLS      = draw_hulls
DRAW_STATS      = draw_stats
EXPAND_NODELIST = sl_expand_nodelist.pl

EXTRA_DIST  = $(SOURCEME_SH) tdbscan-launcher tdbscan-launcher.py $(SWEEP) $(SWEEP).py $(FE_WRAPPER) $(BE_WRAPPER) $(DRAW_HULLS) $(DRAW_STATS) $(EXPAND_NODELIST) tdbscan-mass-submit tdbscan-mass-submit.py tdbscan-process-results tdbscan-process-results.py concat-csv.py concat-csv

if HAVE_MPI
EXTRA_DIST += tdbscan-launcher-mpi TDBSCAN_FE_mpi TDBSCAN_BE_mpi
endif

bin_SCRIPTS  = tdbscan-launcher tdbscan-launcher.py $(SWEEP) $(SWEEP).py $(FE_WRAPPER) $(BE_WRAPPER) $(DRAW_HULLS) $(DRAW_STATS) $(EXPAND_NODELIST) tdbscan-mass-submit tdbscan-mass-submit.py tdbscan-process-results tdbscan-process-results.py concat-csv.py concat-csv

if HAVE_MPI
bin_SCRIPTS += tdbscan-launcher-mpi TDBSCAN_FE_mpi TDBSCAN_BE_mpi
endif

noinst_SCRIPTS = $(SOURCEME_SH)

install-data-hook:
	$(mkdir_p) $(prefix)/etc
	cp $(SOURCEME_SH) $(prefix)/etc
	$(top_srcdir)/substitute-all $(SED) "@sub_PREFIX@" ${prefix} $(prefix)/etc
	$(top_srcdir)/substitute-all $(SED) "@sub_SYNAPSE_HOME@" @SYNAPSE_HOME@ $(prefix)/etc
	$(top_srcdir)/substitute-all $(SED) "@sub_SYNAPSE_LIBSDIR@" @SYNAPSE_LIBSDIR@ $(prefix)/etc
	$(top_srcdir)/substitute-all $(SED) "@sub_CLUSTERING_LD_LIBRARY_PATH@" @CLUSTERING_LD_LIBRARY_PATH@ $(prefix)/etc
	$(top_srcdir)/substitute     $(SED) "@sub_PREFIX@" ${prefix} $(bindir)/$(FE_WRAPPER)
	$(top_srcdir)/substitute     $(SED) "@sub_PREFIX@" ${prefix} $(bindir)/$(BE_WRAPPER)
	$(top_srcdir)/substitute     $(SED) "@sub_PREFIX@" ${prefix} $(bindir)/tdbscan-launcher
	$(top_srcdir)/substitute     $(SED) "@sub_PREFIX@" ${prefix} $(bindir)/tdbscan-launcher.py
	$(top_srcdir)/substitute     $(SED) "@sub_SYNAPSE_HOME@" @SYNAPSE_HOME@ $(bindir)/tdbscan-launcher.py
	$(top_srcdir)/substitute     $(SED) "@sub_PREFIX@" ${prefix} $(bindir)/$(SWEEP)
	$(top_srcdir)/substitute     $(SED) "@sub_PREFIX@" ${prefix} $(bindir)/$(SWEEP).py
	$(top_srcdir)/substitute     $(SED) "@sub_PREFIX@" ${prefix} $(bindir)/tdbscan-mass-submit
	$(top_srcdir)/substitute     $(SED) "@sub_PREFIX@" ${prefix} $(bindir)/tdbscan-process-results
	$(top_srcdir)/substitute     $(SED) "@sub_PREFIX@" ${prefix} $(bindir)/tdbscan-process-results.py
	$(top_srcdir)/substitute     $(SED) "@sub_PREFIX@" ${prefix} $(bindir)/concat-csv
	chmod a+x $(bindir)/tdbscan-launcher $(bindir)/$(SWEEP)
	chmod a+x $(bindir)/$(FE_WRAPPER) $(bindir)/$(BE_WRAPPER)
	chmod a+x $(bindir)/tdbscan-mass-submit $(bindir)/tdbscan-process-results
	chmod a+x $(bindir)/concat-csv
	ln -fs `basename $(bindir)/$(FE_WRAPPER)` $(bindir)/TDBSCAN
if HAVE_MPI
	$(top_srcdir)/substitute     $(SED) "@sub_PREFIX@" ${prefix} $(bindir)/tdbscan-launcher-mpi
	$(top_srcdir)/substitute     $(SED) "@sub_PREFIX@" ${prefix} $(bindir)/TDBSCAN_FE_mpi
	$(top_srcdir)/substitute     $(SED) "@sub_PREFIX@" ${prefix} $(bindir)/TDBSCAN_BE_mpi
	chmod a+x $(bindir)/tdbscan-launcher-mpi $(bindir)/$(SWEEP)
	chmod a+x $(bindir)/TDBSCAN_FE_mpi $(bindir)/TDBSCAN_BE_mpi
endif

