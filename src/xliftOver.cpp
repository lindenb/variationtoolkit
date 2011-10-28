/*
 * xliftOver.h
 *
 *  Created on: Oct 28, 2011
 *      Author: lindenb
 */


#include "common.h"
#include "hash.h"
#include "liftOver.h"
#include "xliftOver.h"

#ifdef min
#undef min
#endif
#ifdef max
#undef max
#endif

#include <cstdio>

using namespace std;

#define CAST_HASH(h) ((struct hash *)h)

LiftOver::LiftOver(const char* mapFile)
	{
	chainHash = ::newHash(0);
	::readLiftOverMap((char*)mapFile,CAST_HASH(chainHash));
	}

LiftOver::~LiftOver()
	{
	struct hash *t=CAST_HASH(chainHash);
	::freeHash(&t);
	}

void LiftOver::convert(const char* chrom,int start,int end, char* strand)
	{

	char *error = ::liftOverRemapRange(
			CAST_HASH(chainHash),
			minMatch, (char*)chrom, start, end, *strand,
			minMatch, (char**)&chrom, &start, &end, strand);

	}
