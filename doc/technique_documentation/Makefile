TARGETS			= tNETacle_Documentation_Technique_FR.pdf

RELPATH 		= $(shell pwd)

RESSOURCE_FILES		=	$(RELPATH)/tnetacle.cls	\
				$(RELPATH)/tnetacle-en.cls \
				$(RELPATH)/tnetacle-fr.cls \
				$(RELPATH)/back-tnetacle.jpeg \
				$(RELPATH)/client.tex \
				$(RELPATH)/core.tex \
				$(RELPATH)/share.tex \
				$(RELPATH)/externe.tex \

export TEXINPUTS	:=  .:$(RELPATH):${TEXINPUTS}:

ALL_DEPENDENCIES	=	$(RESSOURCE_FILES)

all: $(TARGETS)

clean:
	rm -f *.aux *.ent *.log *.out *.toc *.nav *.snm *.vrb *.lof

distclean: clean
	rm -f $(TARGETS)

re: distclean all

%.pdf: %.tex Makefile $(ALL_DEPENDENCIES)
	pdflatex -halt-on-error $< || echo "Build FAILED"
	pdflatex -halt-on-error $< && echo "Build OK" || echo "Build FAILED"

view: all
	evince $(TARGETS)
