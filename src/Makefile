BINDIR=../bin
CPP=g++
CFLAGS=-O3 -Wall
SAMDIR=../ext/samtools
TABIXDIR=../ext/tabix
OBJCPY= objcopy --input binary --output elf64-x86-64 --binary-architecture i386
all:$(BINDIR)/scanvcf \
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
	$(BINDIR)/vcfbigwig

$(BINDIR)/scanvcf: scanvcf.cpp tokenizer.h throw.h zstreambuf.h
	$(CPP) -o $@ $< $(CFLAGS) -lz
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
$(BINDIR)/samplespersnp: samplespersnp.cpp tokenizer.h throw.h zstreambuf.h
	$(CPP) -o $@ $< $(CFLAGS) -lz
$(BINDIR)/ranges: ranges.cpp tokenizer.h throw.h zstreambuf.h
	$(CPP) -o $@ $< $(CFLAGS) -lz
$(BINDIR)/manhattan: manhattan.cpp tokenizer.h throw.h zstreambuf.h
	$(CPP) -o $@ $< $(CFLAGS) -lz
$(BINDIR)/prediction: prediction.cpp knowngene.o xfaidx.o geneticcode.o 
	$(CPP) -o $@ $< $(CFLAGS) knowngene.o xfaidx.o geneticcode.o  -L $(SAMDIR)  `mysql_config --cflags  --libs` -lz -lbam
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



$(BINDIR)/ncbiesearch:ncbiesearch.cpp netstreambuf.o
	$(CPP) -o $@ $< $(CFLAGS) netstreambuf.o  `xml2-config --cflags --libs` `curl-config --cflags --libs` -lz
$(BINDIR)/ncbiefetch : ncbiefetch.cpp netstreambuf.o pubmedxsl.o nuccorexsl.o dbsnpxsl.o genexsl.o taxonomyxsl.o
	$(CPP) -o $@ $< $(CFLAGS) netstreambuf.o  pubmedxsl.o nuccorexsl.o dbsnpxsl.o  genexsl.o taxonomyxsl.o `xml2-config --cflags --libs` `curl-config --cflags --libs` `xslt-config --cflags --libs` -lz



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
	
#UCSC
$(BINDIR)/vcfbigwig: vcfbigwig.cpp kentBigWig.o
	if [ -z "${KENTDIR}" ] ;then $(CPP) -o $@ $< kentBigWig.o $(CFLAGS) -lz; else $(CPP)  -o $@ $< kentBigWig.o $(CFLAGS) ${KENTDIR}/src/lib/*/jkweb.a -lz -lm -lssl; fi

kentBigWig.o: kentBigWig.cpp kentBigWig.h
	if ! [ -z "${KENTDIR}" ] ;then $(CPP) -c -o $@ $< -I $(KENTDIR)/src/inc $(CFLAGS) ; else $(CPP) -c -o $@ $< -DNOKENTSRC $(CFLAGS)  ; fi
	
	
	
man:
	mkdir -p ../man
	for S in groupbygene ; do  awk '/^__BEGIN_DOC__/ {i=1;next;} /^__END_DOC__/ {i=0;next} {if(i==1) print;}' $${S}.cpp > ../man/$${S}.1 ; done
