/*
 * xliftOver.h
 *
 *  Created on: Oct 28, 2011
 *      Author: lindenb
 */
extern "C" {

#include "common.h"
#include "hash.h"
#include "liftOver.h"
}
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

LiftOver::LiftOver(const char* mapFile):_minMatch(LIFTOVER_MINMATCH),_minBlocks(LIFTOVER_MINBLOCKS)
    {
    chainHash = ::newHash(0);
    ::readLiftOverMap((char*)mapFile,CAST_HASH(chainHash));
    }

LiftOver::~LiftOver()
    {
    struct hash *t=CAST_HASH(chainHash);
    ::freeHash(&t);
    }

const char* LiftOver::lastError() const
    {
    return last_error.get()==NULL?NULL:last_error->c_str();
    }

std::auto_ptr<ChromStrandStartEnd> LiftOver::convert(
	const ChromStrandStartEnd* src
	)
    {
    char* retChrom=NULL;
    int retStart;
    int retEnd;
    char retStrand;
    auto_ptr<ChromStrandStartEnd> ret;
    last_error.reset();
    char *error = ::liftOverRemapRange(
	    CAST_HASH(chainHash),
	    minBlocks(),
	    (char*)src->chrom.c_str(),
	    src->start,
	    src->end,
	    src->strand(),
	    minMatch(),
	    &retChrom,
	    &retStart,
	    &retEnd,
	    &retStrand
	    );

    if(error!=NULL)
	{
	last_error.reset(new std::string(error));
	}
    else
	{
	ret.reset(new ChromStrandStartEnd(
	    retChrom,
	    retStart,
	    retEnd,
	    retStrand
	    ));
	}
    if(retChrom!=NULL) freeMem(retChrom);
    return ret;
    }


std::auto_ptr<ChromStartEnd> LiftOver::convert(
	const ChromStartEnd* src
	)
    {
    auto_ptr<ChromStartEnd> ret;
    ChromStrandStartEnd seg(*src,true);
    auto_ptr<ChromStrandStartEnd> p=convert(&seg);
    if(p.get()==NULL)
	{
	return ret;
	}
    ret.reset(new ChromStartEnd(
	p->chrom,
	p->start,
	p->end
	));
    return ret;
    }

void LiftOver::minBlocks(double v)
    {
    _minBlocks=v;
    }
void LiftOver::minMatch(double v)
    {
    _minMatch=v;
    }
double LiftOver::minBlocks() const
    {
    return _minBlocks;
    }
double LiftOver::minMatch() const
    {
    return _minMatch;
    }
