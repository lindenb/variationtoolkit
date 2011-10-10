BINDIR=../bin
CPP=g++
CFLAGS=-O3 -Wall
all:$(BINDIR)/scanvcf \
	$(BINDIR)/groupbygene

$(BINDIR)/scanvcf: scanvcf.cpp tokenizer.h throw.h zstreambuf.h
	$(CPP) -o $@ $< $(CFLAGS) -lz
$(BINDIR)/groupbygene: groupbygene.cpp tokenizer.h throw.h zstreambuf.h
	$(CPP) -o $@ $< $(CFLAGS) -lz
	