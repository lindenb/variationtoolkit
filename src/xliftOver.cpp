/*
 * xliftOver.h
 *
 *  Created on: Oct 28, 2011
 *      Author: lindenb
 */
#ifndef NOKENTSRC
extern "C" {

#include "common.h"
#include "hash.h"
#include "liftOver.h"
}
#endif
#include "xliftOver.h"
#ifdef min
#undef min
#endif
#ifdef max
#undef max
#endif

#include <cstdio>

using namespace std;

#ifndef NOKENTSRC
#define CAST_HASH(h) ((struct hash *)h)
#endif

LiftOver::LiftOver(const char* mapFile):
#ifndef NOKENTSRC
	_minMatch(LIFTOVER_MINMATCH),_minBlocks(LIFTOVER_MINBLOCKS)
#else
	_minMatch(0),_minBlocks(0)
#endif
    {
#ifndef NOKENTSRC
    chainHash = ::newHash(0);
    ::readLiftOverMap((char*)mapFile,CAST_HASH(chainHash));
#endif
    }

LiftOver::~LiftOver()
    {
#ifndef NOKENTSRC
    struct hash *t=CAST_HASH(chainHash);
    ::freeHash(&t);
#endif
    }

const char* LiftOver::lastError() const
    {
#ifndef NOKENTSRC
    return last_error.get()==NULL?NULL:last_error->c_str();
#else
    return "NOT COMPILED WITH UCSC SRC";
#endif
    }

std::auto_ptr<ChromStrandStartEnd> LiftOver::convert(
	const ChromStrandStartEnd* src
	)
    {
#ifdef NOKENTSRC
    return auto_ptr<ChromStrandStartEnd>();
#else
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
#endif
    }


std::auto_ptr<ChromStartEnd> LiftOver::convert(
	const ChromStartEnd* src
	)
    {
    auto_ptr<ChromStartEnd> ret;
    ChromStrandStartEnd seg(*src,'+');
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
