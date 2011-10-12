all: varkit.pdf
varkit.pdf:varkit.dvi
	dvipdf $<
varkit.dvi:varkit.tex
	latex $<
clean:
	rm -f varkit.dvi varkit.pdf
