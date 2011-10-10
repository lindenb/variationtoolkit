BINDIR=../bin
CPP=g++
CFLAGS=-O3 -Wall
all:$(BINDIR)/scanvcf \
	$(BINDIR)/groupbygene \
	$(BINDIR)/numericsplit \
	$(BINDIR)/extractinfo \
	$(BINDIR)/extractformat \
	$(BINDIR)/normalizechrom

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