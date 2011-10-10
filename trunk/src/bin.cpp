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
