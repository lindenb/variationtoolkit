#ifndef GENOMIC_SEQ_H
#define GENOMIC_SEQ_H

#include <memory>
#include "abstractcharsequence.h"
#include "xfaidx.h"

class GenomicSeq:public AbstractCharSequence
    {
    private:
		std::auto_ptr<std::string> array;
		int32_t chromStart;
    public:
		GenomicSeq(IndexedFasta* indexedFasta,
			 const char* chrom,
			 int32_t chromStart0,
			 int32_t chromEnd0
			 );
		virtual ~GenomicSeq();
		int getChromStart() const;
		int getChromEnd() const;
		virtual char at(int32_t index) const;
		virtual int32_t size() const;
    };


#endif
