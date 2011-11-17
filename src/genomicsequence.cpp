#include <cctype>
#include "genomicsequence.h"


GenomicSeq::GenomicSeq(IndexedFasta* indexedFasta,
	     const char* chrom,
	     int32_t chromStart0,
	     int32_t chromEnd0
	     )
	{
	this->array = indexedFasta->fetch(chrom,chromStart0,chromEnd0);
	this->chromStart=chromStart0;
	}

GenomicSeq::~GenomicSeq()
	{
	}

int GenomicSeq::getChromStart() const
	{
	return chromStart;
	}
int GenomicSeq::getChromEnd() const
	{
	return getChromStart()+ (int32_t)array->size();
	}

char GenomicSeq::at(int32_t index) const
	{
	if(index < getChromStart() || index>= getChromEnd() ) return '?';
	return std::toupper(array->at(index-getChromStart()));
	}
int32_t GenomicSeq::size() const
	{
	return getChromEnd();
	}
