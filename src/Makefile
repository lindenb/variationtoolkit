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
	$(BINDIR)/ttmap


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
$(BINDIR)/bam2wig: bam2wig.cpp application.o 
	$(CPP) -o $@ $^ $(CFLAGS) -I $(SAMDIR) -L $(SAMDIR)  -lbam -lz
$(BINDIR)/ttmap: ttmap.cpp application.o 
	$(CPP) -o $@ $^ $(CFLAGS) -lz



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
	

test:$(BINDIR)/redonstructvar
	$<  -f /GENOTYPAGE/data/pubdb/ucsc/hg19/chromosomes/hg19.fa\
	    -b jeter.bam.txt jeter.kg.txt
	
