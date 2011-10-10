TABIXVERSION=0.2.3
SAMTOOLSVERSION=0.1.18
all:ext/tabix ext/samtools
	(cd src;make)
manual:
	(cd src;make man)

ext/tabix:
	mkdir -p $@
	wget -O ext/tabix-$(TABIXVERSION).tar.bz2 "http://ignum.dl.sourceforge.net/project/samtools/tabix/tabix-$(TABIXVERSION).tar.bz2"
	(cd ext; bunzip2 tabix-$(TABIXVERSION).tar.bz2 ; tar xvf tabix-$(TABIXVERSION).tar; rm tabix-$(TABIXVERSION).tar ; mv tabix-$(TABIXVERSION) tabix;  (cd tabix; make) )

ext/samtools:
	mkdir -p $@
	wget -O ext/samtools-$(TABIXVERSION).tar.bz2 "http://ignum.dl.sourceforge.net/project/samtools/samtools/$(SAMTOOLSVERSION)/samtools-$(SAMTOOLSVERSION).tar.bz2"
	(cd ext; bunzip2 samtools-$(SAMTOOLSVERSION).tar.bz2 ; tar xvf samtools-$(SAMTOOLSVERSION).tar; rm samtools-$(SAMTOOLSVERSION).tar ; mv samtools-$(TABIXVERSION) samtools; (cd samtools; make) )
