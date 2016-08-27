#
# doc/Makefile: n64chain documentation Makefile.
#
# n64chain: A (free) open-source N64 development toolchain.
# Copyright 2014-15 Tyler J. Stachecki <tstache1@binghamton.edu>
#
# This file is subject to the terms and conditions defined in
# 'LICENSE', which is part of this source code package.
#

DOC = n64chain-doc

TEX = n64chain-doc.tex
BIBS = n64chain-doc.bib
FIGS =

all: $(DOC).pdf

$(DOC).pdf: $(TEX) $(BIB) $(FIGS)
	@pdflatex $(DOC)
	@pdflatex $(DOC)
	#@bibtex $(DOC)
	@pdflatex $(DOC)
	@pdflatex $(DOC)

.PHONY: clean
clean:
	@rm -f $(DOC).pdf *.aux *.bbl *.blg *.log *.out

