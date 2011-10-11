BINDIR=../bin
CPP=g++
CFLAGS=-O3 -Wall
SAMDIR=../ext/samtools
TABIXDIR=../ext/tabix
all:$(BINDIR)/scanvcf \
	$(BINDIR)/groupbygene \
	$(BINDIR)/numericsplit \
	$(BINDIR)/extractinfo \
	$(BINDIR)/extractformat \
	$(BINDIR)/normalizechrom \
	$(BINDIR)/samplespersnp 

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
$(BINDIR)/manhattan: manhattan.cpp tokenizer.h throw.h zstreambuf.h
	$(CPP) -o $@ $< $(CFLAGS) -lz
$(BINDIR)/prediction: prediction.cpp knowngene.o xfaidx.o geneticcode.o 
	$(CPP) -o $@ $< $(CFLAGS) knowngene.o xfaidx.o geneticcode.o  -L $(SAMDIR)  `mysql_config --cflags  --libs` -lz -lbam

bin.o:bin.cpp bin.h
	$(CPP) -c -o $@ $< $(CFLAGS)
knowngene.o: knowngene.cpp knowngene.h
	$(CPP) -c -o $@ $< $(CFLAGS)
xfaidx.o: xfaidx.cpp xfaidx.h
	$(CPP) -c -o $@ $< $(CFLAGS) -I $(SAMDIR)
xtabix.o: xtabix.cpp xtabix.h
	$(CPP) -c -o $@ $< $(CFLAGS) -I $(TABIXDIR) 
geneticcode.o: geneticcode.cpp geneticcode.h
	$(CPP) -c -o $@ $< $(CFLAGS)
	
man:
	mkdir -p ../man
	for S in groupbygene ; do  awk '/^__BEGIN_DOC__/ {i=1;next;} /^__END_DOC__/ {i=0;next} {if(i==1) print;}' $${S}.cpp > ../man/$${S}.1 ; done
