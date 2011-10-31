all: varkit.pdf
varkit.pdf:varkit.dvi
	dvipdf $<
varkit.dvi:varkit.tex varkit.bib manhattan.eps ucscgenes.eps igvcontrol.epsi
	latex  $<
	latex  $<
	bibtex varkit
	latex  $<
	latex  $<

igvcontrol.pnm:igvcontrol.jpg
	jpegtopnm $< > $@
igvcontrol.ps:igvcontrol.pnm
	pnmtops $< > $@
igvcontrol.epsi:igvcontrol.ps
	ps2epsi $<
clean:
	rm -f varkit.dvi varkit.pdf varkit.aux varkit.log varkit.out varkit.blg \
		 varkit.bbl  igvcontrol.pnm  igvcontrol.ps igvcontrol.epsi
