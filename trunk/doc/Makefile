all: varkit.pdf
varkit.pdf:varkit.dvi
	dvipdf $<
varkit.dvi:varkit.tex varkit.bib manhattan.eps ucscgenes.eps
	latex  $<
	latex  $<
	bibtex varkit
	latex  $<
	latex  $<
clean:
	rm -f varkit.dvi varkit.pdf varkit.aux varkit.log varkit.out varkit.blg varkit.bbl
