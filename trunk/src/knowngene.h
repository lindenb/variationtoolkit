#ifndef UCSC_KGENE_H
#define UCSC_KGENE_H
#include <vector>
#include <string>
#include <stdint.h>

class KnownGene;


struct Exon
	{
		KnownGene* gene;
		int32_t index;
		int32_t start;
		int32_t end;
		int32_t size() const;
	};

class KnownGene
	{
	public:
		std::string chrom;
		std::string name;
		std::string name2;
		char strand;
		int32_t txStart;
		int32_t txEnd;
		int32_t cdsStart;
		int32_t cdsEnd;
		std::vector<Exon> exons;
		const Exon* exon(int32_t  idx) const;
		int32_t countExons() const;
	};
#endif
