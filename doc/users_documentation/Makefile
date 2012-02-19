TARGETS			= tNETacle_Documentation_User_FR.pdf

RELPATH 		= $(shell pwd)

RESSOURCE_FILES		=	$(RELPATH)/tnetacle.cls	\
				$(RELPATH)/tnetacle-en.cls \
				$(RELPATH)/tnetacle-fr.cls \
				$(RELPATH)/back-tnetacle.jpeg \
				$(RELPATH)/description.tex	\
				$(RELPATH)/contacts.tex	\
				$(RELPATH)/liens.tex	\
				$(RELPATH)/installation.tex	\
				$(RELPATH)/desinstallation.tex	\
				$(RELPATH)/faq.tex	\
				$(RELPATH)/bugs.tex	\

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
