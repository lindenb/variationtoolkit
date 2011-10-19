include ../config.mk
BINDIR=../bin
CPP=g++
CFLAGS=-O3 -Wall
OBJCPY= objcopy --input binary --output ${OBJCPYARCH} --binary-architecture i386
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
	$(BINDIR)/vcf2bed \
	$(BINDIR)/emblstringresolve \
	$(BINDIR)/emblstringinteractions \
	$(BINDIR)/emblstringinteractors \
	$(BINDIR)/vcfcut \
	$(BINDIR)/ucscgenesps

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
$(BINDIR)/ranges: ranges.cpp tokenizer.h throw.h zstreambuf.h
	$(CPP) -o $@ $< $(CFLAGS) -lz
$(BINDIR)/manhattan: manhattan.cpp parsecolor.tab.o lex.parsecolor.o color.o application.o
	$(CPP) -o $@ $^ $(CFLAGS) -lz
$(BINDIR)/prediction: prediction.cpp knowngene.o xfaidx.o geneticcode.o application.o
	$(CPP) -o $@ $^ $(CFLAGS) -L $(SAMDIR)  `mysql_config --cflags  --libs` -lz -lbam
$(BINDIR)/dnacontext: dnacontext.cpp xfaidx.o
	$(CPP) -o $@ $< $(CFLAGS) xfaidx.o  -L $(SAMDIR) -lz -lbam
$(BINDIR)/vcftabix: vcftabix.cpp  xtabix.o 
	$(CPP) -o $@ $< $(CFLAGS) xtabix.o  -L $(TABIXDIR)  `mysql_config --cflags  --libs` -lz -ltabix
$(BINDIR)/vcfttview: vcfttview.cpp ttview.o xbam.o xfaidx.o
	$(CPP) -o $@ $< $(CFLAGS) ttview.o xbam.o xfaidx.o \
		$(SAMDIR)/bam_color.o \
		$(SAMDIR)/bam2bcf.o \
		$(SAMDIR)/errmod.o \
		-L $(SAMDIR) -L $(SAMDIR)/bcftools  -lz -lbam -lbcf
$(BINDIR)/mysqlucsc: mysqlucsc.cpp  tokenizer.h throw.h zstreambuf.h
	$(CPP) -o $@ $< $(CFLAGS) `mysql_config --cflags  --libs` -lz 
$(BINDIR)/mysqlquery: mysqlquery.cpp  tokenizer.h throw.h zstreambuf.h
	$(CPP) -o $@ $< $(CFLAGS) `mysql_config --cflags  --libs` -lz 
$(BINDIR)/uniprot: uniprot.cpp  application.o netstreambuf.o
	$(CPP) -o $@ $< application.o netstreambuf.o $(CFLAGS) `xml2-config --cflags --libs` `curl-config --cflags --libs` -lz

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
$(BINDIR)/ucscgenesps: ucscgenesps.cpp application.o knowngene.o 
	$(CPP) -o $@ $^ $(CFLAGS) `mysql_config --cflags  --libs` -lz 

pubmedxsl.o:pubmed.xsl
	$(OBJCPY) $< $@
nuccorexsl.o:nuccore.xsl
	$(OBJCPY) $< $@
dbsnpxsl.o:dbsnp.xsl
	$(OBJCPY) $< $@
genexsl.o:gene.xsl
	$(OBJCPY) $< $@
taxonomyxsl.o:taxonomy.xsl
	$(OBJCPY) $< $@

bin.o:bin.cpp bin.h
	$(CPP) -c -o $@ $< $(CFLAGS)
knowngene.o: knowngene.cpp knowngene.h
	$(CPP) -c -o $@ $< $(CFLAGS)
xfaidx.o: xfaidx.cpp xfaidx.h
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
	
keyvaluedatastore.o: keyvaluedatastore.cpp keyvaluedatastore.h
	if ! [ -z "$(LEVELDBDIR)" ] ;then $(CPP) -c -o $@ $< -I $(LEVELDBDIR)/include $(CFLAGS) ; else $(CPP) -c -o $@ $< -DNOLEVELDB $(CFLAGS)  ; fi

segparser.tab.o lex.segparser.o: parsereg.l parsereg.y
	flex -B -i -Psegparser $<
	bison --locations -v parsereg.y -b segparser -d
	$(CPP) -g -c -o lex.segparser.o lex.segparser.c
	$(CPP) -g  -c -o segparser.tab.o segparser.tab.c
	
#UCSC
$(BINDIR)/vcfbigwig: vcfbigwig.cpp kentBigWig.o
	if [ -z "${KENTDIR}" ] ;then $(CPP) -o $@ $< kentBigWig.o $(CFLAGS) -lz; else $(CPP)  -o $@ $< kentBigWig.o $(CFLAGS) ${KENTDIR}/src/lib/*/jkweb.a -lz -lm -lssl; fi
$(BINDIR)/vcfbigbed: vcfbigbed.cpp kentBigBed.o
	if [ -z "${KENTDIR}" ] ;then $(CPP) -o $@ $< kentBigBed.o $(CFLAGS) -lz; else $(CPP)  -o $@ $< kentBigBed.o $(CFLAGS) ${KENTDIR}/src/lib/*/jkweb.a -lz -lm -lssl; fi

kentBigWig.o: kentBigWig.cpp kentBigWig.h
	if ! [ -z "${KENTDIR}" ] ;then $(CPP) -c -o $@ $< -I $(KENTDIR)/src/inc $(CFLAGS) ; else $(CPP) -c -o $@ $< -DNOKENTSRC $(CFLAGS)  ; fi
kentBigBed.o: kentBigBed.cpp kentBigBed.h
	if ! [ -z "${KENTDIR}" ] ;then $(CPP) -c -o $@ $< -I $(KENTDIR)/src/inc $(CFLAGS) ; else $(CPP) -c -o $@ $< -DNOKENTSRC $(CFLAGS)  ; fi

#not in production
$(BINDIR)/sqlitequery: sqlitequery.cpp application.o
	$(CPP) -o $@ $< application.o $(CFLAGS) -lz -lsqlite3

$(BINDIR)/angelheart: angelheart.cpp application.o xcurses.cpp segments.o
	$(CPP) -o $@ $^ $(CFLAGS) -lz -lcurses

parsecolor.tab.o lex.parsecolor.o: parsecolor.l parsecolor.y color.o
	flex -B -i -Pparsecolor $<
	bison --locations -v parsecolor.y -b parsecolor -d
	$(CPP) $(CFLAGS) -c -o lex.parsecolor.o lex.parsecolor.c
	$(CPP) $(CFLAGS) -c -o parsecolor.tab.o parsecolor.tab.c

clean:
	rm -f $(TARGET_BINARIES) *.o a.out lex.segparser.c \
		segparser.tab.c segparser.tab.h segparser.output
