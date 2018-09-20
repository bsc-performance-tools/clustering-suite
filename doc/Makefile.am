
EXTRA_DIST = \
  ./clusteringsuite_manual.pdf \
  ./clusteringsuite_manual.tex \
  ./clustering_manual_structure.txt \
  ./clusteringsuite_design.tex \
  ./clusteringsuite_bsctools.pdf \
  ./clusteringsuite_design.pdf \
  ./figure_sources/uml_design/libclustering_uml.pdf \
  ./figure_sources/uml_design/libtraceclustering_uml.pdf \
  ./figure_sources/uml_design/libClustering_uml.graffle \
  ./figure_sources/uml_design/libTraceClustering_uml.graffle \
  ./figure_sources/plots_example/GAPGEOFEM.16.mref.ALL_HWC.TIME_SPACE_SHIFTING.chop1.LD_vs_ST.gnuplot \
  ./figure_sources/plots_example/int_vs_fop_plot.pdf \
  ./figure_sources/plots_example/inst_vs_ipc_plot.pdf \
  ./figure_sources/plots_example/GAPGEOFEM.16.mref.ALL_HWC.TIME_SPACE_SHIFTING.chop1.L2DATA_vs_MainMEMData.gnuplot \
  ./figure_sources/plots_example/mainmem_vs_l2_plot.pdf \
  ./figure_sources/plots_example/GAPGEOFEM.16.mref.ALL_HWC.TIME_SPACE_SHIFTING.chop1.CLUSTERED.FULL_POINTS.Cluster3.csv \
  ./figure_sources/plots_example/GAPGEOFEM.16.mref.ALL_HWC.TIME_SPACE_SHIFTING.chop1.L2DATA_vs_MainMEMData.PNG.gnuplot \
  ./figure_sources/plots_example/GAPGEOFEM.16.mref.ALL_HWC.TIME_SPACE_SHIFTING.chop1.CLUSTERED.FULL_POINTS.Cluster1.csv \
  ./figure_sources/plots_example/GAPGEOFEM.16.mref.ALL_HWC.TIME_SPACE_SHIFTING.chop1.FPU_vs_FXU.PNG.gnuplot \
  ./figure_sources/plots_example/int_vs_fop_plot.png \
  ./figure_sources/plots_example/GAPGEOFEM.16.mref.ALL_HWC.TIME_SPACE_SHIFTING.chop1.CLUSTERED.FULL_POINTS.Cluster4.csv \
  ./figure_sources/plots_example/GAPGEOFEM.16.mref.ALL_HWC.TIME_SPACE_SHIFTING.chop1.FPU_vs_FXU.gnuplot \
  ./figure_sources/plots_example/inst_vs_ipc_plot.png \
  ./figure_sources/plots_example/GAPGEOFEM.16.mref.ALL_HWC.TIME_SPACE_SHIFTING.chop1.LD_vs_ST.PNG.gnuplot \
  ./figure_sources/plots_example/GAPGEOFEM.16.mref.ALL_HWC.TIME_SPACE_SHIFTING.chop1.INST_vs_IPC.gnuplot \
  ./figure_sources/plots_example/stores_vs_loads_plot.pdf \
  ./figure_sources/plots_example/GAPGEOFEM.16.mref.ALL_HWC.TIME_SPACE_SHIFTING.chop1.INST_vs_IPC.PNG.gnuplot \
  ./figure_sources/plots_example/GAPGEOFEM.16.mref.ALL_HWC.TIME_SPACE_SHIFTING.chop1.CLUSTERED.FULL_POINTS.Cluster2.csv \
  ./figure_sources/plots_example/stores_vs_loads_plot.png \
  ./figure_sources/plots_example/mainmem_vs_l2_plot.png \
  ./figure_sources/stats_plot/GADGET-DEISA.MEDIUM.256.1HWCG.chop1.bursts.dat \
  ./figure_sources/stats_plot/GADGET-DEISA.MEDIUM.256.1HWCG.chop1.bursts.gnuplot \
  ./figure_sources/stats_plot/stats_plot.pdf \
  ./figure_sources/clustering_xml/xml_plot_definition.pdf \
  ./figure_sources/clustering_xml/xml_structure.pdf \
  ./figure_sources/clustering_xml/xml_single_event_definition.pdf \
  ./figure_sources/clustering_xml/xml_mixed_events_definition.pdf \
  ./figure_sources/clustering_xml/xml_algorithm_definition.pdf \
  ./figure_sources/clustering_xml/clustering_xml_definition.graffle \
  ./cluster_analysis_thesis_bibliography.bib \
  ./extra_commands.tex \
  ./figures/uml_design/libclustering_uml.pdf \
  ./figures/uml_design/libtraceclustering_uml.pdf \
  ./figures/plots_example/int_vs_fop_plot.pdf \
  ./figures/plots_example/inst_vs_ipc_plot.pdf \
  ./figures/plots_example/mainmem_vs_l2_plot.pdf \
  ./figures/plots_example/int_vs_fop_plot.png \
  ./figures/plots_example/inst_vs_ipc_plot.png \
  ./figures/plots_example/stores_vs_loads_plot.pdf \
  ./figures/plots_example/stores_vs_loads_plot.png \
  ./figures/plots_example/mainmem_vs_l2_plot.png \
  ./figures/stats_plot/stats_plot.pdf \
  ./figures/refinement_tree/refinement_tree.pdf \
  ./figures/clustering_xml/xml_plot_definition.pdf \
  ./figures/clustering_xml/xml_structure.pdf \
  ./figures/clustering_xml/xml_single_event_definition.pdf \
  ./figures/clustering_xml/xml_mixed_events_definition.pdf \
  ./figures/clustering_xml/xml_algorithm_definition.pdf \
  ./figures/paraver_example/paraver_clusters_timeline.png \
  ./figures/paraver_example/paraver_clusters_profile.png \
  ./figures/sequences_example/gromacs_step10_sequences.png \
  ./figures/sequences_example/sequences_clustalx.png
   

manual_ug = clusteringsuite_manual

#
# Aquesta combinacio de params pot ser interessant per a generar un single HTML
# -split 0 -show_section_numbers -nonumbered_footnotes -mkdir -dir single

all:
	@echo Nothing to be done by default. If you want to build the documentation issue make build-documentation.

build-documentation: $(manual_ug).tex extra_commands.tex cluster_analysis_thesis_bibliography.bib
	@echo "\newcommand{\TRACEVERSION}{$(PACKAGE_VERSION)}" > version.tex
if BUILD_DOCS_PDF
	$(pdflatex) $(manual_ug)
endif

#if BUILD_HTML_DOCS
#	$(latex2html) -split 0 -show_section_numbers -nonumbered_footnotes $(manual_ug)
#	sed s/HREF=\"user-guide.html/HREF=\"/ < user-guide/index.html > user-guide/index.2.html
#	sed s/SRC=\"img/SRC=\"http:\\/\\/www.bsc.es\\/sites\\/default\\/files\\/public\\/computer_science\\/performance_tools\\/img/ < user-guide/index.2.html > user-guide/index.bsc.html
#	rm -fr user-guide/index.2.html
#	cd ..
#endif

install-exec-hook: $(manual_ug).pdf
	$(MKDIR_P) $(DESTDIR)$(datadir)/doc
#	cp $(manual_ug).ps $(DESTDIR)$(datadir)/doc
	cp $(manual_ug).pdf $(DESTDIR)$(datadir)/doc
#if BUILD_HTML_DOCS
#	$(MKDIR_P) $(DESTDIR)$(datadir)/doc/html
#	cp user-guide/user-guide.html user-guide/user-guide.css user-guide/*.png $(DESTDIR)$(datadir)/doc/html
#	ln -f $(DESTDIR)$(datadir)/doc/html/user-guide.html $(DESTDIR)$(datadir)/doc/html/index.html
#endif

clean:
	@echo Nothing to be done by default. If you want to clean the documentation issue make clean-documentation.


TEMPORARY_FILES=\
 $(manual_ug).aux\
 $(manual_ug).bbl\
 $(manual_ug).blg\
 $(manual_ug).lof\
 $(manual_ug).log\
 $(manual_ug).lot\
 $(manual_ug).maf\
 $(manual_ug).mtc\
 $(manual_ug).mtc0\
 $(manual_ug).out\
 $(manual_ug).pdf\
 $(manual_ug).toc\
 extra_commands.aux\
 version.tex
 
clean-documentation:
	rm -rf $(TEMPORARY_FILES)
