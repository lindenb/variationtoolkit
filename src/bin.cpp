/* from http://genomewiki.ucsc.edu/index.php/Bin_indexing_system */
#include "bin.h"
#include "throw.h"

using namespace std;
/* This file is copyright 2002 Jim Kent, but license is hereby
 * granted for all use - public, private or commercial. */

static int binOffsets[] = {512+64+8+1, 64+8+1, 8+1, 1, 0};
#define _binFirstShift 17       /* How much to shift to get to finest bin. */
#define _binNextShift 3         /* How much to shift to get to next larger bin. */
#define ArraySize(a) (sizeof(a)/sizeof((a)[0]))

/* Given start,end in chromosome coordinates assign it
 * a bin.   There's a bin for each 128k segment, for each
 * 1M segment, for each 8M segment, for each 64M segment,
 * and for each chromosome (which is assumed to be less than
 * 512M.)  A range goes into the smallest bin it will fit in. */
int UcscBin::binFromRangeStandard(int start, int end)
    {
    int startBin = start, endBin = end-1, i;
    startBin >>= _binFirstShift;
    endBin >>= _binFirstShift;
    for (i=0; i< (int)ArraySize(binOffsets); ++i)
	{
	if (startBin == endBin)
	    return binOffsets[i] + startBin;
	startBin >>= _binNextShift;
	endBin >>= _binNextShift;
	}
    THROW("start "<< start << ", end "<< end <<" out of range in findBin (max is 512M)");
    return 0;
    }


void UcscBin::binsInRange(
        int chromStart,
        int chromEnd,
        int binId,
        int level,
        int binRowStart,
        int rowIndex,
        int binRowCount,
        int genomicPos,
        int genomicLength,
        vector<int>& binList
        )
        {
	binList.push_back(binId);
        if(level<4)
		{
		int i;
		int childLength=genomicLength/8;
		int childBinRowCount=binRowCount*8;
		int childRowBinStart=binRowStart+binRowCount;
		int firstChildIndex=rowIndex*8;
		int firstChildBin=childRowBinStart+firstChildIndex;
		for(i=0;i< 8;++i)
		        {
		        int childStart=genomicPos+i*childLength;

		        if( chromStart>(childStart+childLength) ||
		                chromEnd<childStart )
		                {
		                continue;
		                }
		        binsInRange(
		                chromStart,
		                chromEnd,
		                firstChildBin+i,
		                level+1,
		                childRowBinStart,
		                firstChildIndex+i,
		                childBinRowCount,
		                childStart,
		                childLength,
				binList
		                );
		        }
		}
        }

void UcscBin::bins(int chromStart,int chromEnd,vector<int>& binList)
	{
	int genomicLength=536870912;
	binList.clear();
	binsInRange(chromStart,chromEnd,0,0,0,0,1,0,genomicLength,binList);
	}

