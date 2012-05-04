include ../config.mk
BINDIR=../bin
INSTALLDIR=/usr/local/bin
CPP=g++
CFLAGS=-O3 -Wall
COLOR_O=parsecolor.tab.o lex.parsecolor.o color.o
CHARSEQ=abstractcharsequence.o \
	genomicsequence.o \
	mutatedsequence.o \
	proteincharsequence.o \
	stringsequence.o
PARSESEG=segparser.tab.o lex.segparser.o segments.o


TARGET_BINARIES=$(BINDIR)/scanvcf \
	$(BINDIR)/verticalize \
	$(BINDIR)/groupbygene \
	$(BINDIR)/numericsplit \
	$(BINDIR)/extractinfo \
	$(BINDIR)/extractformat \
	$(BINDIR)/normalizechrom \
	$(BINDIR)/manhattan \
	$(BINDIR)/vcftabix \
	$(BINDIR)/prediction \
	$(BINDIR)/samplespersnp \
	$(BINDIR)/groupbysnp \
	$(BINDIR)/dnacontext \
	$(BINDIR)/ranges \
	$(BINDIR)/vcfttview \
	$(BINDIR)/ncbiesearch \
	$(BINDIR)/ncbiefetch \
	$(BINDIR)/mysqlucsc \
	$(BINDIR)/mysqlquery \
	$(BINDIR)/vcfbigbed \
	$(BINDIR)/vcfbigwig \
	$(BINDIR)/uniprot \
	$(BINDIR)/pfamscan \
	$(BINDIR)/vcf2bed \
	$(BINDIR)/emblstringresolve \
	$(BINDIR)/emblstringinteractions \
	$(BINDIR)/emblstringinteractors \
	$(BINDIR)/vcfcut \
	$(BINDIR)/ucscgenesps \
	$(BINDIR)/vcfintersect \
	$(BINDIR)/vcfliftover \
	$(BINDIR)/backlocate \
	$(BINDIR)/igvcontrol \
	$(BINDIR)/shortreadsim \
	$(BINDIR)/bedmerge \
	$(BINDIR)/genomesim \
	$(BINDIR)/loessapp \
	$(BINDIR)/bam2wig \
	$(BINDIR)/ttmap \
	$(BINDIR)/vcf2sqlite \
	$(BINDIR)/fastatail \
	$(BINDIR)/fasta2tsv \
	$(BINDIR)/vcf2xml \
	$(BINDIR)/fastaslice \
	$(BINDIR)/fastasortuniq \
	$(BINDIR)/fastatac \
	$(BINDIR)/fastarevcomp \
	$(BINDIR)/ncbielink \
	$(BINDIR)/ncbicited \
	$(BINDIR)/protein2genome \
	$(BINDIR)/cmpdepth \
	$(BINDIR)/fasta2term \
	$(CGI_BIN_DIR)/ngsproject.cgi \
	$(BINDIR)/xsltstream \
	$(BINDIR)/godbmgr \
	$(BINDIR)/casava2vcf \
	$(BINDIR)/vcfid \
	$(BINDIR)/showprogress \
	$(BINDIR)/md5cols \
	$(BINDIR)/vcf2snpeff \
	$(BINDIR)/bedsplit \
	$(BINDIR)/beddepth \
	$(BINDIR)/extractsnpeff \
	$(BINDIR)/mpileup2pileup \
	$(BINDIR)/depthofcoverage \
	$(BINDIR)/fixindels 

all:$(TARGET_BINARIES)

$(BINDIR)/scanvcf: scanvcf.cpp tokenizer.h throw.h zstreambuf.h
	$(CPP) -o $@ $< $(CFLAGS) -lz
$(BINDIR)/verticalize: verticalize.cpp application.o
	$(CPP) -o $@ $< application.o $(CFLAGS) -lz
$(BINDIR)/groupbygene: groupbygene.cpp tokenizer.h throw.h zstreambuf.h
	$(CPP) -o $@ $< $(CFLAGS) -lz
$(BINDIR)/normalizechrom: normalizechrom.cpp tokenizer.h throw.h zstreambuf.h
	$(CPP) -o $@ $< $(CFLAGS) -lz
$(BINDIR)/numericsplit: numericsplit.cpp tokenizer.h throw.h zstreambuf.h
	$(CPP) -o $@ $< $(CFLAGS) -lz
$(BINDIR)/extractinfo: extractinfo.cpp tokenizer.h throw.h zstreambuf.h
	$(CPP) -o $@ $< $(CFLAGS) -lz
$(BINDIR)/extractsnpeff: extractsnpeff.cpp tokenizer.h throw.h zstreambuf.h
	$(CPP) -o $@ $< $(CFLAGS) -lz
$(BINDIR)/mpileup2pileup: mpileup2pileup.cpp tokenizer.h throw.h zstreambuf.h
	$(CPP) -o $@ $< $(CFLAGS) -lz
$(BINDIR)/extractformat: extractformat.cpp tokenizer.h throw.h zstreambuf.h
	$(CPP) -o $@ $< $(CFLAGS) -lz
$(BINDIR)/samplespersnp: samplespersnp.cpp application.o selectset.l selectset.y
	flex -B -i -Pselectset selectset.l
	bison --locations -v selectset.y -b selectset -d
	$(CPP) -o $@ samplespersnp.cpp application.o lex.selectset.c selectset.tab.c $(CFLAGS) -lz
$(BINDIR)/groupbysnp: groupbysnp.cpp application.o
	$(CPP) -o $@ $^ $(CFLAGS) -lz
$(BINDIR)/ranges: ranges.cpp tokenizer.h throw.h zstreambuf.h
	$(CPP) -o $@ $< $(CFLAGS) -lz
$(BINDIR)/manhattan: manhattan.cpp $(COLOR_O) application.o
	$(CPP) -o $@ $^ $(CFLAGS) -lz
$(BINDIR)/prediction: prediction.cpp knowngene.o xfaidx.o geneticcode.o application.o $(CHARSEQ)
	$(CPP) -o $@ $^ $(CFLAGS) -L $(SAMDIR)  `mysql_config --cflags  --libs` -lbam -lz
$(BINDIR)/backlocate: backlocate.cpp knowngene.o xfaidx.o geneticcode.o application.o $(CHARSEQ)
	$(CPP) -o $@ $^ $(CFLAGS) -L $(SAMDIR)  `mysql_config --cflags  --libs` -lbam -lz
$(BINDIR)/dnacontext: dnacontext.cpp xfaidx.o
	$(CPP) -o $@ $< $(CFLAGS) xfaidx.o  -L $(SAMDIR) -lbam -lz
$(BINDIR)/vcftabix: vcftabix.cpp  xtabix.o 
	$(CPP) -o $@ $< $(CFLAGS) xtabix.o  -L $(TABIXDIR)  `mysql_config --cflags  --libs` -lz -ltabix
$(BINDIR)/vcfttview: vcfttview.cpp ttview.o xbam.o xfaidx.o
	$(CPP) -o $@ $< $(CFLAGS) ttview.o xbam.o xfaidx.o \
		$(SAMDIR)/bam_color.o \
		$(SAMDIR)/bam2bcf.o \
		$(SAMDIR)/errmod.o \
		-L $(SAMDIR) -L $(SAMDIR)/bcftools -lbam -lbcf  -lz
$(BINDIR)/mysqlucsc: mysqlucsc.cpp  bin.o application.o
	$(CPP) -o $@ $^ $(CFLAGS) `mysql_config --cflags  --libs` -lz 
$(BINDIR)/mysqlquery: mysqlquery.cpp  tokenizer.h throw.h zstreambuf.h
	$(CPP) -o $@ $< $(CFLAGS) `mysql_config --cflags  --libs` -lz 
$(BINDIR)/uniprot: uniprot.cpp  application.o netstreambuf.o
	$(CPP) -o $@ $^ $(CFLAGS) `xml2-config --cflags --libs` `curl-config --cflags --libs` -lz
$(BINDIR)/pfamscan: pfam.cpp  application.o netstreambuf.o
	$(CPP) -o $@ $^ $(CFLAGS) `xml2-config --cflags --libs` `curl-config --cflags --libs` -lz
$(BINDIR)/fixindels: fixindels.cpp
	$(CPP) -o $@ $< $(CFLAGS)


$(BINDIR)/ncbiesearch:ncbiesearch.cpp netstreambuf.o
	$(CPP) -o $@ $< $(CFLAGS) netstreambuf.o  `xml2-config --cflags --libs` `curl-config --cflags --libs` -lz
$(BINDIR)/ncbiefetch : ncbiefetch.cpp netstreambuf.o pubmedxsl.o nuccorexsl.o dbsnpxsl.o genexsl.o taxonomyxsl.o
	$(CPP) -o $@ $< $(CFLAGS) netstreambuf.o  pubmedxsl.o nuccorexsl.o dbsnpxsl.o  genexsl.o taxonomyxsl.o `xml2-config --cflags --libs` `curl-config --cflags --libs` `xslt-config --cflags --libs` -lz
$(BINDIR)/vcf2bed: vcf2bed.cpp application.o 
	$(CPP) -o $@ $< application.o  $(CFLAGS) -lz
$(BINDIR)/emblstringresolve : emblstring.cpp application.o netstreambuf.o
	$(CPP) -o $@ $< application.o netstreambuf.o -DEMBL_RESOLVE $(CFLAGS) `curl-config --cflags --libs` -lz
$(BINDIR)/emblstringinteractions : emblstring.cpp application.o netstreambuf.o
	$(CPP) -o $@ $< application.o netstreambuf.o -DEMBL_INTERACTIONS $(CFLAGS) `curl-config --cflags --libs` -lz
$(BINDIR)/emblstringinteractors : emblstring.cpp application.o netstreambuf.o
	$(CPP) -o $@ $< application.o netstreambuf.o -DEMBL_INTERACTOR $(CFLAGS) `curl-config --cflags --libs` -lz
$(BINDIR)/vcfcut:vcfcut.cpp segparser.tab.o lex.segparser.o application.o segments.o
	$(CPP) $(CFLAGS) -o $@ $^ -lz
$(BINDIR)/ucscgenesps: ucscgenesps.cpp application.o knowngene.o $(COLOR_O)
	$(CPP) -o $@ $^ $(CFLAGS) `mysql_config --cflags  --libs` -lz 
$(BINDIR)/vcfintersect : vcfintersect.cpp application.o segments.o netstreambuf.o 
	$(CPP) -o $@ $^ $(CFLAGS) `curl-config --cflags --libs` -lz
$(BINDIR)/igvcontrol: igv.cpp application.o xcurses.cpp 
	$(CPP) -o $@ $^ $(CFLAGS) -lz -lcurses	
$(BINDIR)/shortreadsim: shortreadsim.cpp application.o  segments.o tarball.o
	$(CPP) -o $@ $^ $(CFLAGS)  -L $(SAMDIR) -lbam -lz 
$(BINDIR)/bedmerge: bedmerge.cpp application.o 
	$(CPP) -o $@ $^ $(CFLAGS) -lz 
$(BINDIR)/genomesim: genomesim.cpp application.o  segments.o tarball.o
	$(CPP) -o $@ $^ $(CFLAGS) -lz
$(BINDIR)/loessapp:loessapp.cpp loess.o application.o 
	$(CPP) -o $@ $^ $(CFLAGS) -lz
$(BINDIR)/redonstructvar: redonstructvar.cpp  xfaidx.o knowngene.o loess.o application.o 
	$(CPP) -o $@ $^ $(CFLAGS) -I $(SAMDIR) -L $(SAMDIR)  -lbam -lz
$(BINDIR)/redondepth: redondepth.cpp  xfaidx.o loess.o application.o xbam2.o segments.o tarball.o
	$(CPP) -o $@ $^ $(CFLAGS) -I $(SAMDIR) -L $(SAMDIR)  -lbam -lz 
$(BINDIR)/bam2wig: bam2wig.cpp application.o 
	$(CPP) -o $@ $^ $(CFLAGS) -I $(SAMDIR) -L $(SAMDIR)  -lbam -lz
$(BINDIR)/ttmap: ttmap.cpp application.o 
	$(CPP) -o $@ $^ $(CFLAGS) -lz
$(BINDIR)/vcf2sqlite: vcf2sqlite.cpp xsqlite.cpp application.o
	if ! [ -z "$(SQLITE_LIB)" ] ;then $(CPP) -o $@ $^ $(CFLAGS) -lz $(SQLITE_CFLAGS) $(SQLITE_LIB) ; else $(CPP) -o $@ vcf2sqlite.cpp  -DNOSQLITE $(CFLAGS)  ; fi
$(BINDIR)/fastatail: fastatail.cpp abstractcharsequence.o fastareader.o
	$(CPP) -o $@ $^ $(CFLAGS) -lz
$(BINDIR)/fasta2tsv: fasta2tsv.cpp
	$(CPP) -o $@ $^ $(CFLAGS) -lz
$(BINDIR)/fastasize: fastasize.cpp abstractcharsequence.o fastareader.o
	$(CPP) -o $@ $^ $(CFLAGS) -lz
$(BINDIR)/vcf2xml: vcf2xml.cpp application.o
	$(CPP) -o $@ $^ $(CFLAGS) `xml2-config --cflags --libs` -lz
$(BINDIR)/fastaslice: fastaslice.cpp abstractcharsequence.o fastareader.o
	$(CPP) -o $@ $^ $(CFLAGS) -lz
$(BINDIR)/fastasortuniq: fastasortuniq.cpp abstractcharsequence.o fastareader.o
	$(CPP) -o $@ $^ $(CFLAGS) -lz
$(BINDIR)/fastatac: fastatac.cpp abstractcharsequence.o fastareader.o
	$(CPP) -o $@ $^ $(CFLAGS) -lz
$(BINDIR)/fastarevcomp: fastarevcomp.cpp abstractcharsequence.o fastareader.o
	$(CPP) -o $@ $^ $(CFLAGS) -lz
$(BINDIR)/ncbielink: ncbielink.cpp netstreambuf.o application.o
	$(CPP) -o $@ $^ $(CFLAGS)  `xml2-config --cflags --libs` `curl-config --cflags --libs` -lz
$(BINDIR)/ncbicited: ncbicited.cpp netstreambuf.o application.o
	$(CPP) -o $@ $^ $(CFLAGS)  `xml2-config --cflags --libs` `curl-config --cflags --libs` -lz
$(BINDIR)/protein2genome: protein2genome.cpp knowngene.o application.o segments.o
	$(CPP) -o $@ $^ $(CFLAGS) -L $(SAMDIR)  `mysql_config --cflags  --libs` -lbam -lz
$(CGI_BIN_DIR)/ngsproject.cgi:ngsproject.cpp ngsproject2htmlxsl.o cgi.o $(PARSESEG) ttview.o  xbam.o xfaidx.o
	if [ -z "${CGI_BIN_DIR}" ] ; then echo -e "#\n#\n# CGI_BIN_DIR undefined\n#\n#" ; else \
	$(CPP) -o $@ $^  $(CFLAGS) \
		$(SAMDIR)/bam_color.o \
		$(SAMDIR)/bam2bcf.o \
		$(SAMDIR)/errmod.o \
		-L $(SAMDIR) -L $(SAMDIR)/bcftools \
		`xml2-config --cflags --libs` `xslt-config --cflags --libs` \
		-lbam -lbcf  -lz ; fi
$(BINDIR)/cmpdepth: cmpdepth.cpp xbam2.o $(PARSESEG)
	$(CPP) -o $@ $^ $(CFLAGS) -I $(SAMDIR)  -L $(SAMDIR) -lbam   -lz
$(BINDIR)/fasta2term: fasta2term.cpp
	$(CPP) -o $@ $^  $(CFLAGS) 
$(BINDIR)/xsltstream: xsltstream.cpp xstream.o application.o
	$(CPP) -o $@ $^  $(CFLAGS) `xml2-config --cflags --libs` `xslt-config --cflags --libs` -lz
$(BINDIR)/godbmgr: godatabasemgr.cpp xsqlite.cpp application.o xstream.o xxml.o
	if ! [ -z "$(SQLITE_LIB)" ] ;then $(CPP) -o $@ $^ -g `xml2-config --cflags `  $(SQLITE_CFLAGS)   -lz $(SQLITE_LIB) `xml2-config  --libs` ; else $(CPP) -o $@ godatabasemgr.cpp  -DNOSQLITE $(CFLAGS)  ; fi
$(CGI_BIN_DIR)/tabixserver.cgi: tabixserver.cpp $(PARSESEG) xtabix.o xxml.o cgi.o
	xsltproc --stringparam method _schema_fragment1 xml2libxmlwriter.xsl tabixserver-schema.xsd > _xsdschema.cpp
	if [ -z "${CGI_BIN_DIR}" ] ; then echo -e "#\n#\n# CGI_BIN_DIR undefined\n#\n#" ; else \
		$(CPP) -o $@ -DCGI_VERSION $^ _xsdschema.cpp $(CFLAGS)  `xml2-config --cflags `  -L $(TABIXDIR)   `xml2-config --libs `  -ltabix  -lz ; fi
$(BINDIR)/casava2vcf : casava2vcf.cpp
	$(CPP) $(CFLAGS) -o $@ $^ -lz
$(BINDIR)/vcfid : vcfid.cpp  bin.o application.o
	$(CPP) -o $@ $^ $(CFLAGS) `mysql_config --cflags  --libs` -lz
$(BINDIR)/showprogress : progress.cpp
	$(CPP) $(CFLAGS) -o $@ $<
$(BINDIR)/md5cols : md5cols.cpp  application.o
	$(CPP) $(CFLAGS) -o $@ $^  -lz -lssl
$(BINDIR)/vcf2snpeff : vcf2snpeff.cpp
	$(CPP) $(CFLAGS) -o $@ $^  -lz
$(BINDIR)/beddepth: beddepth.cpp xbam2.o $(PARSESEG)
	$(CPP) -o $@ $^ $(CFLAGS) -I $(SAMDIR)  -L $(SAMDIR) -lbam   -lz
$(BINDIR)/depthofcoverage: depthofcoverage.cpp xbam2.o $(PARSESEG)
	$(CPP) -o $@ $^ $(CFLAGS) -I $(SAMDIR)  -L $(SAMDIR) -lbam   -lz
$(BINDIR)/bedsplit: bedsplit.cpp 
	$(CPP) -o $@ $^ $(CFLAGS)


$(BINDIR)/embbedC:embbedC.c
	$(CPP) -o $@ $^
pubmedxsl.o:pubmed.xsl $(BINDIR)/embbedC
	$(BINDIR)/embbedC pubmedxsl < $< > pubmedxsl.cpp
	$(CPP) -c -o $@ pubmedxsl.cpp $(CFLAGS) 
nuccorexsl.o:nuccore.xsl $(BINDIR)/embbedC
	$(BINDIR)/embbedC nuccorexsl < $< > nuccorexsl.cpp
	$(CPP) -c -o $@ nuccorexsl.cpp $(CFLAGS) 
dbsnpxsl.o:dbsnp.xsl $(BINDIR)/embbedC
	$(BINDIR)/embbedC dbsnpxsl < $< > dbsnpxsl.cpp
	$(CPP) -c -o $@ dbsnpxsl.cpp $(CFLAGS) 
genexsl.o:gene.xsl $(BINDIR)/embbedC
	$(BINDIR)/embbedC genexsl < $< > genexsl.cpp
	$(CPP) -c -o $@ genexsl.cpp $(CFLAGS) 
taxonomyxsl.o:taxonomy.xsl $(BINDIR)/embbedC
	$(BINDIR)/embbedC taxonomyxsl < $< > taxonomyxsl.cpp
	$(CPP) -c -o $@ taxonomyxsl.cpp $(CFLAGS) 
ngsproject2htmlxsl.o:ngsproject2html.xsl $(BINDIR)/embbedC
	$(BINDIR)/embbedC ngsproject2html < $< > ngsproject2htmlxsl.cpp
	$(CPP) -c -o $@ ngsproject2htmlxsl.cpp $(CFLAGS)
	
	
	
bin.o:bin.cpp bin.h
	$(CPP) -c -o $@ $< $(CFLAGS)
knowngene.o: knowngene.cpp knowngene.h
	$(CPP) -c -o $@ $< $(CFLAGS)
xfaidx.o: xfaidx.cpp xfaidx.h
	$(CPP) -c -o $@ $< $(CFLAGS) -I $(SAMDIR)
xbgzf.o: xbgzf.cpp xbgzf.h
	$(CPP) -c -o $@ $< $(CFLAGS) -I $(SAMDIR)
xtabix.o: xtabix.cpp xtabix.h
	$(CPP) -c -o $@ $< $(CFLAGS) -I $(TABIXDIR)
ttview.o: ttview.cpp ttview.h xbam.h xfaidx.h
	$(CPP) -c -o $@ $< $(CFLAGS) -I $(SAMDIR)
xbam.o: xbam.cpp xbam.h
	$(CPP) -c -o $@ $< $(CFLAGS) -I $(SAMDIR) 
xbam2.o: xbam2.cpp xbam2.h
	$(CPP) -c -o $@ $< $(CFLAGS) -I $(SAMDIR) 
geneticcode.o: geneticcode.cpp geneticcode.h
	$(CPP) -c -o $@ $< $(CFLAGS)
netstreambuf.o: netstreambuf.cpp netstreambuf.cpp
	$(CPP) -c -o $@ $< $(CFLAGS) `curl-config --cflags`
application.o: application.cpp application.h
	$(CPP) -c -o $@ $< $(CFLAGS)
segments.o:segments.cpp segments.h smartcmp.h
	$(CPP) -c -o $@ $< $(CFLAGS)
xcurses.o: xcurses.cpp xcurses.h
	$(CPP) -c -o $@ $< $(CFLAGS)
color.o: color.cpp color.h
	$(CPP) -c -o $@ $< $(CFLAGS)
tarball.o: tarball.cpp tarball.h
	$(CPP) -g -c -o $@ $< $(CFLAGS)
abstractcharsequence.o: abstractcharsequence.cpp abstractcharsequence.h
	$(CPP) -c -o $@ $< $(CFLAGS)
genomicsequence.o: genomicsequence.cpp genomicsequence.h
	$(CPP) -c -o $@ $< $(CFLAGS)
proteincharsequence.o:proteincharsequence.cpp proteincharsequence.h
	$(CPP) -c -o $@ $< $(CFLAGS)
mutatedsequence.o : mutatedsequence.cpp mutatedsequence.h
	$(CPP) -c -o $@ $< $(CFLAGS)
stringsequence.o : stringsequence.cpp stringsequence.h
	$(CPP) -c -o $@ $< $(CFLAGS)
xstdio.o: xstdio.cpp xstdio.h
	$(CPP) -c -o $@ $< $(CFLAGS)
xstdlib.o: xstdlib.cpp xstdlib.h
	$(CPP) -c -o $@ $< $(CFLAGS)
loess.o:loess.cpp loess.h
	$(CPP) -c -o $@ $< $(CFLAGS)
fastareader.o:fastareader.cpp fastareader.h
	$(CPP) -c -o $@ $< $(CFLAGS)
cgi.o:cgi.cpp cgi.h
	$(CPP) -c -o $@ $< $(CFLAGS)
xstream.o: xstream.cpp xstream.h
	$(CPP) -c -o $@ $< $(CFLAGS) `xml2-config --cflags `
xxml.o: xxml.cpp xxml.h
	$(CPP) -c -o $@ $< $(CFLAGS) `xml2-config --cflags `


keyvaluedatastore.o: keyvaluedatastore.cpp keyvaluedatastore.h
	if ! [ -z "$(LEVELDBDIR)" ] ;then $(CPP) -c -o $@ $< -I $(LEVELDBDIR)/include $(CFLAGS) ; else $(CPP) -c -o $@ $< -DNOLEVELDB $(CFLAGS)  ; fi

segparser.tab.o lex.segparser.o: parsereg.l parsereg.y
	flex -B -i -Psegparser $<
	bison --locations -v parsereg.y -b segparser -d
	$(CPP) -g -c -o lex.segparser.o lex.segparser.c
	$(CPP) -g  -c -o segparser.tab.o segparser.tab.c
	
#UCSC
$(BINDIR)/vcfbigwig: vcfbigwig.cpp kentBigWig.o
	if [ -z "${KENTDIR}" ] ;then $(CPP) -o $@ $^ $(CFLAGS) -lz; else $(CPP)  -o $@ $^ $(CFLAGS) ${KENTDIR}/src/lib/*/jkweb.a -Wl,--start-group -lz -lm -lssl3 -lssl -Wl,--end-group; fi
$(BINDIR)/vcfbigbed: vcfbigbed.cpp kentBigBed.o
	if [ -z "${KENTDIR}" ] ;then $(CPP) -o $@ $^ $(CFLAGS) -lz; else $(CPP)  -o $@ $^ $(CFLAGS) ${KENTDIR}/src/lib/*/jkweb.a -Wl,--start-group -lz -lm -lssl3 -lssl -Wl,--end-group; fi
$(BINDIR)/vcfliftover: vcfliftover.cpp xliftOver.o application.o segments.o
	if [ -z "${KENTDIR}" ] ;then $(CPP) -o $@ $^ $(CFLAGS) -lz; else $(CPP)  -o $@ $^ $(CFLAGS) ${KENTDIR}/src/lib/*/*.a  -Wl,--start-group -lz -lm -lssl3 -lssl -Wl,--end-group `mysql_config --libs`; fi

kentBigWig.o: kentBigWig.cpp kentBigWig.h
	if ! [ -z "${KENTDIR}" ] ;then $(CPP) -c -o $@ $< -I $(KENTDIR)/src/inc $(CFLAGS) ; else $(CPP) -c -o $@ $< -DNOKENTSRC $(CFLAGS)  ; fi
kentBigBed.o: kentBigBed.cpp kentBigBed.h
	if ! [ -z "${KENTDIR}" ] ;then $(CPP) -c -o $@ $< -I $(KENTDIR)/src/inc $(CFLAGS) ; else $(CPP) -c -o $@ $< -DNOKENTSRC $(CFLAGS)  ; fi
xliftOver.o:xliftOver.cpp xliftOver.h
	if ! [ -z "${KENTDIR}" ] ;then $(CPP) -c -o $@ $< -I $(KENTDIR)/src/inc -I $(KENTDIR)/src/hg/inc $(CFLAGS) ; else $(CPP) -c -o $@ $< -DNOKENTSRC $(CFLAGS)  ; fi


#not in production
$(BINDIR)/sqlitequery: sqlitequery.cpp application.o
	$(CPP) -o $@ $< application.o $(CFLAGS) -lz -lsqlite3



parsecolor.tab.o lex.parsecolor.o: parsecolor.l parsecolor.y color.o
	flex -B -i -Pparsecolor $<
	bison --locations -v parsecolor.y -b parsecolor -d
	$(CPP) $(CFLAGS) -c -o lex.parsecolor.o lex.parsecolor.c
	$(CPP) $(CFLAGS) -c -o parsecolor.tab.o parsecolor.tab.c

install:all
	cp $(TARGET_BINARIES) $(INSTALLDIR)

uninstall:
	rm -f $(subst $(BINDIR),$(INSTALLDIR),$(TARGET_BINARIES))

clean:
	rm -f $(TARGET_BINARIES) *.o a.out lex.segparser.c \
		segparser.tab.c segparser.tab.h segparser.output \
		lex.selectset.c parsecolor.tab.c lex.parsecolor.c  lex.yy.c  \
		selectset.tab.c parsecolor.tab.h selectset.tab.h \
		dbsnpxsl.cpp  genexsl.cpp  nuccorexsl.cpp  \
		pubmedxsl.cpp  taxonomyxsl.cpp selectset.output parsecolor.output

clean-all:clean uninstall
	

#test:$(CGI_BIN_DIR)/tabixserver.cgi
#test2: random.o tarball.o
#	g++ -Wall -g -o a.out vcfsim.cpp random.o tarball.o
test:$(BINDIR)/redondepth
	$<  -f /commun/data/pubdb/ucsc/hg19/chromosomes/hg19.fa \
	    -g jeter.kg.txt \
		/commun/data/projects/NGS/brugada2011/data/Brs[1-9]/rmdup.bam \
		/commun/data/projects/NGS/brugada2011/data/Brs10/rmdup.bam 

#test2:
#	 g++ -DTEST_THIS_CODE -Wall xstream.cpp `xml2-config --cflags --libs`
#	 ./a.out jeter.xml
