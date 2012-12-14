#include <stdint.h>
#include "bedindex.h"
#include "throw.h"

extern "C" {
extern void bed_index(void *_h);
extern int bed_overlap(const void *_h, const char *chr, int beg, int end);
extern  void *bed_read(const char *fn);
extern void bed_destroy(void *_h);
}

BedIndex::BedIndex(void* bed):bed(bed)
	{
	}

BedIndex::~BedIndex()
	{
	::bed_destroy(this->bed);
	}


bool BedIndex::overlap(const char *chr, int beg, int end) const
	{
	return ::bed_overlap(this->bed, chr, beg,end)!=0;
	}

std::auto_ptr<BedIndex> BedIndex::read(const char* filename)
	{
	if(filename==0)  THROW("BedIndex::read : filename is null");
	void* B=bed_read(filename);
	if(B==0)  THROW("Cannot read BED file from " << filename);
	return std::auto_ptr<BedIndex>(new BedIndex(B));
	}

