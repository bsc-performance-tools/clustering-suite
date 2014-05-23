
EXTRA_DIST = \
  clusteringsuite_manual.pdf 

manual_ug = clusteringsuite_manual

#
# Aquesta combinacio de params pot ser interessant per a generar un single HTML
# -split 0 -show_section_numbers -nonumbered_footnotes -mkdir -dir single

all:
	@echo Nothing to be done by default. If you want to build the documentation issue make build-documentation.

build-documentation: $(clusteringsuite_manual).tex extra_commands.tex cluster_analysis_thesis_bibliography.bib
	@echo "\newcommand{\TRACEVERSION}{$(PACKAGE_VERSION)}" > version.tex
if BUILD_DOCS_PDF
	$(dvipdf) $(manual_ug)
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
