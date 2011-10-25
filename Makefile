TABIXVERSION=0.2.5
SAMTOOLSVERSION=0.1.18
.PHONY: all  ext manual doc clean
all:
	(cd src;make)
manual:
	(cd src;make man)
doc:
	(cd doc; make )

install:
	(cd src; make install)
uninstall:
	(cd src; make uninstall)

clean:
	(cd src; make clean)
	(cd doc; make clean)
