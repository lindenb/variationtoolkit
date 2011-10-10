#ifndef UCSC_BIN_H
#define UCSC_BIN_H

class UcscBin
	{
	public:
/* Given start,end in chromosome coordinates assign it
 * a bin.   There's a bin for each 128k segment, for each
 * 1M segment, for each 8M segment, for each 64M segment,
 * and for each chromosome (which is assumed to be less than
 * 512M.)  A range goes into the smallest bin it will fit in. */
		static int binFromRangeStandard(int start, int end);
	};

#endif
