ifndef DVIPS
 DVIPS=dvips
endif

ifndef LATEX
 LATEX=latex
endif

ifndef PDFLATEX
 PDFLATEX=latex
endif

ifndef BIBTEX
 BIBTEX=bibtex
endif


# Run it twice to get cross-references correct.
%.dvi: %.tex
	(TEXINPUTS=$(TEXINPUTS):$(OMEGA_HOME)/Docs:: ; export TEXINPUTS ; $(LATEX) $<)
	$(BIBTEX) $*
	(TEXINPUTS=$(TEXINPUTS):$(OMEGA_HOME)/Docs:: ; export TEXINPUTS ; $(LATEX) $<)

%.ps: %.dvi
	(DVIPSHEADERS=$(OMEGA_HOME)/Docs:$(DVIPSHEADERS) ; export DVIPSHEADERS ; $(DVIPS) -o $@ $<)

%.pdf: %.tex
	(TEXINPUTS=$(TEXINPUTS):$(OMEGA_HOME)/Docs:: ; export TEXINPUTS ; $(PDFLATEX) $<)
	(TEXINPUTS=$(TEXINPUTS):$(OMEGA_HOME)/Docs:: ; export TEXINPUTS ; $(PDFLATEX) $<)
