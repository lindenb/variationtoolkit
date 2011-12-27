%.pnm:%.jpg
	jpegtopnm $< > $@
%.ps:%.pnm
	pnmtops $< > $@
%.epsi:%.ps
	ps2epsi $<
%.ps:%.svg
	inkscape --without-gui --export-ps=$@ $<

all: varkit.pdf
varkit.pdf:varkit.dvi
	dvipdf $<
varkit.dvi:varkit.tex varkit.bib manhattan.eps ucscgenes.eps \
		igvcontrol.epsi \
		schema01.epsi
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
		 varkit.bbl  igvcontrol.pnm  igvcontrol.ps igvcontrol.epsi \
		 schema01.epsi  schema01.ps

