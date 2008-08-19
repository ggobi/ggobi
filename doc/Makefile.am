dist_doc_DATA = manual.pdf XML.pdf

all-local: manual.pdf XML.pdf

PDFLATEX = @PDFLATEX@
BIBTEX = @BIBTEX@

%.pdf: %.tex
	@if test "$(PDFLATEX)" = false; then \
	  $(ECHO) "ERROR: 'pdflatex' needed but missing on your system."; \
	  exit 1; \
	fi
	@if test "$(BIBTEX)" = false; then \
	  $(ECHO) "ERROR: 'bibtex' needed but missing on your system."; \
	  exit 1; \
	fi
	$(PDFLATEX) $<
	-$(BIBTEX) $*
	$(PDFLATEX) $<
	$(PDFLATEX) $<