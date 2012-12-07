#ifndef BEDINDEX_H
#define BEDINDEX_H
#include <memory>
/**
 * Simple wrapper for samtools-0.1.18/bedidx.c
 *
 */
class BedIndex
	{
	private:
		/* the BED structure */
		void* bed;
		/* private CTOR */
		BedIndex(void* bed);
	public:
		virtual ~BedIndex();
		/** test an overlap vs this BED */
		bool overlap(const char *chr, int beg, int end) const;
		/** creates an index from filename or throws an exception on error */
		static std::auto_ptr<BedIndex> read(const char* filename);
		
	};


#endif
