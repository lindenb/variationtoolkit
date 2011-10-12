all: varkit.pdf
varkit.pdf:varkit.dvi
	dvipdf $<
varkit.dvi:varkit.tex
	latex --batchmode $<
clean:
	rm -f varkit.dvi varkit.pdf varkit.aux varkit.log varkit.out
