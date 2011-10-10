#include "knowngene.h"

using namespace std;

int32_t Exon::size() const
	{
	return end-start;
	}

const Exon* KnownGene::exon(int32_t  idx) const
	{
	return &exons[idx];
	}

int32_t KnownGene::countExons() const
	{
	return (int32_t)exons.size();
	}
