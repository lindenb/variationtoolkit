/*
 * KentBigWig.cpp
 *
 *  Created on: Oct 13, 2011
 *      Author: lindenb
 */
#include <cerrno>
#include <cstring>
#include <limits>
#ifdef NOKENTSRC
#warning "variable $KENTDIR hasn't been defined.";
enum { bbiSumMean,bbiSumMax,bbiSumMin,bbiSumStandardDeviation,bbiSumCoverage};
#else
extern "C" {
	#include "common.h"
	#include "linefile.h"
	#include "hash.h"
	#include "options.h"
	#include "sqlNum.h"
	#include "udc.h"
	#include "bigWig.h"
}
#endif
#include "kentBigWig.h"
#include "throw.h"

using namespace std;

#ifndef NOKENTSRC
#define CASTPTR ((struct bbiFile *)bbFile)
#endif

BigWig::BigWig(const char* filename):bbFile(NULL)
	{
#ifdef NOKENTSRC
	THROW("Compiled without kent src");
#else
	if(!isBigWig((char*)filename)) THROW("not a bigwig file \""<< filename << "\" ");
	struct bbiFile *bwf = ::bigWigFileOpen((char*)filename);
	if(bwf==NULL)
		{
		THROW("Cannot open bigwig file \""<< filename << "\" " << strerror(errno));
		}
	bbFile=bwf;
#endif
	}

BigWig::~BigWig()
	{
	close();
	}

double BigWig::summary(const char*  chrom,int start,int end,int summaryType)
	{
	double nan_value= std::numeric_limits<double>::quiet_NaN();
#ifdef NOKENTSRC
	return nan_value;
#else
	if(bbFile==NULL  || chrom==NULL || end<start) return nan_value;
    double summaryValues[]={ nan_value};

	if(::bigWigSummaryArray(CASTPTR,(char*)chrom, start, end, (bbiSummaryType)summaryType, 1, summaryValues))
		{
		if(!isnan(summaryValues[0]))
			{
			return summaryValues[0];
			}
		}
	return nan_value;
#endif
	}

double BigWig::mean(const char*  chrom,int start,int end)
	{
	return summary(chrom,start,end,bbiSumMean);
	}

double BigWig::maximum(const char*  chrom,int start,int end)
	{
	return summary(chrom,start,end,bbiSumMax);
	}

double BigWig::minimum(const char*  chrom,int start,int end)
	{
	return summary(chrom,start,end,bbiSumMin);
	}

double BigWig::coverage(const char*  chrom,int start,int end)
	{
	return summary(chrom,start,end,bbiSumCoverage);
	}

double BigWig::stdDev(const char*  chrom,int start,int end)
	{
	return summary(chrom,start,end,bbiSumStandardDeviation);
	}


void BigWig::close()
	{
#ifndef NOKENTSRC
	if(bbFile!=NULL)
		{
		struct bbiFile *bf=CASTPTR;
		bigWigFileClose(&bf);
		bbFile=NULL;
		}
#endif
	}
