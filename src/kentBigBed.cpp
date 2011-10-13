/*
 * KentBigWig.cpp
 *
 *  Created on: Oct 13, 2011
 *      Author: lindenb
 */
#include <cerrno>
#include <cstring>
#include <limits>
#ifndef NOKENTSRC
extern "C" {
	#include "common.h"
	#include "localmem.h"
	#include "linefile.h"
	#include "hash.h"
	#include "options.h"
	#include "sqlNum.h"
	#include "udc.h"
	#include "bigBed.h"
}
#endif
#include "kentBigBed.h"
#include "throw.h"

using namespace std;

#ifndef NOKENTSRC
#define CASTPTR ((struct bbiFile *)bbFile)
#endif

BigBed::BigBed(const char* filename):bbFile(NULL)
	{
#ifdef NOKENTSRC
	THROW("Compiled without kent src");
#else
	struct bbiFile *bwf = ::bigBedFileOpen((char*)filename);
	if(bwf==NULL)
		{
		THROW("Cannot open bigbed file \""<< filename << "\" " << strerror(errno));
		}
	bbFile=bwf;
#endif
	}

BigBed::~BigBed()
	{
	close();
	}



void BigBed::close()
	{
#ifndef NOKENTSRC
	if(bbFile!=NULL)
		{
		struct bbiFile *bf=CASTPTR;
		bigBedFileClose(&bf);
		bbFile=NULL;
		}
#endif
	}

auto_ptr<vector<BigBed::Interval > >
BigBed::query(const char* chrom,int32_t start,int32_t end,int32_t maxItems)
	{
	std::vector<Interval >* v=new std::vector<Interval>;
#ifdef NOKENTSRC
	return auto_ptr<vector<Interval > >(v);
#else
	 struct lm *lm=lmInit(0);
	 struct bigBedInterval* i=::bigBedIntervalQuery(
			CASTPTR, (char*)chrom,start,end, maxItems, lm);
	 struct bigBedInterval*iter=i;
	 for(;iter!=NULL;iter=iter->next)
	 	 {
		 v->push_back(BigBed::Interval(iter->start,iter->end,iter->rest));
	 	 }
	::lmCleanup(&lm);
	return auto_ptr<vector<Interval > >(v);
#endif
	}


BigBed::Interval::Interval(int32_t start,int32_t end,const char* rest):
		start(start),
		end(end),
		line(rest==NULL?"":rest)
	{
	}

BigBed::Interval::Interval(const BigBed::Interval& cp):
		start(cp.start),
		end(cp.end),
		line(cp.line)
	{
	}

BigBed::Interval& BigBed::Interval::operator=(const BigBed::Interval& cp)
	{
	if(this!=&cp)
		{
		start=cp.start;
		end=cp.end;
		line=cp.line;
		}
	return *this;
	}
